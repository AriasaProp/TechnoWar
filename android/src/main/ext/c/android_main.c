#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>
#include <unistd.h>

#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

#include "engine.h"
#include "log.h"
#include "main_game.h"
#include "util.h"
#include <jni.h>

#include "android_engine.h"

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

struct android_app{
  int destroyed, wait_request;
  int msgread,
      msgwrite;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_t thread;
} *app = nullptr;

typedef struct {
  unsigned char cmd;
  void *data;
} msg_pipe;

static void *android_app_entry (void *n) {
  if (!app) return NULL;
  ANativeActivity *activity = (ANativeActivity *)n;
  AConfiguration *aconfig = AConfiguration_new ();
  AConfiguration_fromAssetManager (aconfig, activity->assetManager);
  ALooper *looper = ALooper_prepare (ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
  ALooper_addFd (looper, app->msgread, 1, ALOOPER_EVENT_INPUT, NULL, nullptr);
  // 1 = created, 2 = running, 4 = started, 8 = resume
  int StateFlags = 0;
  init_engine (activity->assetManager, activity->sdkVersion, looper);
  try {
    msg_pipe read_cmd{APP_CMD_CREATE, nullptr};
    for (;;) {
      switch (ALooper_pollOnce ((StateFlags & 6) == 6 ? 0 : -1, nullptr, nullptr, nullptr)) {
      case ALOOPER_POLL_CALLBACK:
        break;
      case 1:
        // activity handler
        if (read (app->msgread, &read_cmd, sizeof (msg_pipe)) == sizeof (msg_pipe)) {
#define END_WAITING                    \
  pthread_mutex_lock (&app->mutex);    \
  app->wait_request = 0;           \
  pthread_cond_broadcast (&app->cond); \
  pthread_mutex_unlock (&app->mutex);
          switch (read_cmd.cmd) {
          case APP_CMD_WINDOW_UPDATE:
            android_graphics_onWindowChange ((ANativeWindow *)read_cmd.data);
            END_WAITING
            break;
          case APP_CMD_FOCUS_CHANGED:
            if (read_cmd.data)
              android_input_attach_sensor ();
            else
              android_input_detach_sensor ();
            END_WAITING
            break;
          case APP_CMD_INPUT_UPDATE:
            android_input_set_input_queue (looper, (AInputQueue *)read_cmd.data);
            END_WAITING
            break;
          case APP_CMD_PAUSE:
            if (android_graphics_preRender ()) {
              Main_pause ();
              android_graphics_postRender (false);
              StateFlags &= ~2;
            }
            END_WAITING
            break;
          case APP_CMD_SAVE_STATE:
            break;
          case APP_CMD_CONFIG_CHANGED:
            AConfiguration_fromAssetManager (aconfig, (AAssetManager *)read_cmd.data);
            break;
          case APP_CMD_STOP:
            StateFlags &= ~4;
            break;
          case APP_CMD_START:
            StateFlags |= 4;
            break;
          case APP_CMD_RESUME:
            StateFlags |= 10;
            break;
          case APP_CMD_WINDOW_REDRAW_NEEDED:
            break;
          case APP_CMD_CONTENT_RECT_CHANGED:
            android_graphics_onWindowResize ();
            break;
          case APP_CMD_WINDOW_RESIZED:
            android_graphics_onWindowResizeDisplay ();
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
        if (android_graphics_preRender ()) {
          engine_input_process_event ();

          if (!(StateFlags & 1)) {
            Main_start ();
            StateFlags |= 1; // created
            StateFlags &= ~8; // not resume
          } else if (StateFlags & 8) { // resuming
            Main_resume ();
            StateFlags &= ~8; // not resume
          }
          Main_render ();
          android_graphics_postRender (false);
        }
        break;
      }
    }

  } catch (...) {
    // when destroy
    if (android_graphics_preRender ())
      Main_end ();
    StateFlags = 0; // reset flags
    android_graphics_postRender (true);
  }
  term_engine ();
  // loop ends
  ALooper_removeFd (looper, app->msgread);
  AConfiguration_delete (aconfig);
  pthread_mutex_lock (&app->mutex);
  app->destroyed = 1;
  pthread_cond_broadcast (&app->cond);
  pthread_mutex_unlock (&app->mutex);
  return NULL;
}

static msg_pipe write_cmd;
void ANativeActivity_onCreate (ANativeActivity *activity, void *, size_t) {
  // initialize application
  app = (struct android_app*)allocate_imemory(sizeof(struct android_app));
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
  app->wait_request = 1;                      \
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
    free_memory(app);
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

JNIEXPORT void Java_com_ariasaproject_technowar_MainActivity_insetNative (JNIEnv *, jobject, jint left, jint top, jint right, jint bottom) {
  android_graphics_cur_safe_insets[0] = left;
  android_graphics_cur_safe_insets[1] = top;
  android_graphics_cur_safe_insets[2] = right;
  android_graphics_cur_safe_insets[3] = bottom;
  if (android_graphics_onWindowResize)
    android_graphics_onWindowResize ();
}