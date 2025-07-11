#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>

#include <errno.h>
#include <jni.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
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
  STATE_APP_DELAY_UNWINDOW = 16,
  STATE_APP_DELAY_UNRUNNING = 32,
};

struct android_app {
  ANativeActivity *activity;
  AConfiguration *config;
  ARect contentRect;

  int8_t cmdState;
  int msgread, msgwrite;
  int stateApp;

  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_t thread;
} *app = NULL;

enum {
  APP_CMD_NONE,
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

static int process_cmd(int fd, int UNUSED_ARG(event), void *UNUSED_ARG(data)) {
  static struct msg_pipe rmsg;
  if (read(fd, &rmsg, sizeof(struct msg_pipe)) != sizeof(struct msg_pipe)) {
    LOGE("No data on command pipe!");
    return 0;
  }
  switch (rmsg.cmd) {
  case APP_CMD_SAVE_STATE:
  case APP_CMD_WINDOW_REDRAW:
  case APP_CMD_START:
  case APP_CMD_STOP:
    // nothing important
    break;
  case APP_CMD_INPUT_CREATED:
    android_inputManager_createInputQueue((AInputQueue *)rmsg.data);
    break;
  case APP_CMD_INPUT_DESTROYED:
    android_inputManager_destroyInputQueue();
    break;
  case APP_CMD_WINDOW_CREATED:
    android_graphicsManager_onWindowCreate((ANativeWindow *)rmsg.data);
    app->stateApp |= STATE_APP_WINDOW;
    break;
  case APP_CMD_RESUME:
    app->stateApp |= STATE_APP_RUNNING;
    break;
  case APP_CMD_CONTENT_RECT_CHANGED:
    android_graphicsManager_onWindowResize();
    break;
  case APP_CMD_WINDOW_RESIZE:
    android_graphicsManager_onWindowResizeDisplay();
    break;
  case APP_CMD_GAINED_FOCUS:
    android_inputManager_enableSensor();
    break;
  case APP_CMD_LOST_FOCUS:
    android_inputManager_disableSensor();
    break;
  case APP_CMD_CONFIG_CHANGED:
    AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
    break;
  case APP_CMD_DESTROY:
    app->stateApp &= ~STATE_APP_INIT;
    break;
  }
  app->cmdState = rmsg.cmd;
  return 1;
}

static void *android_app_entry(void *UNUSED_ARG(param)) {
  app->config = AConfiguration_new();
  AConfiguration_fromAssetManager(app->config, app->activity->assetManager);

  ALooper *looper = ALooper_prepare(0);
  ALooper_addFd(looper, app->msgread, 1, ALOOPER_EVENT_INPUT, process_cmd, NULL);

  engine_init();
  android_inputManager_init(looper);
  android_graphicsManager_init();
  android_assetManager_init(app->activity->assetManager);

  pthread_mutex_lock(&app->mutex);
  app->stateApp |= STATE_APP_INIT;
  pthread_cond_signal(&app->cond);
  pthread_mutex_unlock(&app->mutex);

  while (app->stateApp & STATE_APP_INIT) {
    int block = (!(app->stateApp & STATE_APP_WINDOW) || !(app->stateApp & STATE_APP_RUNNING));

    if (ALooper_pollOnce(block * -1, NULL, NULL, NULL) == ALOOPER_POLL_ERROR) {
      LOGE("ALooper_pollOnce returned an error");
    }

    if ((app->stateApp & STATE_APP_WINDOW) &&
        (app->stateApp & STATE_APP_RUNNING) &&
        android_graphicsManager_preRender()) {
      Main_update();
      if ((app->cmdState == APP_CMD_WINDOW_DESTROYED) ||
          (app->cmdState == APP_CMD_PAUSE)) {
        Main_pause();
      }
      android_graphicsManager_postRender();
    }
    switch (app->cmdState) {
    case APP_CMD_WINDOW_DESTROYED:
      android_graphicsManager_onWindowDestroy();
      app->stateApp &= ~STATE_APP_WINDOW;
      break;
    case APP_CMD_PAUSE:
      app->stateApp &= ~STATE_APP_RUNNING;
    }
    pthread_mutex_lock(&app->mutex);
    pthread_cond_signal(&app->cond);
    pthread_mutex_unlock(&app->mutex);
  }
  Main_term();
  android_graphicsManager_term();
  android_inputManager_term();
  android_assetManager_term();

  AConfiguration_delete(app->config);
  pthread_mutex_lock(&app->mutex);
  app->stateApp |= STATE_APP_DESTROY;
  pthread_cond_signal(&app->cond);
  pthread_mutex_unlock(&app->mutex);
  // Can't touch app object after this.
  return NULL;
}

static struct msg_pipe wmsg;
static void android_app_write_cmd(int8_t cmd, void *data) {
  wmsg.cmd = cmd;
  wmsg.data = data;
  while (write(app->msgwrite, &wmsg, sizeof(struct msg_pipe)) != sizeof(struct msg_pipe))
    LOGE("Failure writing android_app cmd: %s\n", strerror(errno));
  pthread_mutex_lock(&app->mutex);
  pthread_cond_wait(&app->cond, &app->mutex);
  pthread_mutex_unlock(&app->mutex);
}

static void onDestroy(ANativeActivity *UNUSED_ARG(activity)) {
  wmsg.cmd = APP_CMD_DESTROY;
  wmsg.data = NULL;
  if (write(app->msgwrite, &wmsg, sizeof(struct msg_pipe)) != sizeof(struct msg_pipe))
    LOGE("Failure writing android_app cmd: %s\n", strerror(errno));
  pthread_mutex_lock(&app->mutex);
  while (!(app->stateApp & STATE_APP_DESTROY))
    pthread_cond_wait(&app->cond, &app->mutex);
  pthread_mutex_unlock(&app->mutex);

  close(app->msgread);
  close(app->msgwrite);
  pthread_cond_destroy(&app->cond);
  pthread_mutex_destroy(&app->mutex);
  free(app);
  app = NULL;
}
static void onStart(ANativeActivity *UNUSED_ARG(activity)) {
  android_app_write_cmd(APP_CMD_START, NULL);
}
static void onResume(ANativeActivity *UNUSED_ARG(activity)) {
  android_app_write_cmd(APP_CMD_RESUME, NULL);
}
static void *onSaveInstanceState(ANativeActivity *UNUSED_ARG(activity), size_t *outLen) {
  *outLen = sizeof(struct core);
  void *savedState = malloc(*outLen);
  memcpy(savedState, &core_cache, *outLen);
  android_app_write_cmd(APP_CMD_SAVE_STATE, NULL);
  return savedState;
}
static void onPause(ANativeActivity *UNUSED_ARG(activity)) {
  android_app_write_cmd(APP_CMD_PAUSE, NULL);
}
static void onStop(ANativeActivity *UNUSED_ARG(activity)) {
  android_app_write_cmd(APP_CMD_STOP, NULL);
}
static void onConfigurationChanged(ANativeActivity *UNUSED_ARG(activity)) {
  android_app_write_cmd(APP_CMD_CONFIG_CHANGED, NULL);
}
static void onLowMemory(ANativeActivity *UNUSED_ARG(activity)) {
  android_app_write_cmd(APP_CMD_LOW_MEMORY, NULL);
}
static void onWindowFocusChanged(ANativeActivity *UNUSED_ARG(activity), int focused) {
  android_app_write_cmd(focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS, NULL);
}
static void onContentRectChanged(ANativeActivity *UNUSED_ARG(activity), const ARect *rect) {
  android_app_write_cmd(APP_CMD_CONTENT_RECT_CHANGED, (void *)rect);
}
static void onNativeWindowResized(ANativeActivity *UNUSED_ARG(activity), ANativeWindow *UNUSED_ARG(window)) {
  android_app_write_cmd(APP_CMD_WINDOW_RESIZE, NULL);
}
static void onNativeWindowRedrawNeeded(ANativeActivity *UNUSED_ARG(activity), ANativeWindow *UNUSED_ARG(window)) {
  android_app_write_cmd(APP_CMD_WINDOW_REDRAW, NULL);
}
static void onNativeWindowCreated(ANativeActivity *UNUSED_ARG(activity), ANativeWindow *window) {
  android_app_write_cmd(APP_CMD_WINDOW_CREATED, (void *)window);
}
static void onNativeWindowDestroyed(ANativeActivity *UNUSED_ARG(activity), ANativeWindow *UNUSED_ARG(window)) {
  android_app_write_cmd(APP_CMD_WINDOW_DESTROYED, NULL);
}
static void onInputQueueCreated(ANativeActivity *UNUSED_ARG(activity), AInputQueue *queue) {
  android_app_write_cmd(APP_CMD_INPUT_CREATED, (void *)queue);
}
static void onInputQueueDestroyed(ANativeActivity *UNUSED_ARG(activity), AInputQueue *UNUSED_ARG(queue)) {
  android_app_write_cmd(APP_CMD_INPUT_DESTROYED, NULL);
}

void ANativeActivity_onCreate(ANativeActivity *activity, void *savedState, size_t savedStateSize) {
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

  app = (struct android_app *)calloc(1, sizeof(struct android_app));
  app->activity = activity;
  
  pthread_mutex_init(&app->mutex, NULL);
  pthread_cond_init(&app->cond, NULL);

  if (savedState != NULL && savedStateSize == sizeof(struct core))
    memcpy(&core_cache, savedState, sizeof(struct core));
  
  if (pipe(&app->msgread))
    LOGE("could not create pipe: %s", strerror(errno));

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&app->thread, &attr, android_app_entry, NULL);
  pthread_attr_destroy(&attr);
  
  pthread_mutex_lock(&app->mutex);
  pthread_cond_wait(&app->cond, &app->mutex);
  pthread_mutex_unlock(&app->mutex);
}
// used by log.h
#ifdef _DEBUG
void toastMessage(const char x, ...) {
  if (!app) return;
	static char msg[512];
  va_list args;
  va_start(args, x);
  vsnprintf(msg, 512, x, args);
  va_end(args);
  
  JNIEnv *env;
  static jclass cls = 0;
  static jmethodID id = 0;
  if (JNI_OK == (*(app->activity->vm))->AttachCurrentThread(app->activity->vm, &env, NULL)) {
    if(!cls) cls = (*env)->GetObjectClass(env, app->activity->clazz);
    if(!id) id = (*env)->GetMethodID(env, cls, "showToast", "(Ljava/lang/String;)V");
    jstring jmsg = (*env)->NewStringUTF(env, msg);
    (*env)->CallVoidMethod(env, app->activity->clazz, id, jmsg);
    (*(app->activity->vm))->AttachCurrentThread(app->activity->vm);
  }
}
#endif


// native MainActivity.java
JNIEXPORT void Java_com_ariasaproject_technowar_MainActivity_insetNative(JNIEnv *env, jobject o, jint left, jint top, jint right, jint bottom) {
  UNUSED(env);
  UNUSED(o);
  android_graphicsManager_resizeInsets(left, top, right, bottom);
}
