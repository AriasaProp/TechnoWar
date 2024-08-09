#include <algorithm>
#include <cassert>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <string>
#include <sys/resource.h>
#include <unistd.h>

#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

#include "engine.hpp"
#include "log.hpp"
#include "main_game.hpp"
#include <jni.h>

#include "android_engine.hpp"

enum APP_CMD : unsigned char {
  APP_CMD_CREATE = 0,
  APP_CMD_START,
  APP_CMD_RESUME,
  APP_CMD_INPUT_UPDATE,
  APP_CMD_WINDOW_UPDATE,
  APP_CMD_WINDOW_RESIZED,
  APP_CMD_WINDOW_REDRAW_NEEDED,
  APP_CMD_CONTENT_RECT_CHANGED,
  APP_CMD_CONFIG_CHANGED,
  APP_CMD_FOCUS_CHANGED,
  APP_CMD_LOW_MEMORY,
  APP_CMD_SAVE_STATE,
  APP_CMD_PAUSE,
  APP_CMD_STOP,
  APP_CMD_DESTROY,
};
/*
static JavaVMAttachArgs attachArgs{
    .version = JNI_VERSION_1_6,
    .name = "TechnoWar",
    .group = NULL};
*/
struct android_app {
  bool destroyed,
      wait_request = false;
  int msgread,
      msgwrite;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_t thread;
} *app = nullptr;

struct msg_pipe {
  unsigned char cmd;
  void *data;
};

static void *android_app_entry (void *n) {
  if (!app) return NULL;
  ANativeActivity *activity = (ANativeActivity *)n;
  AConfiguration *aconfig = AConfiguration_new ();
  AConfiguration_fromAssetManager (aconfig, activity->assetManager);
  ALooper *looper = ALooper_prepare (ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
  ALooper_addFd (looper, app->msgread, 1, ALOOPER_EVENT_INPUT, NULL, nullptr);
  bool created = false;
  bool running = false,
       started = false,
       resume = false;
  init_engine (activity->assetManager, activity->sdkVersion, looper);
  try {
    msg_pipe read_cmd{APP_CMD_CREATE, nullptr};
    for (;;) {
      switch (ALooper_pollOnce ((started && running) ? 0 : -1, nullptr, nullptr, nullptr)) {
      case ALOOPER_POLL_CALLBACK:
        break;
      case 1:
        // activity handler
        if (read (app->msgread, &read_cmd, sizeof (msg_pipe)) == sizeof (msg_pipe)) {
#define END_WAITING                    \
  pthread_mutex_lock (&app->mutex);    \
  app->wait_request = false;           \
  pthread_cond_broadcast (&app->cond); \
  pthread_mutex_unlock (&app->mutex);
          switch (read_cmd.cmd) {
          case APP_CMD_WINDOW_UPDATE:
            android_graphics::onWindowChange ((ANativeWindow *)read_cmd.data);
            END_WAITING
            break;
          case APP_CMD_FOCUS_CHANGED:
            if (read_cmd.data)
              android_input::attach_sensor ();
            else
              android_input::detach_sensor ();
            END_WAITING
            break;
          case APP_CMD_INPUT_UPDATE:
            android_input::set_input_queue (looper, (AInputQueue *)read_cmd.data);
            END_WAITING
            break;
          case APP_CMD_PAUSE:
            if (android_graphics::preRender ()) {
              Main::pause ();
              android_graphics::postRender (false);
              running = false;
            }
            END_WAITING
            break;
          case APP_CMD_SAVE_STATE:
            break;
          case APP_CMD_CONFIG_CHANGED:
            AConfiguration_fromAssetManager (aconfig, (AAssetManager *)read_cmd.data);
            break;
          case APP_CMD_STOP:
            started = false;
            break;
          case APP_CMD_START:
            started = true;
            break;
          case APP_CMD_RESUME:
            running = true;
            resume = true;
            break;
          case APP_CMD_WINDOW_REDRAW_NEEDED:
            break;
          case APP_CMD_CONTENT_RECT_CHANGED:
            android_graphics::onWindowResize ();
            break;
          case APP_CMD_WINDOW_RESIZED:
            android_graphics::onWindowResizeDisplay ();
            break;
          case APP_CMD_LOW_MEMORY:
            break;
          case APP_CMD_DESTROY:
            throw "";
          default:
            break;
          }
#undef END_WAITING
        }
        break;
      default:
        // base render
        if (android_graphics::preRender ()) {
          engine::input::process_event ();

          if (!created) {
            Main::start ();
            created = true;
            resume = false;
          } else if (resume) {
            Main::resume ();
            resume = false;
          }
          Main::render ();
          android_graphics::postRender (false);
        }
        break;
      }
    }

  } catch (...) {
    // when destroy
    if (android_graphics::preRender ())
      Main::end ();
    created = false;
    android_graphics::postRender (true);
  }
  term_engine ();
  // loop ends
  ALooper_removeFd (looper, app->msgread);
  AConfiguration_delete (aconfig);
  pthread_mutex_lock (&app->mutex);
  app->destroyed = true;
  pthread_cond_broadcast (&app->cond);
  pthread_mutex_unlock (&app->mutex);
  return NULL;
}

static msg_pipe write_cmd;
void ANativeActivity_onCreate (ANativeActivity *activity, void *, size_t) {
  // initialize application
  app = new android_app;
  pthread_mutex_init (&app->mutex, NULL);
  pthread_cond_init (&app->cond, NULL);
  while (pipe (&app->msgread) == -1) {
    // force loop foor provide pipe
    LOGE ("Failed to create pipe, %s", strerror (errno));
  }

#define WRITE_ANDROID_CMD(A, B)                                                     \
  write_cmd.cmd = A;                                                                \
  write_cmd.data = B;                                                               \
  while (write (app->msgwrite, &write_cmd, sizeof (msg_pipe)) != sizeof (msg_pipe)) \
    LOGE ("cannot write on pipe , %s", strerror (errno));

#define WRITE_ANDROID_CMD_W(A, B)                \
  pthread_mutex_lock (&app->mutex);              \
  app->wait_request = true;                      \
  pthread_mutex_unlock (&app->mutex);            \
  WRITE_ANDROID_CMD (A, B)                       \
  pthread_mutex_lock (&app->mutex);              \
  while (app->wait_request)                      \
    pthread_cond_wait (&app->cond, &app->mutex); \
  pthread_mutex_unlock (&app->mutex);

  // initialize lifecycle
  activity->callbacks->onStart = [] (ANativeActivity *) {
    WRITE_ANDROID_CMD (APP_CMD_START, nullptr)
  };
  activity->callbacks->onResume = [] (ANativeActivity *) {
    WRITE_ANDROID_CMD (APP_CMD_RESUME, nullptr)
  };
  activity->callbacks->onNativeWindowCreated = [] (ANativeActivity *, ANativeWindow *window) {
    WRITE_ANDROID_CMD_W (APP_CMD_WINDOW_UPDATE, (void *)window)
  };
  activity->callbacks->onInputQueueCreated = [] (ANativeActivity *, AInputQueue *queue) {
    WRITE_ANDROID_CMD_W (APP_CMD_INPUT_UPDATE, (void *)queue)
  };
  activity->callbacks->onConfigurationChanged = [] (ANativeActivity *activity) {
    WRITE_ANDROID_CMD (APP_CMD_CONFIG_CHANGED, (void *)activity->assetManager)
  };
  activity->callbacks->onLowMemory = [] (ANativeActivity *) {
    WRITE_ANDROID_CMD (APP_CMD_LOW_MEMORY, nullptr)
  };
  activity->callbacks->onWindowFocusChanged = [] (ANativeActivity *, int focused) {
    WRITE_ANDROID_CMD_W (APP_CMD_FOCUS_CHANGED, reinterpret_cast<void *> (focused));
  };
  activity->callbacks->onNativeWindowResized = [] (ANativeActivity *, ANativeWindow *) {
    WRITE_ANDROID_CMD (APP_CMD_WINDOW_RESIZED, nullptr)
  };
  activity->callbacks->onNativeWindowRedrawNeeded = [] (ANativeActivity *, ANativeWindow *) {
    WRITE_ANDROID_CMD (APP_CMD_WINDOW_REDRAW_NEEDED, nullptr)
  };
  activity->callbacks->onContentRectChanged = [] (ANativeActivity *, const ARect *) {
    WRITE_ANDROID_CMD (APP_CMD_CONTENT_RECT_CHANGED, nullptr)
  };
  activity->callbacks->onSaveInstanceState = [] (ANativeActivity *, size_t *outLen) -> void * {
    WRITE_ANDROID_CMD (APP_CMD_SAVE_STATE, nullptr)
    *outLen = 0;
    return nullptr;
  };
  activity->callbacks->onNativeWindowDestroyed = [] (ANativeActivity *, ANativeWindow *) {
    WRITE_ANDROID_CMD_W (APP_CMD_WINDOW_UPDATE, nullptr)
  };
  activity->callbacks->onInputQueueDestroyed = [] (ANativeActivity *, AInputQueue *) {
    WRITE_ANDROID_CMD_W (APP_CMD_INPUT_UPDATE, nullptr)
  };
  activity->callbacks->onPause = [] (ANativeActivity *) {
    WRITE_ANDROID_CMD_W (APP_CMD_PAUSE, nullptr)
  };
  activity->callbacks->onStop = [] (ANativeActivity *) {
    WRITE_ANDROID_CMD (APP_CMD_STOP, nullptr)
  };
  activity->callbacks->onDestroy = [] (ANativeActivity *) {
    WRITE_ANDROID_CMD (APP_CMD_DESTROY, nullptr)
    pthread_mutex_lock (&app->mutex);
    while (!app->destroyed)
      pthread_cond_wait (&app->cond, &app->mutex);
    pthread_mutex_unlock (&app->mutex);
    close (app->msgread);
    close (app->msgwrite);
    pthread_cond_destroy (&app->cond);
    pthread_mutex_destroy (&app->mutex);
    delete app;
  };
#undef WRITE_ANDROID_CMD_W
#undef WRITE_ANDROID_CMD

  // start thread game
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  pthread_create (&app->thread, &attr, android_app_entry, activity);
  pthread_attr_destroy (&attr);
}

// native MainActivity.java

extern "C" JNIEXPORT void Java_com_ariasaproject_technowar_MainActivity_insetNative (JNIEnv *, jobject, jint left, jint top, jint right, jint bottom) {
  android_graphics::cur_safe_insets[0] = left;
  android_graphics::cur_safe_insets[1] = top;
  android_graphics::cur_safe_insets[2] = right;
  android_graphics::cur_safe_insets[3] = bottom;
  if (android_graphics::onWindowResize)
    android_graphics::onWindowResize ();
}