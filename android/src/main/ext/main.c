#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/choreographer.h>
#include <android/configuration.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>
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

#include "log.h"
#include "manager.h"
#include "util.h"

struct android_app;
struct android_poll_source {
  int32_t id;
  void (*process) (struct android_poll_source *source);
};
struct android_app {
  void *userData;
  void (*onAppCmd) (int32_t cmd);
  int32_t (*onInputEvent) (AInputEvent *event);
  ANativeActivity *activity;
  AConfiguration *config;
  void *savedState;
  size_t savedStateSize;
  ALooper *looper;
  AInputQueue *inputQueue;
  ANativeWindow *window;
  ARect contentRect;
  int activityState;
  int destroyRequested;

  pthread_mutex_t mutex;
  pthread_cond_t cond;

  int msgread;
  int msgwrite;

  pthread_t thread;

  struct android_poll_source cmdPollSource;
  struct android_poll_source inputPollSource;

  int running;
  int stateSaved;
  int destroyed;
  int redrawNeeded;
  AInputQueue *pendingInputQueue;
  ANativeWindow *pendingWindow;
  ARect pendingContentRect;
} *app = NULL;

enum {
  LOOPER_ID_MAIN = 1,
  LOOPER_ID_INPUT = 2,
  LOOPER_ID_USER = 3,
};

enum {
  APP_CMD_INPUT_CHANGED,
  APP_CMD_WINDOW_CHANGED,
  APP_CMD_WINDOW_RESIZED,
  APP_CMD_WINDOW_REDRAW_NEEDED,
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
int8_t android_app_read_cmd ();
void android_app_pre_exec_cmd (int8_t cmd);
void android_app_post_exec_cmd (int8_t cmd);

struct SavedState {
  float angle;
  int32_t x;
  int32_t y;
};

struct Engine {
  ASensorManager *sensorManager;
  const ASensor *accelerometerSensor;
  ASensorEventQueue *sensorEventQueue;

  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  int32_t width;
  int32_t height;
  struct SavedState state;
  int running_;
};

static void CreateSensorListener (struct Engine *engine, ALooper_callbackFunc callback) {

  engine->sensorManager = ASensorManager_getInstance ();
  if (engine->sensorManager == NULL) {
    return;
  }
  engine->accelerometerSensor = ASensorManager_getDefaultSensor (engine->sensorManager, ASENSOR_TYPE_ACCELEROMETER);
  engine->sensorEventQueue = ASensorManager_createEventQueue (engine->sensorManager, app->looper, ALOOPER_POLL_CALLBACK, callback, engine);
}

static void Pause (struct Engine *engine) { engine->running_ = 0; }
static void ScheduleNextTick (struct Engine *);
static void Tick (long UNUSED (timeout), void *data) {
  struct Engine *engine = (struct Engine *)data;
  if (!engine->running_) {
    return;
  }
  ScheduleNextTick (engine);
  engine->state.angle += .01f;
  if (engine->state.angle > 1) {
    engine->state.angle = 0;
  }
  if (engine->display == NULL) {
    // No display.
    return;
  }

  // Just fill the screen with a color.
  glClearColor (((float)engine->state.x) / engine->width, engine->state.angle, ((float)engine->state.y) / engine->height, 1);
  glClear (GL_COLOR_BUFFER_BIT);

  eglSwapBuffers (engine->display, engine->surface);
}
static void ScheduleNextTick (struct Engine *engine) {
  AChoreographer_postFrameCallback (AChoreographer_getInstance (), Tick, engine);
}
static void Resume (struct Engine *engine) {
  if (!engine->running_) {
    engine->running_ = 1;
    ScheduleNextTick (engine);
  }
}
static int engine_init_display (struct Engine *engine) {
  // initialize OpenGL ES and EGL

  /*
   * Here specify the attributes of the desired configuration.
   * Below, we select an EGLConfig with at least 8 bits per color
   * component compatible with on-screen windows
   */
  const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE};
  EGLint w, h, format;
  EGLint numConfigs;
  EGLConfig config = NULL;
  EGLSurface surface;
  EGLContext context;

  EGLDisplay display = eglGetDisplay (EGL_DEFAULT_DISPLAY);

  eglInitialize (display, NULL, NULL);

  /* Here, the application chooses the configuration it desires.
   * find the best match if possible, otherwise use the very first one
   */
  eglChooseConfig (display, attribs, NULL, 0, &numConfigs);
  EGLConfig *supportedConfigs = (EGLConfig *)new_mem (sizeof (EGLConfig) * numConfigs);
  eglChooseConfig (display, attribs, supportedConfigs, numConfigs, &numConfigs);
  if (!numConfigs) {
    LOGW ("Unable to initialize EGLConfig");
    return -1;
  }

  config = supportedConfigs[0];
  for (EGLint i = 0; i < numConfigs; ++i) {
    EGLint r, g, b, d;
    if (eglGetConfigAttrib (display, supportedConfigs[i], EGL_RED_SIZE, &r) &&
        eglGetConfigAttrib (display, supportedConfigs[i], EGL_GREEN_SIZE, &g) &&
        eglGetConfigAttrib (display, supportedConfigs[i], EGL_BLUE_SIZE, &b) &&
        eglGetConfigAttrib (display, supportedConfigs[i], EGL_DEPTH_SIZE, &d) && r == 8 &&
        g == 8 && b == 8 && d == 0) {
      config = supportedConfigs[i];
      break;
    }
  }
  free_mem (supportedConfigs);

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib (display, config, EGL_NATIVE_VISUAL_ID, &format);
  surface =
      eglCreateWindowSurface (display, config, app->window, NULL);

  /* A version of OpenGL has not been specified here.  This will default to
   * OpenGL 1.0.  You will need to change this if you want to use the newer
   * features of OpenGL like shaders. */
  context = eglCreateContext (display, config, NULL, NULL);

  if (eglMakeCurrent (display, surface, surface, context) == EGL_FALSE) {
    LOGW ("Unable to eglMakeCurrent");
    return -1;
  }

  eglQuerySurface (display, surface, EGL_WIDTH, &w);
  eglQuerySurface (display, surface, EGL_HEIGHT, &h);

  engine->display = display;
  engine->context = context;
  engine->surface = surface;
  engine->width = w;
  engine->height = h;
  engine->state.angle = 0;

  // Initialize GL state.
  glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  glEnable (GL_CULL_FACE);
  glDisable (GL_DEPTH_TEST);

  return 0;
}
static void engine_term_display (struct Engine *engine) {
  if (engine->display != EGL_NO_DISPLAY) {
    eglMakeCurrent (engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (engine->context != EGL_NO_CONTEXT) {
      eglDestroyContext (engine->display, engine->context);
    }
    if (engine->surface != EGL_NO_SURFACE) {
      eglDestroySurface (engine->display, engine->surface);
    }
    eglTerminate (engine->display);
  }
  Pause (engine);
  engine->display = EGL_NO_DISPLAY;
  engine->context = EGL_NO_CONTEXT;
  engine->surface = EGL_NO_SURFACE;
}
static int32_t engine_handle_input (AInputEvent *event) {
  struct Engine *engine = (struct Engine *)app->userData;
  if (AInputEvent_getType (event) == AINPUT_EVENT_TYPE_MOTION) {
    engine->state.x = AMotionEvent_getX (event, 0);
    engine->state.y = AMotionEvent_getY (event, 0);
    return 1;
  }
  return 0;
}
static void engine_handle_cmd (int32_t cmd) {
  struct Engine *engine = (struct Engine *)app->userData;
  switch (cmd) {
  case APP_CMD_SAVE_STATE:
    // The system has asked us to save our current state.  Do so.
    app->savedState = new_mem (sizeof (struct SavedState));
    *((struct SavedState *)app->savedState) = engine->state;
    app->savedStateSize = sizeof (struct SavedState);
    break;
  case APP_CMD_WINDOW_CHANGED:
    // The window is being shown, get it ready.
    if (app->window) {
      engine_init_display (engine);
    } else {
      engine_term_display (engine);
    }
    break;
    break;
  case APP_CMD_GAINED_FOCUS:
    // When our app gains focus, we start monitoring the accelerometer.
    if (engine->accelerometerSensor != NULL) {
      ASensorEventQueue_enableSensor (engine->sensorEventQueue,
                                      engine->accelerometerSensor);
      // We'd like to get 60 events per second (in us).
      ASensorEventQueue_setEventRate (engine->sensorEventQueue,
                                      engine->accelerometerSensor,
                                      (1000L / 60) * 1000);
    }
    Resume (engine);
    break;
  case APP_CMD_LOST_FOCUS:
    // When our app loses focus, we stop monitoring the accelerometer.
    // This is to avoid consuming battery while not being used.
    if (engine->accelerometerSensor != NULL) {
      ASensorEventQueue_disableSensor (engine->sensorEventQueue,
                                       engine->accelerometerSensor);
    }
    Pause (engine);
    break;
  default:
    break;
  }
}

int OnSensorEvent (int UNUSED (fd), int UNUSED (events), void *data) {
  struct Engine *engine = (struct Engine *)data;

  ASensorEvent event;
  while (ASensorEventQueue_getEvents (engine->sensorEventQueue, &event, 1) > 0) {
    // ("accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
  }

  // From the docs:
  //
  // Implementations should return 1 to continue receiving callbacks, or 0 to
  // have this file descriptor and callback unregistered from the looper.
  return 1;
}

void android_main () {
  struct Engine engine = {0};
  app->userData = &engine;
  app->onAppCmd = engine_handle_cmd;
  app->onInputEvent = engine_handle_input;

  // Prepare to monitor accelerometer
  CreateSensorListener (&engine, OnSensorEvent);

  if (app->savedState != NULL) {
    // We are starting with a previous saved state; restore from it.
    engine.state = *(struct SavedState *)app->savedState;
  }

  while (!app->destroyRequested) {
    // Our input, sensor, and update/render logic is all driven by callbacks, so
    // we don't need to use the non-blocking poll.
    struct android_poll_source *source = NULL;
    int result = ALooper_pollOnce (-1, NULL, NULL, (void **)&source);
    if (result == ALOOPER_POLL_ERROR) {
      LOGE ("ALooper_pollOnce returned an error");
    }

    if (source != NULL) {
      source->process (source);
    }
  }

  engine_term_display (&engine);
}

static void free_saved_state () {
  pthread_mutex_lock (&app->mutex);
  if (app->savedState != NULL) {
    free_mem (app->savedState);
    app->savedState = NULL;
    app->savedStateSize = 0;
  }
  pthread_mutex_unlock (&app->mutex);
}

int8_t android_app_read_cmd () {
  int8_t cmd;
  if (read (app->msgread, &cmd, sizeof (cmd)) == sizeof (cmd)) {
    switch (cmd) {
    case APP_CMD_SAVE_STATE:
      free_saved_state ();
      break;
    }
    return cmd;
  } else {
    LOGE ("No data on command pipe!");
  }
  return -1;
}

void android_app_pre_exec_cmd (int8_t cmd) {
  switch (cmd) {
  case APP_CMD_INPUT_CHANGED:
    pthread_mutex_lock (&app->mutex);
    if (app->inputQueue != NULL) {
      AInputQueue_detachLooper (app->inputQueue);
    }
    app->inputQueue = app->pendingInputQueue;
    if (app->inputQueue != NULL) {
      AInputQueue_attachLooper (app->inputQueue, app->looper, LOOPER_ID_INPUT, NULL, &app->inputPollSource);
    }
    pthread_cond_broadcast (&app->cond);
    pthread_mutex_unlock (&app->mutex);
    break;

  case APP_CMD_WINDOW_CHANGED:
    pthread_mutex_lock (&app->mutex);
    app->window = app->pendingWindow;
    pthread_cond_broadcast (&app->cond);
    pthread_mutex_unlock (&app->mutex);
    break;
  case APP_CMD_RESUME:
  case APP_CMD_START:
  case APP_CMD_PAUSE:
  case APP_CMD_STOP:
    pthread_mutex_lock (&app->mutex);
    app->activityState = cmd;
    pthread_cond_broadcast (&app->cond);
    pthread_mutex_unlock (&app->mutex);
    break;

  case APP_CMD_CONFIG_CHANGED:
    AConfiguration_fromAssetManager (app->config, app->activity->assetManager);
    break;

  case APP_CMD_DESTROY:
    app->destroyRequested = 1;
    break;
  }
}

void android_app_post_exec_cmd (int8_t cmd) {
  switch (cmd) {
  case APP_CMD_SAVE_STATE:
    pthread_mutex_lock (&app->mutex);
    app->stateSaved = 1;
    pthread_cond_broadcast (&app->cond);
    pthread_mutex_unlock (&app->mutex);
    break;

  case APP_CMD_RESUME:
    free_saved_state ();
    break;
  }
}

static void android_app_destroy () {
  free_saved_state ();
  pthread_mutex_lock (&app->mutex);
  if (app->inputQueue != NULL) {
    AInputQueue_detachLooper (app->inputQueue);
  }
  AConfiguration_delete (app->config);
  app->destroyed = 1;
  pthread_cond_broadcast (&app->cond);
  pthread_mutex_unlock (&app->mutex);
  // Can't touch app object after this.
}

static void process_input (struct android_poll_source *UNUSED (source)) {
  AInputEvent *event = NULL;
  if (AInputQueue_getEvent (app->inputQueue, &event) >= 0) {
    if (AInputQueue_preDispatchEvent (app->inputQueue, event)) {
      return;
    }
    int32_t handled = 0;
    if (app->onInputEvent != NULL) handled = app->onInputEvent (event);
    AInputQueue_finishEvent (app->inputQueue, event, handled);
  } else {
    LOGE ("Failure reading next input event: %s\n", strerror (errno));
  }
}

static void process_cmd (struct android_poll_source *UNUSED (source)) {
  int8_t cmd = android_app_read_cmd ();
  android_app_pre_exec_cmd (cmd);
  if (app->onAppCmd != NULL) app->onAppCmd (cmd);
  android_app_post_exec_cmd (cmd);
}

static void *android_app_entry (void *UNUSED (param)) {
  app->config = AConfiguration_new ();
  AConfiguration_fromAssetManager (app->config, app->activity->assetManager);

  app->cmdPollSource.id = LOOPER_ID_MAIN;
  app->cmdPollSource.process = process_cmd;
  app->inputPollSource.id = LOOPER_ID_INPUT;
  app->inputPollSource.process = process_input;

  ALooper *looper = ALooper_prepare (ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
  ALooper_addFd (looper, app->msgread, LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL, &app->cmdPollSource);
  app->looper = looper;

  pthread_mutex_lock (&app->mutex);
  app->running = 1;
  pthread_cond_broadcast (&app->cond);
  pthread_mutex_unlock (&app->mutex);

  android_main ();

  android_app_destroy ();
  return NULL;
}

// --------------------------------------------------------------------
// Native activity interaction (called from main thread)
// --------------------------------------------------------------------

static void android_app_write_cmd (int8_t cmd) {
  if (write (app->msgwrite, &cmd, sizeof (cmd)) != sizeof (cmd)) {
    LOGE ("Failure writing android_app cmd: %s\n", strerror (errno));
  }
}

static void android_app_set_activity_state (int8_t cmd) {
  pthread_mutex_lock (&app->mutex);
  android_app_write_cmd (cmd);
  while (app->activityState != cmd) {
    pthread_cond_wait (&app->cond, &app->mutex);
  }
  pthread_mutex_unlock (&app->mutex);
}
static void onDestroy (ANativeActivity *UNUSED (activity)) {
  pthread_mutex_lock (&app->mutex);
  android_app_write_cmd (APP_CMD_DESTROY);
  while (!app->destroyed) {
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
  android_app_set_activity_state (APP_CMD_START);
}

static void onResume (ANativeActivity *UNUSED (activity)) {
  android_app_set_activity_state (APP_CMD_RESUME);
}

static void *onSaveInstanceState (ANativeActivity *UNUSED (activity), size_t *outLen) {
  void *savedState = NULL;

  pthread_mutex_lock (&app->mutex);
  app->stateSaved = 0;
  android_app_write_cmd (APP_CMD_SAVE_STATE);
  while (!app->stateSaved) {
    pthread_cond_wait (&app->cond, &app->mutex);
  }

  if (app->savedState != NULL) {
    savedState = app->savedState;
    *outLen = app->savedStateSize;
    app->savedState = NULL;
    app->savedStateSize = 0;
  }

  pthread_mutex_unlock (&app->mutex);

  return savedState;
}

static void onPause (ANativeActivity *UNUSED (activity)) {
  android_app_set_activity_state (APP_CMD_PAUSE);
}

static void onStop (ANativeActivity *UNUSED (activity)) {
  android_app_set_activity_state (APP_CMD_STOP);
}

static void onConfigurationChanged (ANativeActivity *UNUSED (activity)) {
  android_app_write_cmd (APP_CMD_CONFIG_CHANGED);
}

static void onLowMemory (ANativeActivity *UNUSED (activity)) {
  android_app_write_cmd (APP_CMD_LOW_MEMORY);
}

static void onWindowFocusChanged (ANativeActivity *UNUSED (activity), int focused) {
  android_app_write_cmd (focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}
static void onNativeWindowCreated (ANativeActivity *UNUSED (activity), ANativeWindow *window) {
  pthread_mutex_lock (&app->mutex);
  app->pendingWindow = window;
  android_app_write_cmd (APP_CMD_WINDOW_CHANGED);
  while (!app->window) {
    pthread_cond_wait (&app->cond, &app->mutex);
  }
  pthread_mutex_unlock (&app->mutex);
}
static void onNativeWindowDestroyed (ANativeActivity *UNUSED (activity), ANativeWindow *UNUSED (window)) {
  pthread_mutex_lock (&app->mutex);
  app->pendingWindow = NULL;
  android_app_write_cmd (APP_CMD_WINDOW_CHANGED);
  while (app->window) {
    pthread_cond_wait (&app->cond, &app->mutex);
  }
  pthread_mutex_unlock (&app->mutex);
}
static void onInputQueueCreated (ANativeActivity *UNUSED (activity), AInputQueue *queue) {
  pthread_mutex_lock (&app->mutex);
  app->pendingInputQueue = queue;
  android_app_write_cmd (APP_CMD_INPUT_CHANGED);
  while (app->inputQueue != app->pendingInputQueue) {
    pthread_cond_wait (&app->cond, &app->mutex);
  }
  pthread_mutex_unlock (&app->mutex);
}
static void onInputQueueDestroyed (ANativeActivity *UNUSED (activity), AInputQueue *UNUSED (queue)) {
  pthread_mutex_lock (&app->mutex);
  app->pendingInputQueue = NULL;
  android_app_write_cmd (APP_CMD_INPUT_CHANGED);
  while (app->inputQueue) {
    pthread_cond_wait (&app->cond, &app->mutex);
  }
  pthread_mutex_unlock (&app->mutex);
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
  activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
  activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
  activity->callbacks->onInputQueueCreated = onInputQueueCreated;
  activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;

  app = (struct android_app *)new_imem (sizeof (struct android_app));
  app->activity = activity;

  pthread_mutex_init (&app->mutex, NULL);
  pthread_cond_init (&app->cond, NULL);

  if (savedState != NULL) {
    app->savedState = new_mem (savedStateSize);
    app->savedStateSize = savedStateSize;
    memcpy (app->savedState, savedState, savedStateSize);
  }

  if (pipe (&app->msgread)) {
    LOGE ("could not create pipe: %s", strerror (errno));
    return;
  }

  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  pthread_create (&app->thread, &attr, android_app_entry, NULL);

  // Wait for thread to start.
  pthread_mutex_lock (&app->mutex);
  while (!app->running) {
    pthread_cond_wait (&app->cond, &app->mutex);
  }
  pthread_mutex_unlock (&app->mutex);
}

// native MainActivity.java
JNIEXPORT void Java_com_ariasaproject_technowar_MainActivity_insetNative (JNIEnv *UNUSED (env), jobject UNUSED (o), jint left, jint top, jint right, jint bottom) {
  android_graphicsManager_resizeInsets (left, top, right, bottom);
}
