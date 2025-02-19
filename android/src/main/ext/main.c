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
#include <sys/socket.h>
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
  APP_REQ_ACC,
};
enum {
  APP_FLAG_WAITING = 1,
  APP_FLAG_DESTROYED = 2,
};

struct android_app {
  int flags;
  int msgread, msgwrite;
  pthread_t thread;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} *app = NULL;

struct msg_pipe {
  enum APP_CMD cmd;
  void *data;
};

static void *android_app_entry (void *n) {
  engine_init ();
  ANativeActivity *act = (ANativeActivity *)n;
  AConfiguration *aconfig = AConfiguration_new ();
  AConfiguration_fromAssetManager (aconfig, act->assetManager);
  ALooper *looper = ALooper_prepare (ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
  ALooper_addFd (looper, app->msgread, 1, ALOOPER_EVENT_INPUT, NULL, NULL);
  int animating = 0;
  android_inputManager_init (looper);
  android_graphicsManager_init ();
  struct msg_pipe read_cmd = {APP_CMD_CREATE, NULL};
  do {
    if (ALooper_pollOnce (!(animating & 1) * -1, NULL, NULL, NULL) == 1) {
      // activity handler
      read (app->msgwrite, &read_cmd, sizeof (struct msg_pipe));
      switch (read_cmd.cmd) {
      case APP_CMD_WINDOW_UPDATE:
        android_graphicsManager_onWindowChange ((ANativeWindow *)read_cmd.data);
        if (read_cmd.data != NULL) {
          animating |= 1;
        } else {
          animating &= ~1;
        }
        break;
      case APP_CMD_FOCUS_CHANGED:
        android_inputManager_switchSensor (read_cmd.data);
        break;
      case APP_CMD_INPUT_UPDATE:
        android_inputManager_setInputQueue (looper, (AInputQueue *)read_cmd.data);
        break;
      case APP_CMD_CONFIG_CHANGED:
        AConfiguration_fromAssetManager (aconfig, (AAssetManager *)read_cmd.data);
        break;
      case APP_CMD_CONTENT_RECT_CHANGED:
        android_graphicsManager_onWindowResize ();
        break;
      case APP_CMD_WINDOW_RESIZED:
        android_graphicsManager_onWindowResizeDisplay ();
        break;
      case APP_CMD_DESTROY:
        animating |= 2;
        continue;
      case APP_CMD_PAUSE:
      case APP_CMD_SAVE_STATE:
      case APP_CMD_STOP:
      case APP_CMD_START:
      case APP_CMD_LOW_MEMORY:
      case APP_CMD_WINDOW_REDRAW_NEEDED:
      case APP_CMD_RESUME:
      default:
        break;
      }
    }
    if ((animating & 1) && android_graphicsManager_preRender ()) {
      Main_update ();
      android_graphicsManager_postRender ();
    }
    if (app->flags & APP_FLAG_WAITING) {
      pthread_mutex_lock (&app->mutex);
      app->flags &= ~APP_FLAG_WAITING;
      pthread_cond_broadcast (&app->cond);
    }
    pthread_mutex_unlock (&app->mutex);
  } while ((animating & 2) != 2);

  animating = 0; // reset flags
  android_graphicsManager_term ();
  android_inputManager_term ();
  ALooper_removeFd (looper, app->msgread);
  AConfiguration_delete (aconfig);
  pthread_mutex_lock (&app->mutex);
  app->flags |= APP_FLAG_DESTROYED;
  pthread_cond_broadcast (&app->cond);
  pthread_mutex_unlock (&app->mutex);
  return NULL;
}

static struct msg_pipe main_pipe;
static void onStart (ANativeActivity *UNUSED (act)) {
  main_pipe.cmd = APP_CMD_START;
  main_pipe.data = NULL;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
}
static void onResume (ANativeActivity *UNUSED (act)) {
  main_pipe.cmd = APP_CMD_RESUME;
  main_pipe.data = NULL;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
}
static void onNativeWindowCreated (ANativeActivity *UNUSED (act), ANativeWindow *window) {
  pthread_mutex_lock (&app->mutex);
  app->flags |= APP_FLAG_WAITING;
  pthread_mutex_unlock (&app->mutex);
  main_pipe.cmd = APP_CMD_WINDOW_UPDATE;
  main_pipe.data = (void *)window;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
  pthread_mutex_lock (&app->mutex);
  while (app->flags & APP_FLAG_WAITING)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}
static void onInputQueueCreated (ANativeActivity *UNUSED (act), AInputQueue *queue) {
  pthread_mutex_lock (&app->mutex);
  app->flags |= APP_FLAG_WAITING;
  pthread_mutex_unlock (&app->mutex);
  main_pipe.cmd = APP_CMD_INPUT_UPDATE;
  main_pipe.data = (void *)queue;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
  pthread_mutex_lock (&app->mutex);
  while (app->flags & APP_FLAG_WAITING)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}
static void onConfigurationChanged (ANativeActivity *act) {
  pthread_mutex_lock (&app->mutex);
  app->flags |= APP_FLAG_WAITING;
  pthread_mutex_unlock (&app->mutex);
  main_pipe.cmd = APP_CMD_CONFIG_CHANGED;
  main_pipe.data = (void *)act->assetManager;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
  pthread_mutex_lock (&app->mutex);
  while (app->flags & APP_FLAG_WAITING)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}
static void onLowMemory (ANativeActivity *UNUSED (act)) {
  main_pipe.cmd = APP_CMD_LOW_MEMORY;
  main_pipe.data = NULL;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
}
static void onWindowFocusChanged (ANativeActivity *UNUSED (act), int f) {
  pthread_mutex_lock (&app->mutex);
  app->flags |= APP_FLAG_WAITING;
  pthread_mutex_unlock (&app->mutex);
  main_pipe.cmd = APP_CMD_FOCUS_CHANGED;
  intptr_t focus = f;
  main_pipe.data = (void *)focus;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
  pthread_mutex_lock (&app->mutex);
  while (app->flags & APP_FLAG_WAITING)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}
static void onNativeWindowResized (ANativeActivity *UNUSED (act), ANativeWindow *UNUSED (window)) {
  pthread_mutex_lock (&app->mutex);
  app->flags |= APP_FLAG_WAITING;
  pthread_mutex_unlock (&app->mutex);
  main_pipe.cmd = APP_CMD_WINDOW_RESIZED;
  main_pipe.data = NULL;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
  pthread_mutex_lock (&app->mutex);
  while (app->flags & APP_FLAG_WAITING)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}
static void onNativeWindowRedrawNeeded (ANativeActivity *UNUSED (act), ANativeWindow *UNUSED (window)) {
  main_pipe.cmd = APP_CMD_WINDOW_REDRAW_NEEDED;
  main_pipe.data = NULL;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
}
static void onContentRectChanged (ANativeActivity *UNUSED (act), const ARect *UNUSED (r)) {
  pthread_mutex_lock (&app->mutex);
  app->flags |= APP_FLAG_WAITING;
  pthread_mutex_unlock (&app->mutex);
  main_pipe.cmd = APP_CMD_CONTENT_RECT_CHANGED;
  main_pipe.data = NULL;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
  pthread_mutex_lock (&app->mutex);
  while (app->flags & APP_FLAG_WAITING)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}
static void *onSaveInstanceState (ANativeActivity *UNUSED (act), size_t *outLen) {
  main_pipe.cmd = APP_CMD_SAVE_STATE;
  main_pipe.data = NULL;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
  *outLen = 0;
  return NULL;
}
static void onNativeWindowDestroyed (ANativeActivity *UNUSED (act), ANativeWindow *UNUSED (window)) {
  pthread_mutex_lock (&app->mutex);
  app->flags |= APP_FLAG_WAITING;
  pthread_mutex_unlock (&app->mutex);
  main_pipe.cmd = APP_CMD_WINDOW_UPDATE;
  main_pipe.data = NULL;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
  pthread_mutex_lock (&app->mutex);
  while (app->flags & APP_FLAG_WAITING)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}
static void onInputQueueDestroyed (ANativeActivity *UNUSED (act), AInputQueue *UNUSED (queue)) {
  pthread_mutex_lock (&app->mutex);
  app->flags |= APP_FLAG_WAITING;
  pthread_mutex_unlock (&app->mutex);
  main_pipe.cmd = APP_CMD_INPUT_UPDATE;
  main_pipe.data = NULL;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
  pthread_mutex_lock (&app->mutex);
  while (app->flags & APP_FLAG_WAITING)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}
static void onPause (ANativeActivity *UNUSED (act)) {
  main_pipe.cmd = APP_CMD_PAUSE;
  main_pipe.data = NULL;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
}
static void onStop (ANativeActivity *UNUSED (act)) {
  main_pipe.cmd = APP_CMD_STOP;
  main_pipe.data = NULL;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
}
static void onDestroy (ANativeActivity *UNUSED (act)) {
  main_pipe.cmd = APP_CMD_DESTROY;
  main_pipe.data = NULL;
  write (app->msgwrite, &main_pipe, sizeof (struct msg_pipe));
  pthread_mutex_lock (&app->mutex);
  while (!(app->flags & APP_FLAG_DESTROYED))
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
  close (app->msgread);
  close (app->msgwrite);
  free_mem (app);
  app = NULL;
}

void ANativeActivity_onCreate (ANativeActivity *activity, void *UNUSED (savedata), size_t UNUSED (save_len)) {
  // initialize application
  struct android_app *app = (struct android_app *)new_imem (sizeof (struct android_app));
  pthread_mutex_init (&app->mutex, NULL);
  pthread_cond_init (&app->cond, NULL);

  if (pipe (&app->msgread) < 0) {
    LOGE ("could not create pipe: %s", strerror (errno));
    return NULL;
  }

  // initialize lifecycle
  activity->callbacks->onStart = onStart;
  activity->callbacks->onResume = onResume;
  activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
  activity->callbacks->onInputQueueCreated = onInputQueueCreated;
  activity->callbacks->onConfigurationChanged = onConfigurationChanged;
  activity->callbacks->onLowMemory = onLowMemory;
  activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
  activity->callbacks->onNativeWindowResized = onNativeWindowResized;
  activity->callbacks->onNativeWindowRedrawNeeded = onNativeWindowRedrawNeeded;
  activity->callbacks->onContentRectChanged = onContentRectChanged;
  activity->callbacks->onSaveInstanceState = onSaveInstanceState;
  activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
  activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
  activity->callbacks->onPause = onPause;
  activity->callbacks->onStop = onStop;
  activity->callbacks->onDestroy = onDestroy;

  // start thread game
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  pthread_create (&app->thread, &attr, android_app_entry, activity);
  pthread_attr_destroy (&attr);
}

// native MainActivity.java
JNIEXPORT void Java_com_ariasaproject_technowar_MainActivity_insetNative (JNIEnv *env, jobject o, jint left, jint top, jint right, jint bottom) {
  android_graphicsManager_resizeInsets (left, top, right, bottom);
}
