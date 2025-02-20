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

/**
 * Data associated with an ALooper fd that will be returned as the "outData"
 * when that source has data ready.
 */
struct android_poll_source {
  // The identifier of this source.  May be LOOPER_ID_MAIN or
  // LOOPER_ID_INPUT.
  int32_t id;

  // The android_app this ident is associated with.
  struct android_app *app;

  // Function to call to perform the standard processing of data from
  // this source.
  void (*process) (struct android_app *app, struct android_poll_source *source);
};

/**
 * This is the interface for the standard glue code of a threaded
 * application.  In this model, the application's code is running
 * in its own thread separate from the main thread of the process.
 * It is not required that this thread be associated with the Java
 * VM, although it will need to be in order to make JNI calls any
 * Java objects.
 */
struct android_app {
  // The application can place a pointer to its own state object
  // here if it likes.
  void *userData;

  // Fill this in with the function to process main app commands (APP_CMD_*)
  void (*onAppCmd) (struct android_app *app, int32_t cmd);

  // Fill this in with the function to process input events.  At this point
  // the event has already been pre-dispatched, and it will be finished upon
  // return.  Return 1 if you have handled the event, 0 for any default
  // dispatching.
  int32_t (*onInputEvent) (struct android_app *app, AInputEvent *event);

  // The ANativeActivity object instance that this app is running in.
  ANativeActivity *activity;

  // The current configuration the app is running in.
  AConfiguration *config;

  // This is the last instance's saved state, as provided at creation time.
  // It is NULL if there was no state.  You can use this as you need; the
  // memory will remain around until you call android_app_exec_cmd() for
  // APP_CMD_RESUME, at which point it will be freed and savedState set to NULL.
  // These variables should only be changed when processing a APP_CMD_SAVE_STATE,
  // at which point they will be initialized to NULL and you can malloc your
  // state and place the information here.  In that case the memory will be
  // freed for you later.
  void *savedState;
  size_t savedStateSize;

  // The ALooper associated with the app's thread.
  ALooper *looper;

  // When non-NULL, this is the input queue from which the app will
  // receive user input events.
  AInputQueue *inputQueue;

  // When non-NULL, this is the window surface that the app can draw in.
  ANativeWindow *window;

  // Current content rectangle of the window; this is the area where the
  // window's content should be placed to be seen by the user.
  ARect contentRect;

  // Current state of the app's activity.  May be either APP_CMD_START,
  // APP_CMD_RESUME, APP_CMD_PAUSE, or APP_CMD_STOP; see below.
  int activityState;

  // This is non-zero when the application's NativeActivity is being
  // destroyed and waiting for the app thread to complete.
  int destroyRequested;

  // -------------------------------------------------
  // Below are "private" implementation of the glue code.

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
  /**
   * Looper data ID of commands coming from the app's main thread, which
   * is returned as an identifier from ALooper_pollOnce().  The data for this
   * identifier is a pointer to an android_poll_source structure.
   * These can be retrieved and processed with android_app_read_cmd()
   * and android_app_exec_cmd().
   */
  LOOPER_ID_MAIN = 1,

  /**
   * Looper data ID of events coming from the AInputQueue of the
   * application's window, which is returned as an identifier from
   * ALooper_pollOnce().  The data for this identifier is a pointer to an
   * android_poll_source structure.  These can be read via the inputQueue
   * object of android_app.
   */
  LOOPER_ID_INPUT = 2,

  /**
   * Start of user-defined ALooper identifiers.
   */
  LOOPER_ID_USER = 3,
};

enum {
  /**
   * Command from main thread: the AInputQueue has changed.  Upon processing
   * this command, android_app->inputQueue will be updated to the new queue
   * (or NULL).
   */
  APP_CMD_INPUT_CHANGED,

  /**
   * Command from main thread: a new ANativeWindow is ready for use.  Upon
   * receiving this command, android_app->window will contain the new window
   * surface.
   */
  APP_CMD_INIT_WINDOW,

  /**
   * Command from main thread: the existing ANativeWindow needs to be
   * terminated.  Upon receiving this command, android_app->window still
   * contains the existing window; after calling android_app_exec_cmd
   * it will be set to NULL.
   */
  APP_CMD_TERM_WINDOW,

  /**
   * Command from main thread: the current ANativeWindow has been resized.
   * Please redraw with its new size.
   */
  APP_CMD_WINDOW_RESIZED,

  /**
   * Command from main thread: the system needs that the current ANativeWindow
   * be redrawn.  You should redraw the window before handing this to
   * android_app_exec_cmd() in order to avoid transient drawing glitches.
   */
  APP_CMD_WINDOW_REDRAW_NEEDED,

  /**
   * Command from main thread: the content area of the window has changed,
   * such as from the soft input window being shown or hidden.  You can
   * find the new content rect in android_app::contentRect.
   */
  APP_CMD_CONTENT_RECT_CHANGED,

  /**
   * Command from main thread: the app's activity window has gained
   * input focus.
   */
  APP_CMD_GAINED_FOCUS,

  /**
   * Command from main thread: the app's activity window has lost
   * input focus.
   */
  APP_CMD_LOST_FOCUS,

  /**
   * Command from main thread: the current device configuration has changed.
   */
  APP_CMD_CONFIG_CHANGED,

  /**
   * Command from main thread: the system is running low on memory.
   * Try to reduce your memory use.
   */
  APP_CMD_LOW_MEMORY,

  /**
   * Command from main thread: the app's activity has been started.
   */
  APP_CMD_START,

  /**
   * Command from main thread: the app's activity has been resumed.
   */
  APP_CMD_RESUME,

  APP_CMD_SAVE_STATE,

  /**
   * Command from main thread: the app's activity has been paused.
   */
  APP_CMD_PAUSE,

  /**
   * Command from main thread: the app's activity has been stopped.
   */
  APP_CMD_STOP,

  /**
   * Command from main thread: the app's activity is being destroyed,
   * and waiting for the app thread to clean up and exit before proceeding.
   */
  APP_CMD_DESTROY,
};

/**
 * Call when ALooper_pollAll() returns LOOPER_ID_MAIN, reading the next
 * app command message.
 */
int8_t android_app_read_cmd (struct android_app *android_app);

/**
 * Call with the command returned by android_app_read_cmd() to do the
 * initial pre-processing of the given command.  You can perform your own
 * actions for the command after calling this function.
 */
void android_app_pre_exec_cmd (struct android_app *android_app, int8_t cmd);

/**
 * Call with the command returned by android_app_read_cmd() to do the
 * final post-processing of the given command.  You must have done your own
 * actions for the command before calling this function.
 */
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

/**
 * Initialize an EGL context for the current display.
 */
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

/**
 * Tear down the EGL context currently associated with the display.
 */
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

/**
 * Process the next input event.
 */
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

/**
 * Process the next main command.
 */
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

int OnSensorEvent (int UNUSED (fd), int UNUSED (events), void *data) {
  struct Engine *engine = (struct Engine *)data;

  ASensorEvent event;
  while (ASensorEventQueue_getEvents (engine->sensorEventQueue, &event, 1) > 0) {
    LOGI ("accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
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

static void print_cur_config (struct android_app *android_app) {
  char lang[2], country[2];
  AConfiguration_getLanguage (android_app->config, lang);
  AConfiguration_getCountry (android_app->config, country);

  LOGI ("Config: mcc=%d mnc=%d lang=%c%c cnt=%c%c orien=%d touch=%d dens=%d "
        "keys=%d nav=%d keysHid=%d navHid=%d sdk=%d size=%d long=%d "
        "modetype=%d modenight=%d",
        AConfiguration_getMcc (android_app->config),
        AConfiguration_getMnc (android_app->config),
        lang[0],
        lang[1],
        country[0],
        country[1],
        AConfiguration_getOrientation (android_app->config),
        AConfiguration_getTouchscreen (android_app->config),
        AConfiguration_getDensity (android_app->config),
        AConfiguration_getKeyboard (android_app->config),
        AConfiguration_getNavigation (android_app->config),
        AConfiguration_getKeysHidden (android_app->config),
        AConfiguration_getNavHidden (android_app->config),
        AConfiguration_getSdkVersion (android_app->config),
        AConfiguration_getScreenSize (android_app->config),
        AConfiguration_getScreenLong (android_app->config),
        AConfiguration_getUiModeType (android_app->config),
        AConfiguration_getUiModeNight (android_app->config));
}

void android_app_pre_exec_cmd (struct android_app *android_app, int8_t cmd) {
  switch (cmd) {
  case APP_CMD_INPUT_CHANGED:
    LOGI ("APP_CMD_INPUT_CHANGED\n");
    pthread_mutex_lock (&android_app->mutex);
    if (android_app->inputQueue != NULL) {
      AInputQueue_detachLooper (android_app->inputQueue);
    }
    android_app->inputQueue = android_app->pendingInputQueue;
    if (android_app->inputQueue != NULL) {
      LOGI ("Attaching input queue to looper");
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
    LOGI ("APP_CMD_INIT_WINDOW\n");
    pthread_mutex_lock (&android_app->mutex);
    android_app->window = android_app->pendingWindow;
    pthread_cond_broadcast (&android_app->cond);
    pthread_mutex_unlock (&android_app->mutex);
    break;

  case APP_CMD_TERM_WINDOW:
    LOGI ("APP_CMD_TERM_WINDOW\n");
    pthread_cond_broadcast (&android_app->cond);
    break;

  case APP_CMD_RESUME:
  case APP_CMD_START:
  case APP_CMD_PAUSE:
  case APP_CMD_STOP:
    LOGI ("activityState=%d\n", cmd);
    pthread_mutex_lock (&android_app->mutex);
    android_app->activityState = cmd;
    pthread_cond_broadcast (&android_app->cond);
    pthread_mutex_unlock (&android_app->mutex);
    break;

  case APP_CMD_CONFIG_CHANGED:
    LOGI ("APP_CMD_CONFIG_CHANGED\n");
    AConfiguration_fromAssetManager (android_app->config,
                                     android_app->activity->assetManager);
    print_cur_config (android_app);
    break;

  case APP_CMD_DESTROY:
    LOGI ("APP_CMD_DESTROY\n");
    android_app->destroyRequested = 1;
    break;
  }
}

void android_app_post_exec_cmd (struct android_app *android_app, int8_t cmd) {
  switch (cmd) {
  case APP_CMD_TERM_WINDOW:
    LOGI ("APP_CMD_TERM_WINDOW\n");
    pthread_mutex_lock (&android_app->mutex);
    android_app->window = NULL;
    pthread_cond_broadcast (&android_app->cond);
    pthread_mutex_unlock (&android_app->mutex);
    break;

  case APP_CMD_SAVE_STATE:
    LOGI ("APP_CMD_SAVE_STATE\n");
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
  LOGI ("android_app_destroy!");
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

static void process_input (struct android_app *app, struct android_poll_source *UNUSED (source)) {
  AInputEvent *event = NULL;
  if (AInputQueue_getEvent (app->inputQueue, &event) >= 0) {
    LOGI ("New input event: type=%d\n", AInputEvent_getType (event));
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

static void process_cmd (struct android_app *app, struct android_poll_source *UNUSED (source)) {
  int8_t cmd = android_app_read_cmd (app);
  android_app_pre_exec_cmd (app, cmd);
  if (app->onAppCmd != NULL) app->onAppCmd (app, cmd);
  android_app_post_exec_cmd (app, cmd);
}

static void *android_app_entry (void *param) {
  struct android_app *android_app = (struct android_app *)param;

  android_app->config = AConfiguration_new ();
  AConfiguration_fromAssetManager (android_app->config, android_app->activity->assetManager);

  print_cur_config (android_app);

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

static struct android_app *android_app_create (ANativeActivity *activity,
                                               void *savedState,
                                               size_t savedStateSize) {
  struct android_app *android_app = (struct android_app *)new_imem (sizeof (struct android_app));
  android_app->activity = activity;

  pthread_mutex_init (&android_app->mutex, NULL);
  pthread_cond_init (&android_app->cond, NULL);

  if (savedState != NULL) {
    android_app->savedState = new_mem (savedStateSize);
    android_app->savedStateSize = savedStateSize;
    memcpy (android_app->savedState, savedState, savedStateSize);
  }

  int msgpipe[2];
  if (pipe (msgpipe)) {
    LOGE ("could not create pipe: %s", strerror (errno));
    return NULL;
  }
  android_app->msgread = msgpipe[0];
  android_app->msgwrite = msgpipe[1];

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

  return android_app;
}

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
  LOGI ("Destroy: %p\n", activity);
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
  LOGI ("Start: %p\n", activity);
  android_app_set_activity_state ((struct android_app *)activity->instance, APP_CMD_START);
}

static void onResume (ANativeActivity *activity) {
  LOGI ("Resume: %p\n", activity);
  android_app_set_activity_state ((struct android_app *)activity->instance, APP_CMD_RESUME);
}

static void *onSaveInstanceState (ANativeActivity *activity, size_t *outLen) {
  struct android_app *android_app = (struct android_app *)activity->instance;
  void *savedState = NULL;

  LOGI ("SaveInstanceState: %p\n", activity);
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
  LOGI ("Pause: %p\n", activity);
  android_app_set_activity_state ((struct android_app *)activity->instance, APP_CMD_PAUSE);
}

static void onStop (ANativeActivity *activity) {
  LOGI ("Stop: %p\n", activity);
  android_app_set_activity_state ((struct android_app *)activity->instance, APP_CMD_STOP);
}

static void onConfigurationChanged (ANativeActivity *activity) {
  struct android_app *android_app = (struct android_app *)activity->instance;
  LOGI ("ConfigurationChanged: %p\n", activity);
  android_app_write_cmd (android_app, APP_CMD_CONFIG_CHANGED);
}

static void onLowMemory (ANativeActivity *activity) {
  struct android_app *android_app = (struct android_app *)activity->instance;
  LOGI ("LowMemory: %p\n", activity);
  android_app_write_cmd (android_app, APP_CMD_LOW_MEMORY);
}

static void onWindowFocusChanged (ANativeActivity *activity, int focused) {
  LOGI ("WindowFocusChanged: %p -- %d\n", activity, focused);
  android_app_write_cmd ((struct android_app *)activity->instance,
                         focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}

static void onNativeWindowCreated (ANativeActivity *activity, ANativeWindow *window) {
  LOGI ("NativeWindowCreated: %p -- %p\n", activity, window);
  android_app_set_window ((struct android_app *)activity->instance, window);
}

static void onNativeWindowDestroyed (ANativeActivity *activity, ANativeWindow *window) {
  LOGI ("NativeWindowDestroyed: %p -- %p\n", activity, window);
  android_app_set_window ((struct android_app *)activity->instance, NULL);
}

static void onInputQueueCreated (ANativeActivity *activity, AInputQueue *queue) {
  LOGI ("InputQueueCreated: %p -- %p\n", activity, queue);
  android_app_set_input ((struct android_app *)activity->instance, queue);
}

static void onInputQueueDestroyed (ANativeActivity *activity, AInputQueue *queue) {
  LOGI ("InputQueueDestroyed: %p -- %p\n", activity, queue);
  android_app_set_input ((struct android_app *)activity->instance, NULL);
}

void ANativeActivity_onCreate (ANativeActivity *activity,
                               void *savedState,
                               size_t savedStateSize) {
  LOGI ("Creating: %p\n", activity);
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

  activity->instance = android_app_create (activity, savedState, savedStateSize);
}

// native MainActivity.java
JNIEXPORT void Java_com_ariasaproject_technowar_MainActivity_insetNative (JNIEnv *UNUSED (env), jobject UNUSED (o), jint left, jint top, jint right, jint bottom) {
  android_graphicsManager_resizeInsets (left, top, right, bottom);
}
