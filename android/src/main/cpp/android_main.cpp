#include <algorithm>
#include <cerrno>
#include <climits>
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

#include "android_asset/android_asset.hpp"
#include "android_input/android_input.hpp"
#include "api_graphics/android_graphics.hpp"
#include "api_graphics/opengles_graphics.hpp"

enum APP_CMD : char {
  APP_CMD_CREATE = 0,
  APP_CMD_START,
  APP_CMD_RESUME,
  APP_CMD_INPUT_INIT,
  APP_CMD_INIT_WINDOW,
  APP_CMD_GAINED_FOCUS,
  APP_CMD_WINDOW_RESIZED,
  APP_CMD_CONTENT_RECT_CHANGED,
  APP_CMD_CONFIG_CHANGED,
  APP_CMD_LOST_FOCUS,
  APP_CMD_LOW_MEMORY,
  APP_CMD_SAVE_STATE,
  APP_CMD_TERM_WINDOW,
  APP_CMD_INPUT_TERM,
  APP_CMD_PAUSE,
  APP_CMD_STOP,
  APP_CMD_DESTROY,
};
struct android_app {
  bool destroyed;
  char appCmdState = APP_CMD_CREATE;
  int msgread, msgwrite;
  ANativeActivity *activity;
  AConfiguration *config;
  ALooper *looper;
  ANativeWindow *window = nullptr; // update in mainThread
  AInputQueue *inputQueue;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_t thread;
};

static android_graphics *a_graphics;
#include <cstdio>

static void *android_app_entry (void *param) {
  android_app *app = (android_app *)param;
  app->config = AConfiguration_new ();
  AConfiguration_fromAssetManager (app->config, app->activity->assetManager);
  app->looper = ALooper_prepare (ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
  ALooper_addFd (app->looper, app->msgread, 1, ALOOPER_EVENT_INPUT, NULL, nullptr);
  a_graphics = new opengles_graphics{};
  {
    bool running = false, resume = false;
    unsigned int resize = 0;
    char cmd = APP_CMD_CREATE;
    Main *m_Main = nullptr;
    ANativeWindow *window = nullptr;
    android_asset a_asset (app->activity->assetManager);
    android_input a_input (app->looper);
    while (cmd != APP_CMD_DESTROY) {
      switch (ALooper_pollAll (running ? 0 : -1, nullptr, nullptr, nullptr)) {
      case 2: // input queue
        a_input.process_input ();
        break;
      case 3: // sensor queue
        a_input.process_sensor ();
        break;
      case 1: // android activity queue
        if (read (app->msgread, &cmd, sizeof (cmd)) != sizeof (cmd)) break;
        switch (cmd) {
        case APP_CMD_INIT_WINDOW:
          pthread_mutex_lock (&app->mutex);
          window = app->window;
          pthread_mutex_unlock (&app->mutex);
          break;
        case APP_CMD_GAINED_FOCUS:
          a_input.attach_sensor ();
          break;
        case APP_CMD_INPUT_INIT:
          a_input.set_input_queue (app->looper, app->inputQueue);
          break;
        case APP_CMD_INPUT_TERM:
          if (app->inputQueue != NULL) {
            a_input.set_input_queue (app->looper, NULL);
            app->inputQueue = NULL;
          }
          break;
        case APP_CMD_LOST_FOCUS:
          a_input.detach_sensor ();
          break;
        case APP_CMD_TERM_WINDOW:
          a_graphics->onWindowTerm ();
          window = nullptr;
          break;
        case APP_CMD_CONFIG_CHANGED:
          AConfiguration_fromAssetManager (app->config, app->activity->assetManager);
          break;
        case APP_CMD_PAUSE:
          if (window) {
            a_graphics->preRender(window, resize);
            // core
            if (!m_Main)
              m_Main = new Main;
            m_Main->pause ();
            a_graphics->postRender(false);
          }
          running = false;
          break;
        case APP_CMD_DESTROY:
          if (window) {
            a_graphics->preRender(window, resize);
            // core
            if (m_Main) {
              delete m_Main;
              m_Main = nullptr;
            }
          }
          a_graphics->postRender(true);
          break;
        default:
          // ?
          break;
        }
        pthread_mutex_lock (&app->mutex);
        app->appCmdState = cmd;
        pthread_cond_broadcast (&app->cond);
        pthread_mutex_unlock (&app->mutex);
        switch (cmd) {
        case APP_CMD_RESUME:
          running = true;
          resume = true;
          break;
        case APP_CMD_CONTENT_RECT_CHANGED:
          resize |= 1;
          break;
        case APP_CMD_WINDOW_RESIZED:
          resize |= 2;
          break;
        default:
          // ?
          break;
        }
        break;
      default:
        if (!running) break;
        a_input.process_event ();
        if (!window) break;
        a_graphics->preRender(window, resize);
        // core
        if (!m_Main) {
          m_Main = new Main;
          resume = false;
        } else if (resume) {
          m_Main->resume ();
          resume = false;
        }
        m_Main->render ();
        a_graphics->postRender(false);
        break;
      }
    }
  }
  delete a_graphics;
  a_graphics = nullptr;
  pthread_mutex_lock (&app->mutex);
  AConfiguration_delete (app->config);
  app->destroyed = true;
  pthread_cond_broadcast (&app->cond);
  pthread_mutex_unlock (&app->mutex);
  return NULL;
}
static const size_t WRITEPIPE_SIZE = sizeof (char);
static void write_android_cmd (android_app *app, char cmd) {
  while (write (app->msgwrite, &cmd, WRITEPIPE_SIZE) != WRITEPIPE_SIZE)
    LOGE ("cannot write on pipe , %s", strerror (errno));
  pthread_mutex_lock (&app->mutex);
  while (app->appCmdState != cmd)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}
static void onStart (ANativeActivity *activity) {
  android_app *app = (android_app *)activity->instance;
  write_android_cmd (app, APP_CMD_START);
}
static void onResume (ANativeActivity *activity) {
  android_app *app = (android_app *)activity->instance;
  write_android_cmd (app, APP_CMD_RESUME);
}
static void onNativeWindowCreated (ANativeActivity *activity, ANativeWindow *window) {
  android_app *app = (android_app *)activity->instance;
  if (app->window) // window should null when window create
    write_android_cmd (app, APP_CMD_TERM_WINDOW);
  app->window = window;
  write_android_cmd (app, APP_CMD_INIT_WINDOW);
}
static void onInputQueueCreated (ANativeActivity *activity, AInputQueue *queue) {
  android_app *app = (android_app *)activity->instance;
  if (app->inputQueue)
    write_android_cmd (app, APP_CMD_INPUT_TERM);
  app->inputQueue = queue;
  write_android_cmd (app, APP_CMD_INPUT_INIT);
}
static void onConfigurationChanged (ANativeActivity *activity) {
  android_app *app = (android_app *)activity->instance;
  write_android_cmd (app, APP_CMD_CONFIG_CHANGED);
}
static void onLowMemory (ANativeActivity *activity) {
  android_app *app = (android_app *)activity->instance;
  write_android_cmd (app, APP_CMD_LOW_MEMORY);
}
static void onWindowFocusChanged (ANativeActivity *activity, int focused) {
  android_app *app = (android_app *)activity->instance;
  write_android_cmd (app, focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}
static void onNativeWindowResized (ANativeActivity *activity, ANativeWindow *) {
  android_app *app = (android_app *)activity->instance;
  write_android_cmd (app, APP_CMD_WINDOW_RESIZED);
}
static void onContentRectChanged (ANativeActivity *activity, const ARect *) {
  android_app *app = (android_app *)activity->instance;
  write_android_cmd (app, APP_CMD_CONTENT_RECT_CHANGED);
}
// wanna safe setting to Android Bundle safe instance?
static void *onSaveInstanceState (ANativeActivity *activity, size_t *outLen) {
  android_app *app = (android_app *)activity->instance;
  write_android_cmd (app, APP_CMD_SAVE_STATE);
  *outLen = 0;
  return nullptr;
}
static void onNativeWindowDestroyed (ANativeActivity *activity, ANativeWindow *) {
  android_app *app = (android_app *)activity->instance;
  if (!app->window) return;
  app->window = nullptr;
  write_android_cmd (app, APP_CMD_TERM_WINDOW);
}
static void onInputQueueDestroyed (ANativeActivity *activity, AInputQueue *) {
  android_app *app = (android_app *)activity->instance;
  if (!app->inputQueue) return;
  write_android_cmd (app, APP_CMD_INPUT_TERM);
}
static void onPause (ANativeActivity *activity) {
  android_app *app = (android_app *)activity->instance;
  write_android_cmd (app, APP_CMD_PAUSE);
}
static void onStop (ANativeActivity *activity) {
  android_app *app = (android_app *)activity->instance;
  write_android_cmd (app, APP_CMD_STOP);
}
static void onDestroy (ANativeActivity *activity) {
  android_app *app = (android_app *)activity->instance;
  write_android_cmd (app, APP_CMD_DESTROY);
  pthread_mutex_lock (&app->mutex);
  while (!app->destroyed)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
  close (app->msgread);
  close (app->msgwrite);
  pthread_cond_destroy (&app->cond);
  pthread_mutex_destroy (&app->mutex);
  delete app;
  activity->instance = nullptr;
}

void ANativeActivity_onCreate (ANativeActivity *activity, void *, size_t) {
  activity->callbacks->onStart = onStart;
  activity->callbacks->onResume = onResume;
  activity->callbacks->onInputQueueCreated = onInputQueueCreated;
  activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
  activity->callbacks->onNativeWindowResized = onNativeWindowResized;
  activity->callbacks->onContentRectChanged = onContentRectChanged;
  activity->callbacks->onConfigurationChanged = onConfigurationChanged;
  activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
  activity->callbacks->onLowMemory = onLowMemory;
  activity->callbacks->onSaveInstanceState = onSaveInstanceState;
  activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
  activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
  activity->callbacks->onPause = onPause;
  activity->callbacks->onStop = onStop;
  activity->callbacks->onDestroy = onDestroy;

  android_app *app = new android_app{};
  activity->instance = app;
  app->activity = activity;
  pthread_mutex_init (&app->mutex, NULL);
  pthread_cond_init (&app->cond, NULL);
  while (pipe (&app->msgread))
    LOGE ("Failed to create pipe, %s", strerror (errno));
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  pthread_create (&app->thread, &attr, android_app_entry, app);
  pthread_attr_destroy (&attr);
}

// native MainActivity.java
#include <jni.h>

extern "C" JNIEXPORT void JNICALL Java_com_ariasaproject_technowar_MainActivity_setInsets (JNIEnv *, jclass, jint left, jint top, jint right, jint bottom) {
  if (!a_graphics) return;
  a_graphics->cur_safe_insets[0] = left;
  a_graphics->cur_safe_insets[1] = top;
  a_graphics->cur_safe_insets[2] = right;
  a_graphics->cur_safe_insets[3] = bottom;
}
