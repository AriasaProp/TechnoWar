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
  struct android_app *app;
  void (*process) (struct android_app *app, struct android_poll_source *source);
};
struct android_app {
  void *userData;
  void (*onAppCmd) (struct android_app *app, int32_t cmd);
  int32_t (*onInputEvent) (struct android_app *app, AInputEvent *event);
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
};

enum {
  LOOPER_ID_MAIN = 1,
  LOOPER_ID_INPUT = 2,
  LOOPER_ID_USER = 3,
};

enum {
  APP_CMD_INPUT_CHANGED,
  APP_CMD_INIT_WINDOW,
  APP_CMD_TERM_WINDOW,
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
int8_t android_app_read_cmd (struct android_app *android_app);
void android_app_pre_exec_cmd (struct android_app *android_app, int8_t cmd);
void android_app_post_exec_cmd (struct android_app *android_app, int8_t cmd);

struct SavedState {
  float angle;
  int32_t x;
  int32_t y;
};

struct Engine {
  struct android_app *app;

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
  engine->sensorEventQueue = ASensorManager_createEventQueue (engine->sensorManager, engine->app->looper, ALOOPER_POLL_CALLBACK, callback, engine);
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
      eglCreateWindowSurface (display, config, engine->app->window, NULL);

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
static int32_t engine_handle_input (struct android_app *app,
                                    AInputEvent *event) {
  struct Engine *engine = (struct Engine *)app->userData;
  if (AInputEvent_getType (event) == AINPUT_EVENT_TYPE_MOTION) {
    engine->state.x = AMotionEvent_getX (event, 0);
    engine->state.y = AMotionEvent_getY (event, 0);
    return 1;
  }
  return 0;
}
static void engine_handle_cmd (struct android_app *app, int32_t cmd) {
  struct Engine *engine = (struct Engine *)app->userData;
  switch (cmd) {
  case APP_CMD_SAVE_STATE:
    // The system has asked us to save our current state.  Do so.
    engine->app->savedState = new_mem (sizeof (struct SavedState));
    *((struct SavedState *)engine->app->savedState) = engine->state;
    engine->app->savedStateSize = sizeof (struct SavedState);
    break;
  case APP_CMD_INIT_WINDOW:
    // The window is being shown, get it ready.
    if (engine->app->window != NULL) {
      engine_init_display (engine);
    }
    break;
  case APP_CMD_TERM_WINDOW:
    // The window is being hidden or closed, clean it up.
    engine_term_display (engine);
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

int OnSensorEvent (int UNUSED(fd), int UNUSED(events), void *data) {
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

void android_main (struct android_app *state) {
  struct Engine engine = {0};
  state->userData = &engine;
  state->onAppCmd = engine_handle_cmd;
  state->onInputEvent = engine_handle_input;
  engine.app = state;

  // Prepare to monitor accelerometer
  CreateSensorListener (&engine, OnSensorEvent);

  if (state->savedState != NULL) {
    // We are starting with a previous saved state; restore from it.
    engine.state = *(struct SavedState *)state->savedState;
  }

  while (!state->destroyRequested) {
    // Our input, sensor, and update/render logic is all driven by callbacks, so
    // we don't need to use the non-blocking poll.
    struct android_poll_source *source = NULL;
    int result = ALooper_pollOnce (-1, NULL, NULL, (void **)&source);
    if (result == ALOOPER_POLL_ERROR) {
      LOGE ("ALooper_pollOnce returned an error");
    }

    if (source != NULL) {
      source->process (state, source);
    }
  }

  engine_term_display (&engine);
}

static void free_saved_state (struct android_app *android_app) {
  pthread_mutex_lock (&android_app->mutex);
  if (android_app->savedState != NULL) {
    free_mem (android_app->savedState);
    android_app->savedState = NULL;
    android_app->savedStateSize = 0;
  }
  pthread_mutex_unlock (&android_app->mutex);
}

int8_t android_app_read_cmd (struct android_app *android_app) {
  int8_t cmd;
  if (read (android_app->msgread, &cmd, sizeof (cmd)) == sizeof (cmd)) {
    switch (cmd) {
    case APP_CMD_SAVE_STATE:
      free_saved_state (android_app);
      break;
    }
    return cmd;
  } else {
    LOGE ("No data on command pipe!");
  }
  return -1;
}


void android_app_pre_exec_cmd (struct android_app *android_app, int8_t cmd) {
  switch (cmd) {
  case APP_CMD_INPUT_CHANGED:
    pthread_mutex_lock (&android_app->mutex);
    if (android_app->inputQueue != NULL) {
      AInputQueue_detachLooper (android_app->inputQueue);
    }
    android_app->inputQueue = android_app->pendingInputQueue;
    if (android_app->inputQueue != NULL) {
      AInputQueue_attachLooper (android_app->inputQueue,
                                android_app->looper,
                                LOOPER_ID_INPUT,
                                NULL,
                                &android_app->inputPollSource);
    }
    pthread_cond_broadcast (&android_app->cond);
    pthread_mutex_unlock (&android_app->mutex);
    break;

  case APP_CMD_INIT_WINDOW:
    pthread_mutex_lock (&android_app->mutex);
    android_app->window = android_app->pendingWindow;
    pthread_cond_broadcast (&android_app->cond);
    pthread_mutex_unlock (&android_app->mutex);
    break;

  case APP_CMD_TERM_WINDOW:
    pthread_cond_broadcast (&android_app->cond);
    break;

  case APP_CMD_RESUME:
  case APP_CMD_START:
  case APP_CMD_PAUSE:
  case APP_CMD_STOP:
    pthread_mutex_lock (&android_app->mutex);
    android_app->activityState = cmd;
    pthread_cond_broadcast (&android_app->cond);
    pthread_mutex_unlock (&android_app->mutex);
    break;

  case APP_CMD_CONFIG_CHANGED:
    AConfiguration_fromAssetManager (android_app->config,
                                     android_app->activity->assetManager);
    break;

  case APP_CMD_DESTROY:
    android_app->destroyRequested = 1;
    break;
  }
}

void android_app_post_exec_cmd (struct android_app *android_app, int8_t cmd) {
  switch (cmd) {
  case APP_CMD_TERM_WINDOW:
    pthread_mutex_lock (&android_app->mutex);
    android_app->window = NULL;
    pthread_cond_broadcast (&android_app->cond);
    pthread_mutex_unlock (&android_app->mutex);
    break;

  case APP_CMD_SAVE_STATE:
    pthread_mutex_lock (&android_app->mutex);
    android_app->stateSaved = 1;
    pthread_cond_broadcast (&android_app->cond);
    pthread_mutex_unlock (&android_app->mutex);
    break;

  case APP_CMD_RESUME:
    free_saved_state (android_app);
    break;
  }
}

static void android_app_destroy (struct android_app *android_app) {
  free_saved_state (android_app);
  pthread_mutex_lock (&android_app->mutex);
  if (android_app->inputQueue != NULL) {
    AInputQueue_detachLooper (android_app->inputQueue);
  }
  AConfiguration_delete (android_app->config);
  android_app->destroyed = 1;
  pthread_cond_broadcast (&android_app->cond);
  pthread_mutex_unlock (&android_app->mutex);
  // Can't touch android_app object after this.
}

static void process_input (struct android_app *app, struct android_poll_source *UNUSED(source)) {
  AInputEvent *event = NULL;
  if (AInputQueue_getEvent (app->inputQueue, &event) >= 0) {
    if (AInputQueue_preDispatchEvent (app->inputQueue, event)) {
      return;
    }
    int32_t handled = 0;
    if (app->onInputEvent != NULL) handled = app->onInputEvent (app, event);
    AInputQueue_finishEvent (app->inputQueue, event, handled);
  } else {
    LOGE ("Failure reading next input event: %s\n", strerror (errno));
  }
}

static void process_cmd (struct android_app *app, struct android_poll_source *UNUSED(source)) {
  int8_t cmd = android_app_read_cmd (app);
  android_app_pre_exec_cmd (app, cmd);
  if (app->onAppCmd != NULL) app->onAppCmd (app, cmd);
  android_app_post_exec_cmd (app, cmd);
}

static void *android_app_entry (void *param) {
  struct android_app *android_app = (struct android_app *)param;

  android_app->config = AConfiguration_new ();
  AConfiguration_fromAssetManager (android_app->config, android_app->activity->assetManager);


  android_app->cmdPollSource.id = LOOPER_ID_MAIN;
  android_app->cmdPollSource.app = android_app;
  android_app->cmdPollSource.process = process_cmd;
  android_app->inputPollSource.id = LOOPER_ID_INPUT;
  android_app->inputPollSource.app = android_app;
  android_app->inputPollSource.process = process_input;

  ALooper *looper = ALooper_prepare (ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
  ALooper_addFd (looper, android_app->msgread, LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL, &android_app->cmdPollSource);
  android_app->looper = looper;

  pthread_mutex_lock (&android_app->mutex);
  android_app->running = 1;
  pthread_cond_broadcast (&android_app->cond);
  pthread_mutex_unlock (&android_app->mutex);

  android_main (android_app);

  android_app_destroy (android_app);
  return NULL;
}

// --------------------------------------------------------------------
// Native activity interaction (called from main thread)
// --------------------------------------------------------------------

static void android_app_write_cmd (struct android_app *android_app, int8_t cmd) {
  if (write (android_app->msgwrite, &cmd, sizeof (cmd)) != sizeof (cmd)) {
    LOGE ("Failure writing android_app cmd: %s\n", strerror (errno));
  }
}

static void android_app_set_input (struct android_app *android_app, AInputQueue *inputQueue) {
  pthread_mutex_lock (&android_app->mutex);
  android_app->pendingInputQueue = inputQueue;
  android_app_write_cmd (android_app, APP_CMD_INPUT_CHANGED);
  while (android_app->inputQueue != android_app->pendingInputQueue) {
    pthread_cond_wait (&android_app->cond, &android_app->mutex);
  }
  pthread_mutex_unlock (&android_app->mutex);
}

static void android_app_set_window (struct android_app *android_app, ANativeWindow *window) {
  pthread_mutex_lock (&android_app->mutex);
  if (android_app->pendingWindow != NULL) {
    android_app_write_cmd (android_app, APP_CMD_TERM_WINDOW);
  }
  android_app->pendingWindow = window;
  if (window != NULL) {
    android_app_write_cmd (android_app, APP_CMD_INIT_WINDOW);
  }
  while (android_app->window != android_app->pendingWindow) {
    pthread_cond_wait (&android_app->cond, &android_app->mutex);
  }
  pthread_mutex_unlock (&android_app->mutex);
}

static void android_app_set_activity_state (struct android_app *android_app, int8_t cmd) {
  pthread_mutex_lock (&android_app->mutex);
  android_app_write_cmd (android_app, cmd);
  while (android_app->activityState != cmd) {
    pthread_cond_wait (&android_app->cond, &android_app->mutex);
  }
  pthread_mutex_unlock (&android_app->mutex);
}
static void onDestroy (ANativeActivity *activity) {
  struct android_app *android_app = (struct android_app *)activity->instance;
  pthread_mutex_lock (&android_app->mutex);
  android_app_write_cmd (android_app, APP_CMD_DESTROY);
  while (!android_app->destroyed) {
    pthread_cond_wait (&android_app->cond, &android_app->mutex);
  }
  pthread_mutex_unlock (&android_app->mutex);

  close (android_app->msgread);
  close (android_app->msgwrite);
  pthread_cond_destroy (&android_app->cond);
  pthread_mutex_destroy (&android_app->mutex);
  free_mem (android_app);
}

static void onStart (ANativeActivity *activity) {
  android_app_set_activity_state ((struct android_app *)activity->instance, APP_CMD_START);
}

static void onResume (ANativeActivity *activity) {
  android_app_set_activity_state ((struct android_app *)activity->instance, APP_CMD_RESUME);
}

static void *onSaveInstanceState (ANativeActivity *activity, size_t *outLen) {
  struct android_app *android_app = (struct android_app *)activity->instance;
  void *savedState = NULL;

  pthread_mutex_lock (&android_app->mutex);
  android_app->stateSaved = 0;
  android_app_write_cmd (android_app, APP_CMD_SAVE_STATE);
  while (!android_app->stateSaved) {
    pthread_cond_wait (&android_app->cond, &android_app->mutex);
  }

  if (android_app->savedState != NULL) {
    savedState = android_app->savedState;
    *outLen = android_app->savedStateSize;
    android_app->savedState = NULL;
    android_app->savedStateSize = 0;
  }

  pthread_mutex_unlock (&android_app->mutex);

  return savedState;
}

static void onPause (ANativeActivity *activity) {
  android_app_set_activity_state ((struct android_app *)activity->instance, APP_CMD_PAUSE);
}

static void onStop (ANativeActivity *activity) {
  android_app_set_activity_state ((struct android_app *)activity->instance, APP_CMD_STOP);
}

static void onConfigurationChanged (ANativeActivity *activity) {
  struct android_app *android_app = (struct android_app *)activity->instance;
  android_app_write_cmd (android_app, APP_CMD_CONFIG_CHANGED);
}

static void onLowMemory (ANativeActivity *activity) {
  struct android_app *android_app = (struct android_app *)activity->instance;
  android_app_write_cmd (android_app, APP_CMD_LOW_MEMORY);
}

static void onWindowFocusChanged (ANativeActivity *activity, int focused) {
  android_app_write_cmd ((struct android_app *)activity->instance,
                         focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}

static void onNativeWindowCreated (ANativeActivity *activity, ANativeWindow *window) {
  android_app_set_window ((struct android_app *)activity->instance, window);
}

static void onNativeWindowDestroyed (ANativeActivity *activity, ANativeWindow *window) {
  android_app_set_window ((struct android_app *)activity->instance, NULL);
}

static void onInputQueueCreated (ANativeActivity *activity, AInputQueue *queue) {
  android_app_set_input ((struct android_app *)activity->instance, queue);
}

static void onInputQueueDestroyed (ANativeActivity *activity, AInputQueue *queue) {
  android_app_set_input ((struct android_app *)activity->instance, NULL);
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

  struct android_app *android_app = (struct android_app *)new_imem (sizeof (struct android_app));
  activity->instance = android_app;
  android_app->activity = activity;

  pthread_mutex_init (&android_app->mutex, NULL);
  pthread_cond_init (&android_app->cond, NULL);

  if (savedState != NULL) {
    android_app->savedState = new_mem (savedStateSize);
    android_app->savedStateSize = savedStateSize;
    memcpy (android_app->savedState, savedState, savedStateSize);
  }

  if (pipe (&android_app->msgread)) {
    LOGE ("could not create pipe: %s", strerror (errno));
    return;
  }

  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  pthread_create (&android_app->thread, &attr, android_app_entry, android_app);

  // Wait for thread to start.
  pthread_mutex_lock (&android_app->mutex);
  while (!android_app->running) {
    pthread_cond_wait (&android_app->cond, &android_app->mutex);
  }
  pthread_mutex_unlock (&android_app->mutex);
}


// native MainActivity.java
JNIEXPORT void Java_com_ariasaproject_technowar_MainActivity_insetNative (JNIEnv *UNUSED (env), jobject UNUSED (o), jint left, jint top, jint right, jint bottom) {
  android_graphicsManager_resizeInsets (left, top, right, bottom);
}
