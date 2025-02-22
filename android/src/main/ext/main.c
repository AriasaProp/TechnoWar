#include <android/choreographer.h>
#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/set_abort_message.h>

#include <assert.h>
#include <errno.h>
#include <jni.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

#include "core.h"
#include "engine.h"
#include "log.h"
#include "manager.h"
#include "util.h"

struct msg_pipe {
  int8_t cmd;
  void *data;
};

enum {
  STATE_APP_INIT = 1,
  STATE_APP_WINDOW = 2,
  STATE_APP_RUNNING = 4,
  STATE_APP_DESTROY = 8,
};

struct android_app {
  void *userData;
  ANativeActivity *activity;
  AConfiguration *config;
  struct core *savedState;
  ALooper *looper;
  ARect contentRect;

  int8_t cmdState;

  pthread_mutex_t mutex;
  pthread_cond_t cond;

  int msgread, msgwrite;

  pthread_t thread;

  int stateApp;
} *app = NULL;

enum {
  APP_CMD_INPUT_CREATED,
  APP_CMD_INPUT_DESTROYED,
  APP_CMD_WINDOW_CREATED,
  APP_CMD_WINDOW_DESTROYED,
  APP_CMD_WINDOW_RESIZE,
  APP_CMD_WINDOW_REDRAW,
  APP_CMD_CONTENT_RECT_CHANGED,
  APP_CMD_GAINED_FOCUS,
  APP_CMD_LOST_FOCUS,
  APP_CMD_CONFIG_CHANGED,
  APP_CMD_LOW_MEMORY,
  APP_CMD_START,
  APP_CMD_RESUME,
  APP_CMD_SAVE_STATE,
  APP_CMD_PAUSE,
  APP_CMD_STOP,
  APP_CMD_DESTROY,
};

static void Tick (long UNUSED (timeout), void *UNUSED (data)) {
  if (!(app->stateApp & STATE_APP_WINDOW) || !(app->stateApp & STATE_APP_RUNNING))
  	return;
  AChoreographer_postFrameCallback (AChoreographer_getInstance (), Tick, NULL);
  if (!android_graphicsManager_preRender ())
  	return;

  Main_update ();

  android_graphicsManager_postRender ();
}
static int process_cmd (int fd, int UNUSED (event), void *UNUSED (data)) {
  static struct msg_pipe rmsg;
  if (read (fd, &rmsg, sizeof (struct msg_pipe)) == sizeof (struct msg_pipe)) {
    switch (rmsg.cmd) {
    case APP_CMD_SAVE_STATE:
      break;
    case APP_CMD_INPUT_CREATED:
      android_inputManager_createInputQueue ((AInputQueue *)rmsg.data);
      break;
    case APP_CMD_INPUT_DESTROYED:
      android_inputManager_destroyInputQueue ();
      break;
    case APP_CMD_WINDOW_CREATED:
      android_graphicsManager_onWindowCreate ((ANativeWindow *)rmsg.data);
      app->stateApp |= STATE_APP_WINDOW;
      Tick (0, NULL);
      break;
    case APP_CMD_WINDOW_DESTROYED:
      android_graphicsManager_onWindowDestroy ();
      app->stateApp &= ~STATE_APP_WINDOW;
      break;
    case APP_CMD_RESUME:
      // post
      app->stateApp |= STATE_APP_RUNNING;
      Tick (0, NULL);
      break;
    case APP_CMD_PAUSE:
      app->stateApp &= ~STATE_APP_RUNNING;
      break;
    case APP_CMD_CONTENT_RECT_CHANGED:
      android_graphicsManager_onWindowResize ();
      break;
    case APP_CMD_WINDOW_RESIZE:
      android_graphicsManager_onWindowResizeDisplay ();
      break;
    case APP_CMD_WINDOW_REDRAW:
      break;
    case APP_CMD_START:
    case APP_CMD_STOP:
      break;
    case APP_CMD_GAINED_FOCUS:
      android_inputManager_enableSensor ();
      break;
    case APP_CMD_LOST_FOCUS:
      android_inputManager_disableSensor ();
      break;
    case APP_CMD_CONFIG_CHANGED:
      AConfiguration_fromAssetManager (app->config, (AAssetManager *)rmsg.data);
      break;
    case APP_CMD_DESTROY:
      app->stateApp &= ~STATE_APP_INIT;
      break;
    }
    pthread_mutex_lock (&app->mutex);
    app->cmdState = rmsg.cmd;
    pthread_cond_broadcast (&app->cond);
    pthread_mutex_unlock (&app->mutex);
    return 1;
  } else {
    LOGE ("No data on command pipe!");
    return 0;
  }
}

static void *android_app_entry (void *param) {
  app->config = AConfiguration_new ();
  ANativeActivity *activity = (ANativeActivity *)param;
  AConfiguration_fromAssetManager (app->config, activity->assetManager);

  app->looper = ALooper_prepare (0);
  ALooper_addFd (app->looper, app->msgread, 1, ALOOPER_EVENT_INPUT, process_cmd, NULL);

  engine_init ();
  android_inputManager_init (app->looper);
  android_graphicsManager_init ();

  pthread_mutex_lock (&app->mutex);
  app->stateApp |= STATE_APP_INIT;
  pthread_cond_broadcast (&app->cond);
  pthread_mutex_unlock (&app->mutex);

  while (app->stateApp & STATE_APP_INIT) {
    if (ALooper_pollOnce (-1, NULL, NULL, NULL) == ALOOPER_POLL_ERROR) {
      LOGE ("ALooper_pollOnce returned an error");
    }
  }
  Main_term ();
  android_graphicsManager_term ();
  android_inputManager_term ();

  AConfiguration_delete (app->config);
  pthread_mutex_lock (&app->mutex);
  app->stateApp |= STATE_APP_DESTROY;
  pthread_cond_broadcast (&app->cond);
  pthread_mutex_unlock (&app->mutex);
  // Can't touch app object after this.
  return NULL;
}

static struct msg_pipe wmsg;
static void android_app_write_cmd (int8_t cmd, void *data) {
  static struct msg_pipe wmsg;
  wmsg.cmd = cmd;
  wmsg.data = data;
  if (write (app->msgwrite, &wmsg, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe)) {
    LOGE ("Failure writing android_app cmd: %s\n", strerror (errno));
  }
  pthread_mutex_lock (&app->mutex);
  while (app->cmdState != cmd) {
    pthread_cond_wait (&app->cond, &app->mutex);
  }
  pthread_cond_broadcast (&app->cond);
  pthread_mutex_unlock (&app->mutex);
}

static void onDestroy (ANativeActivity *UNUSED (activity)) {
  wmsg.cmd = APP_CMD_DESTROY;
  wmsg.data = NULL;
  if (write (app->msgwrite, &wmsg, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe)) {
    LOGE ("Failure writing android_app cmd: %s\n", strerror (errno));
  }
  pthread_mutex_lock (&app->mutex);
  while (!(app->stateApp & STATE_APP_DESTROY)) {
    pthread_cond_wait (&app->cond, &app->mutex);
  }
  pthread_mutex_unlock (&app->mutex);

  close (app->msgread);
  close (app->msgwrite);
  pthread_cond_destroy (&app->cond);
  pthread_mutex_destroy (&app->mutex);
  free_mem (app);
  app = NULL;
}
static void onStart (ANativeActivity *UNUSED (activity)) {
  android_app_write_cmd (APP_CMD_START, NULL);
}
static void onResume (ANativeActivity *UNUSED (activity)) {
  android_app_write_cmd (APP_CMD_RESUME, NULL);
}
static void *onSaveInstanceState (ANativeActivity *UNUSED (activity), size_t *outLen) {
  *outLen = sizeof (struct core);
  void *savedState = malloc (*outLen);
  memcpy (savedState, &core_cache, *outLen);
  android_app_write_cmd (APP_CMD_SAVE_STATE, NULL);
  return savedState;
}
static void onPause (ANativeActivity *UNUSED (activity)) {
  android_app_write_cmd (APP_CMD_PAUSE, NULL);
}
static void onStop (ANativeActivity *UNUSED (activity)) {
  android_app_write_cmd (APP_CMD_STOP, NULL);
}
static void onConfigurationChanged (ANativeActivity *activity) {
  android_app_write_cmd (APP_CMD_CONFIG_CHANGED, (void *)activity->assetManager);
}
static void onLowMemory (ANativeActivity *UNUSED (activity)) {
  android_app_write_cmd (APP_CMD_LOW_MEMORY, NULL);
}
static void onWindowFocusChanged (ANativeActivity *UNUSED (activity), int focused) {
  android_app_write_cmd (focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS, NULL);
}
static void onContentRectChanged (ANativeActivity *UNUSED (activity), const ARect *rect) {
  android_app_write_cmd (APP_CMD_CONTENT_RECT_CHANGED, (void *)rect);
}
static void onNativeWindowResized (ANativeActivity *UNUSED (activity), ANativeWindow *UNUSED (window)) {
  android_app_write_cmd (APP_CMD_WINDOW_RESIZE, NULL);
}
static void onNativeWindowRedrawNeeded (ANativeActivity *UNUSED (activity), ANativeWindow *UNUSED (window)) {
  android_app_write_cmd (APP_CMD_WINDOW_REDRAW, NULL);
}
static void onNativeWindowCreated (ANativeActivity *UNUSED (activity), ANativeWindow *window) {
  android_app_write_cmd (APP_CMD_WINDOW_CREATED, (void *)window);
}
static void onNativeWindowDestroyed (ANativeActivity *UNUSED (activity), ANativeWindow *UNUSED (window)) {
  android_app_write_cmd (APP_CMD_WINDOW_DESTROYED, NULL);
}
static void onInputQueueCreated (ANativeActivity *UNUSED (activity), AInputQueue *queue) {
  android_app_write_cmd (APP_CMD_INPUT_CREATED, (void *)queue);
}
static void onInputQueueDestroyed (ANativeActivity *UNUSED (activity), AInputQueue *UNUSED (queue)) {
  android_app_write_cmd (APP_CMD_INPUT_DESTROYED, NULL);
}

void ANativeActivity_onCreate (ANativeActivity *activity, void *savedState, size_t savedStateSize) {
  activity->callbacks->onDestroy = onDestroy;
  activity->callbacks->onStart = onStart;
  activity->callbacks->onResume = onResume;
  activity->callbacks->onSaveInstanceState = onSaveInstanceState;
  activity->callbacks->onPause = onPause;
  activity->callbacks->onStop = onStop;
  activity->callbacks->onConfigurationChanged = onConfigurationChanged;
  activity->callbacks->onLowMemory = onLowMemory;
  activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
  activity->callbacks->onContentRectChanged = onContentRectChanged;
  activity->callbacks->onNativeWindowResized = onNativeWindowResized;
  activity->callbacks->onNativeWindowRedrawNeeded = onNativeWindowRedrawNeeded;
  activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
  activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
  activity->callbacks->onInputQueueCreated = onInputQueueCreated;
  activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;

  app = (struct android_app *)new_imem (sizeof (struct android_app));
  app->activity = activity;

  pthread_mutex_init (&app->mutex, NULL);
  pthread_cond_init (&app->cond, NULL);

  if (savedState != NULL && savedStateSize == sizeof (struct core)) {
    memcpy (&core_cache, savedState, sizeof (struct core));
  }
  if (pipe (&app->msgread)) {
    LOGE ("could not create pipe: %s", strerror (errno));
    return;
  }

  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  pthread_create (&app->thread, &attr, android_app_entry, activity);

  // Wait for thread to start.
  pthread_mutex_lock (&app->mutex);
  while (!(app->stateApp & STATE_APP_INIT)) {
    pthread_cond_wait (&app->cond, &app->mutex);
  }
  pthread_mutex_unlock (&app->mutex);
}
extern char extGLMsg[1024];
// native MainActivity.java
JNIEXPORT void Java_com_ariasaproject_technowar_MainActivity_insetNative (JNIEnv *env, jobject o, jint left, jint top, jint right, jint bottom) {
  android_graphicsManager_resizeInsets (left, top, right, bottom);
  if (extGLMsg[0]) {
	  jclass cls = (*env)->GetObjectClass(env, o);
	  jstring jmsg = (*env)->NewStringUTF(env, extGLMsg);
	  jmethodID id = (*env)->GetMethodID(env, cls, "showToast", "(Ljava/lang/String;)V");
	  (*env)->CallVoidMethod(env, o, id, jmsg);
  }
}
