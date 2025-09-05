#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>

#include <errno.h>
#include <jni.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "core.h"
#include "engine.h"
#include "log.h"
#include "manager.h"

void (*androidGraphics_onWindowCreate)(void *) = NULL;
void (*androidGraphics_onWindowDestroy)(void) = NULL;
void (*androidGraphics_onWindowResizeDisplay)(void) = NULL;
void (*androidGraphics_onWindowResize)(void) = NULL;
void (*androidGraphics_resizeInsets)(float, float, float, float) = NULL;
int (*androidGraphics_preRender)(void) = NULL;
void (*androidGraphics_postRender)(void) = NULL;
void (*androidGraphics_term)(void) = NULL;

struct msg_pipe {
  int8_t cmd;
  void *data;
};

enum {
  STATE_APP_INIT = 1,
  STATE_APP_WINDOW = 2,
  STATE_APP_RUNNING = 4,
  STATE_APP_DESTROY = 8
};

struct android_app {
  void *userData;
  ANativeActivity *activity;
  AConfiguration *config;
  struct core *savedState;
  ARect contentRect;

  int8_t delayed_cmdState;
  int8_t cmdState;

  pthread_mutex_t mutex;
  pthread_cond_t cond;

  int msgread, msgwrite;

  pthread_t thread;

  int stateApp;
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
    break;
  case APP_CMD_INPUT_CREATED:
    androidInput_createInputQueue((AInputQueue *)rmsg.data);
    break;
  case APP_CMD_INPUT_DESTROYED:
    androidInput_destroyInputQueue();
    break;
  case APP_CMD_WINDOW_CREATED:
    androidGraphics_onWindowCreate((ANativeWindow *)rmsg.data);
    app->stateApp |= STATE_APP_WINDOW;
    break;
  case APP_CMD_RESUME:
    app->stateApp |= STATE_APP_RUNNING;
    break;
  case APP_CMD_CONTENT_RECT_CHANGED:
    androidGraphics_onWindowResize();
    break;
  case APP_CMD_WINDOW_RESIZE:
    androidGraphics_onWindowResizeDisplay();
    break;
  case APP_CMD_GAINED_FOCUS:
    androidInput_enableSensor();
    break;
  case APP_CMD_LOST_FOCUS:
    androidInput_disableSensor();
    break;
  case APP_CMD_CONFIG_CHANGED:
    AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
    break;
  case APP_CMD_DESTROY:
    app->stateApp &= ~STATE_APP_INIT;
    break;
  }
  app->delayed_cmdState = rmsg.cmd;
  return 1;
}

static void *android_app_entry(void *UNUSED_ARG(param)) {
  app->config = AConfiguration_new();
  AConfiguration_fromAssetManager(app->config, app->activity->assetManager);

  ALooper *looper = ALooper_prepare(0);
  ALooper_addFd(looper, app->msgread, 1, ALOOPER_EVENT_INPUT, process_cmd, NULL);

  androidAssetManager_init(app->activity->assetManager);
  androidTimerManager_init();
  androidInput_init(looper);
  opengles_init();

  pthread_mutex_lock(&app->mutex);
  app->stateApp |= STATE_APP_INIT;
  pthread_cond_signal(&app->cond);
  pthread_mutex_unlock(&app->mutex);

  while (app->stateApp & STATE_APP_INIT) {
    int ready = app->stateApp & (STATE_APP_WINDOW | STATE_APP_RUNNING);
    if (ALooper_pollOnce(!ready * -1, NULL, NULL, NULL) == ALOOPER_POLL_ERROR)
      LOGE("ALooper_pollOnce returned an error");
    if (ready && androidGraphics_preRender()) {
      Main_update();
      if ((app->delayed_cmdState == APP_CMD_WINDOW_DESTROYED) ||
          (app->delayed_cmdState == APP_CMD_PAUSE)) {
        Main_pause();
      }
      androidTimerManager_onFrame();
      androidGraphics_postRender();
    }
    switch (app->delayed_cmdState) {
    case APP_CMD_WINDOW_DESTROYED:
      androidGraphics_onWindowDestroy();
      app->stateApp &= ~STATE_APP_WINDOW;
      break;
    case APP_CMD_PAUSE:
      app->stateApp &= ~STATE_APP_RUNNING;
    }
    if (app->cmdState != app->delayed_cmdState) {
      pthread_mutex_lock(&app->mutex);
      app->cmdState = app->delayed_cmdState;
      pthread_cond_signal(&app->cond);
      pthread_mutex_unlock(&app->mutex);
    }
  }
  Main_term();
  androidGraphics_term();
  androidInput_term();
  androidTimerManager_term();
  androidAssetManager_term();

  memset(&global_engine, 0, sizeof(struct engine));

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
  if (write(app->msgwrite, &wmsg, sizeof(struct msg_pipe)) != sizeof(struct msg_pipe))
    LOGE("Failure writing android_app cmd: %s\n", strerror(errno));
  pthread_mutex_lock(&app->mutex);
  while (app->cmdState != cmd)
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
  *outLen = core_stateLength();
  void *savedState = core_stateSaved();
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

  if (savedState != NULL && savedStateSize == core_stateLength()) {
    core_stateLoad(savedState);
  }
  if (pipe(&app->msgread)) {
    ANativeActivity_finish(activity);
    return;
  }

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  if (pthread_create(&app->thread, &attr, android_app_entry, NULL)) {
    ANativeActivity_finish(activity);
    return;
  }

  // Wait for thread to start.
  pthread_mutex_lock(&app->mutex);
  while (!(app->stateApp & STATE_APP_INIT))
    pthread_cond_wait(&app->cond, &app->mutex);
  pthread_mutex_unlock(&app->mutex);
}

#ifdef _DEBUG
void toastMessage(const char *msg, ...) {
  if (!app)
    return;

  static char temp[512];
  va_list args;
  va_start(args, msg);
  vsnprintf(temp, 512, msg, args);
  va_end(args);

  JavaVM *vm = app->activity->vm;
  JNIEnv *env;
  static jmethodID id = 0;
  if ((*vm)->AttachCurrentThread(vm, &env, NULL) != JNI_OK) {
    if (!id) {
      jclass cls = (*env)->GetObjectClass(env, app->activity->clazz);
      id = (*env)->GetMethodID(env, cls, "showToast", "(Ljava/lang/String;)V");
    }
    jstring jmsg = (*env)->NewStringUTF(env, temp);
    (*env)->CallVoidMethod(env, app->activity->clazz, id, jmsg);
    (*vm)->DetachCurrentThread(vm);
  }
}
void finish() {
  if (!app)
    return;
  ANativeActivity_finish(app->activity);
}
#endif // _DEBUG

// native MainActivity.java

JNIEXPORT void JNICALL Java_com_ariasaproject_technowar_MainActivity_insetNative(JNIEnv *env, jobject o, jint left, jint top, jint right, jint bottom) {
  UNUSED(env), UNUSED(o);
  androidGraphics_resizeInsets(left, top, right, bottom);
}
