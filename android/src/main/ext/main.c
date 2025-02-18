#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <jni.h>
#include <limits.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

#include "core.h"
#include "engine.h"
#include "log.h"
#include "manager.h"
#include "util.h"

enum APP_CMD {
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
enum {
  APP_FLAG_WAITING = 1,
  APP_FLAG_DESTROYED = 2,
}

struct android_app {
  int flags;
  int msgread, msgwrite;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_t thread;
  struct android_inputManager *inMngr;
} *app = NULL;

struct msg_pipe {
  enum APP_CMD cmd;
  void *data;
};

enum {
  STATE_CREATED = 1,
  STATE_RUNNING = 2,
  STATE_STARTED = 4,
  STATE_RESUME = 8,
  STATE_WINDOW_EXIST = 16,
  STATE_DESTROY = 32,
};

static void *android_app_entry (void *n) {
  engine_init ();
  ANativeActivity *act = (ANativeActivity *)n;
  AConfiguration *aconfig = AConfiguration_new ();
  AConfiguration_fromAssetManager (aconfig, act->assetManager);
  ALooper *looper = ALooper_prepare (ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
  ALooper_addFd (looper, app->msgread, 1, ALOOPER_EVENT_INPUT, NULL, NULL);
  int StateFlags = 0;
  app->inMngr = android_inputManager_init (looper);
  android_graphicsManager_init ();
  struct msg_pipe read_cmd = {APP_CMD_CREATE, NULL};
  do {
    switch (ALooper_pollOnce ((StateFlags & (STATE_RUNNING | STATE_STARTED | STATE_WINDOW_EXIST)) == (STATE_RUNNING | STATE_STARTED | STATE_WINDOW_EXIST) ? 0 : -1, NULL, NULL, NULL)) {
    case ALOOPER_POLL_CALLBACK:
      break;
    case 1:
      // activity handler
      if (read (app->msgread, &read_cmd, sizeof (struct msg_pipe)) == sizeof (struct msg_pipe)) {
        switch (read_cmd.cmd) {
        case APP_CMD_WINDOW_UPDATE:
          android_graphicsManager_onWindowChange ((ANativeWindow *)read_cmd.data);
          break;
        case APP_CMD_FOCUS_CHANGED:
          android_inputManager_switchSensor (app->inMngr, read_cmd.data);
          break;
        case APP_CMD_INPUT_UPDATE:
          android_inputManager_setInputQueue (app->inMngr, looper, (AInputQueue *)read_cmd.data);
          break;
        case APP_CMD_PAUSE:
          if (android_graphicsManager_preRender ()) {
            pause ();
            android_graphicsManager_postRender ();
            StateFlags &= ~STATE_RUNNING;
          }
          break;
        case APP_CMD_SAVE_STATE:
          break;
        case APP_CMD_CONFIG_CHANGED:
          AConfiguration_fromAssetManager (aconfig, (AAssetManager *)read_cmd.data);
          break;
        case APP_CMD_STOP:
          StateFlags &= ~STATE_STARTED;
          break;
        case APP_CMD_START:
          StateFlags |= STATE_STARTED;
          break;
        case APP_CMD_RESUME:
          StateFlags |= STATE_RUNNING | STATE_RESUME;
          break;
        case APP_CMD_WINDOW_REDRAW_NEEDED:
          break;
        case APP_CMD_CONTENT_RECT_CHANGED:
          android_graphicsManager_onWindowResize ();
          break;
        case APP_CMD_WINDOW_RESIZED:
          android_graphicsManager_onWindowResizeDisplay ();
          break;
        case APP_CMD_LOW_MEMORY:
          break;
        case APP_CMD_DESTROY:
          StateFlags |= STATE_DESTROY;
          continue;
        default:
          break;
        }
        pthread_mutex_lock (&app->mutex);
        if (app->flags & APP_FLAG_WAITING) {
          app->flags &= ~APP_FLAG_WAITING;
          pthread_cond_broadcast (&app->cond);
        }
        pthread_mutex_unlock (&app->mutex);
      }
      break;
    default:
      // base render
      if (android_graphicsManager_preRender ()) {
        // engine_input_process_event ();

        if (!(StateFlags & STATE_CREATED)) {
          start ();
          StateFlags |= STATE_CREATED;          // created
          StateFlags &= ~STATE_RESUME;          // not resume
        } else if (StateFlags & STATE_RESUME) { // resuming
          resume ();
          StateFlags &= ~STATE_RESUME; // not resume
        }
        update ();
        android_graphicsManager_postRender ();
      }
      break;
    }
  } while (!(StateFlags & STATE_DESTROY));
  StateFlags = 0; // reset flags
  // when destroy
  if (android_graphicsManager_preRender ())
    end ();
  android_graphicsManager_term ();
  android_inputManager_term (app->inMngr);
  // loop ends
  ALooper_removeFd (looper, app->msgread);
  AConfiguration_delete (aconfig);
  pthread_mutex_lock (&app->mutex);
  app->flags = APP_FLAG_DESTROYED;
  pthread_cond_broadcast (&app->cond);
  pthread_mutex_unlock (&app->mutex);
  return NULL;
}

static struct msg_pipe write_cmd;
static void onStart (ANativeActivity *UNUSED (act)) {
  write_cmd.cmd = APP_CMD_START;
  write_cmd.data = NULL;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
}
static void onResume (ANativeActivity *UNUSED (act)) {
  write_cmd.cmd = APP_CMD_RESUME;
  write_cmd.data = NULL;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
}
static void onNativeWindowCreated (ANativeActivity *UNUSED (act), ANativeWindow *window) {
  pthread_mutex_lock (&app->mutex);
  app->flags |= APP_FLAG_WAITING;
  pthread_mutex_unlock (&app->mutex);
  write_cmd.cmd = APP_CMD_WINDOW_UPDATE;
  write_cmd.data = (void *)window;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
  pthread_mutex_lock (&app->mutex);
  while (app->flags & APP_FLAG_WAITING)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}
static void onInputQueueCreated (ANativeActivity *UNUSED (act), AInputQueue *queue) {
  pthread_mutex_lock (&app->mutex);
  app->flags |= APP_FLAG_WAITING;
  pthread_mutex_unlock (&app->mutex);
  write_cmd.cmd = APP_CMD_INPUT_UPDATE;
  write_cmd.data = (void *)queue;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
  pthread_mutex_lock (&app->mutex);
  while (app->flags & APP_FLAG_WAITING)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}
static void onConfigurationChanged (ANativeActivity *UNUSED (act)) {
  write_cmd.cmd = APP_CMD_CONFIG_CHANGED;
  write_cmd.data = (void *)act->assetManager;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
}
static void onLowMemory (ANativeActivity *UNUSED (act)) {
  write_cmd.cmd = APP_CMD_LOW_MEMORY;
  write_cmd.data = NULL;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
}
static void onWindowFocusChanged (ANativeActivity *UNUSED (act), int f) {
  pthread_mutex_lock (&app->mutex);
  app->flags |= APP_FLAG_WAITING;
  pthread_mutex_unlock (&app->mutex);
  write_cmd.cmd = APP_CMD_FOCUS_CHANGED;
  intptr_t focus = f;
  write_cmd.data = (void *)focus;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
  pthread_mutex_lock (&app->mutex);
  while (app->flags & APP_FLAG_WAITING)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}
static void onNativeWindowResized (ANativeActivity *UNUSED (act), ANativeWindow *UNUSED (window)) {
  write_cmd.cmd = APP_CMD_WINDOW_RESIZED;
  write_cmd.data = NULL;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
}
static void onNativeWindowRedrawNeeded (ANativeActivity *UNUSED (act), ANativeWindow *UNUSED (window)) {
  write_cmd.cmd = APP_CMD_WINDOW_REDRAW_NEEDED;
  write_cmd.data = NULL;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
}
static void onContentRectChanged (ANativeActivity *UNUSED (act), const ARect *UNUSED (window)) {
  write_cmd.cmd = APP_CMD_CONTENT_RECT_CHANGED;
  write_cmd.data = NULL;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
}
static void *onSaveInstanceState (ANativeActivity *UNUSED (act), size_t *outLen) {
  write_cmd.cmd = APP_CMD_SAVE_STATE;
  write_cmd.data = NULL;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
  *outLen = 0;
  return NULL;
}
static void onNativeWindowDestroyed (ANativeActivity *UNUSED (act), ANativeWindow *UNUSED (window)) {
  pthread_mutex_lock (&app->mutex);
  app->flags |= APP_FLAG_WAITING;
  pthread_mutex_unlock (&app->mutex);
  write_cmd.cmd = APP_CMD_WINDOW_UPDATE;
  write_cmd.data = NULL;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
  pthread_mutex_lock (&app->mutex);
  while (app->flags & APP_FLAG_WAITING)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}
static void onInputQueueDestroyed (ANativeActivity *UNUSED (act), AInputQueue *UNUSED (queue)) {
  pthread_mutex_lock (&app->mutex);
  app->flags |= APP_FLAG_WAITING;
  pthread_mutex_unlock (&app->mutex);
  write_cmd.cmd = APP_CMD_INPUT_UPDATE;
  write_cmd.data = NULL;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
  pthread_mutex_lock (&app->mutex);
  while (app->flags & APP_FLAG_WAITING)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}
static void onPause (ANativeActivity *UNUSED (act)) {
  write_cmd.cmd = APP_CMD_PAUSE;
  write_cmd.data = NULL;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
}
static void onStop (ANativeActivity *UNUSED (act)) {
  write_cmd.cmd = APP_CMD_STOP;
  write_cmd.data = NULL;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
}
static void onDestroy (ANativeActivity *UNUSED (act)) {
  write_cmd.cmd = APP_CMD_DESTROY;
  write_cmd.data = NULL;
  while (write (app->msgwrite, &write_cmd, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe))
    LOGE ("cannot write on pipe , %s", strerror (errno));
  pthread_mutex_lock (&app->mutex);
  while (!(app->flags & APP_FLAG_DESTROYED))
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
  close (app->msgread);
  close (app->msgwrite);
  pthread_cond_destroy (&app->cond);
  pthread_mutex_destroy (&app->mutex);
  free_mem (app);
  app = NULL;
}

void ANativeActivity_onCreate (ANativeActivity *act, void *UNUSED (savedata), size_t UNUSED (save_len)) {
  // initialize application
  app = (struct android_app *)new_imem (sizeof (struct android_app));
  pthread_mutex_init (&app->mutex, NULL);
  pthread_cond_init (&app->cond, NULL);
  while (pipe (&app->msgread) == -1) {
    // force loop to provide pipe
    LOGE ("Failed to create pipe, %s", strerror (errno));
  }

  // initialize lifecycle
  act->callbacks->onStart = onStart;
  act->callbacks->onResume = onResume;
  act->callbacks->onNativeWindowCreated = onNativeWindowCreated;
  act->callbacks->onInputQueueCreated = onInputQueueCreated;
  act->callbacks->onConfigurationChanged = onConfigurationChanged;
  act->callbacks->onLowMemory = onLowMemory;
  act->callbacks->onWindowFocusChanged = onWindowFocusChanged;
  act->callbacks->onNativeWindowResized = onNativeWindowResized;
  act->callbacks->onNativeWindowRedrawNeeded = onNativeWindowRedrawNeeded;
  act->callbacks->onContentRectChanged = onContentRectChanged;
  act->callbacks->onSaveInstanceState = onSaveInstanceState;
  act->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
  act->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
  act->callbacks->onPause = onPause;
  act->callbacks->onStop = onStop;
  act->callbacks->onDestroy = onDestroy;

  // start thread game
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  pthread_create (&app->thread, &attr, android_app_entry, activity);
  pthread_attr_destroy (&attr);
}

// native MainActivity.java
JNIEXPORT void Java_com_ariasaproject_technowar_MainActivity_insetNative (JNIEnv *UNUSED (env), jobject UNUSED (o), jint left, jint top, jint right, jint bottom) {
  if (app == NULL) return;
  android_graphicsManager_resizeInsets(left, top, right, bottom);
}
