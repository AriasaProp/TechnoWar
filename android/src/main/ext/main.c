#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/choreographer.h>
#include <android/log.h>
#include <android/sensor.h>
#include <android/set_abort_message.h>
#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <sys/resource.h>
#include <jni.h>

#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define LOG_TAG "TechnoWar Activity"
#define _LOG(priority, fmt, ...) ((void)__android_log_print((priority), (LOG_TAG), (fmt)__VA_OPT__(, ) __VA_ARGS__))

#ifndef NDEBUG
#  define LOGV(fmt, ...) _LOG(ANDROID_LOG_VERBOSE, (fmt)__VA_OPT__(, ) __VA_ARGS__)
#else
#  define LOGV(fmt, ...)  ((void)0)
#endif
#define LOGE(fmt, ...) _LOG(ANDROID_LOG_ERROR, (fmt)__VA_OPT__(, ) __VA_ARGS__)
#define LOGW(fmt, ...) _LOG(ANDROID_LOG_WARN, (fmt)__VA_OPT__(, ) __VA_ARGS__)
#define LOGI(fmt, ...) _LOG(ANDROID_LOG_INFO, (fmt)__VA_OPT__(, ) __VA_ARGS__)

#if defined(__GNUC__)
#define UNUSED(x)       x##_UNUSED __attribute__((unused))
#else
#define UNUSED(x)       x##_UNUSED
#endif

struct android_app;

struct android_poll_source {
    // The identifier of this source.  May be LOOPER_ID_MAIN or
    // LOOPER_ID_INPUT.
    int32_t id;

    // The android_app this ident is associated with.
    struct android_app* app;

    // Function to call to perform the standard processing of data from
    // this source.
    void (*process)(struct android_app*, struct android_poll_source*);
};

struct android_app {
    // The application can place a pointer to its own state object
    // here if it likes.
    void* userData;
    // The ANativeActivity object instance that this app is running in.
    ANativeActivity* activity;

    // The current configuration the app is running in.
    AConfiguration* config;

    // This is the last instance's saved state, as provided at creation time.
    // It is NULL if there was no state.  You can use this as you need; the
    // memory will remain around until you call android_app_exec_cmd() for
    // APP_CMD_RESUME, at which point it will be freed and savedState set to NULL.
    // These variables should only be changed when processing a APP_CMD_SAVE_STATE,
    // at which point they will be initialized to NULL and you can malloc your
    // state and place the information here.  In that case the memory will be
    // freed for you later.
    void* savedState;
    size_t savedStateSize;

    // The ALooper associated with the app's thread.
    ALooper* looper;

    // When non-NULL, this is the input queue from which the app will
    // receive user input events.
    AInputQueue* inputQueue;

    // When non-NULL, this is the window surface that the app can draw in.
    ANativeWindow* window;

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
    
    int msg_pipe[2];

    pthread_t thread;

    struct android_poll_source cmdPollSource;
    struct android_poll_source inputPollSource;

    int running;
    int stateSaved;
    int destroyed;
    int redrawNeeded;
    AInputQueue* pendingInputQueue;
    ANativeWindow* pendingWindow;
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

struct SavedState {
  float angle;
  int32_t x;
  int32_t y;
};

struct Engine {
  struct android_app* app;

  ASensorManager* sensorManager;
  const ASensor* accelerometerSensor;
  ASensorEventQueue* sensorEventQueue;

  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  int32_t width;
  int32_t height;
  struct SavedState state;

  int running_;
};

static void Tick (long UNUSED(a), void *UNUSED(b));



static void Tick(long UNUSED(a), void* data) {
  struct Engine *e = (struct Engine*)data;
  if (!e->running_)
    return;
  AChoreographer_postFrameCallback(AChoreographer_getInstance(), Tick, e);
  // update
  {
  	e->state.angle += .01f;
    if (e->state.angle > 1) {
      e->state.angle = 0;
    }
  }
  // draw
  {
    if (!e->display)
      return;

    // Just fill the screen with a color.
    glClearColor(((float)e->state.x) / e->width, e->state.angle, ((float)e->state.y) / e->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(e->display, e->surface);
  }
}



static int engine_init_display(struct Engine* engine) {
  const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                            EGL_BLUE_SIZE,    8,
                            EGL_GREEN_SIZE,   8,
                            EGL_RED_SIZE,     8,
                            EGL_NONE};
  EGLint w, h, format;
  EGLint numConfigs;
  EGLConfig config = {0};
  EGLSurface surface;
  EGLContext context;

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  eglInitialize(display, 0, 0);

  eglChooseConfig(display, attribs, 0, 0, &numConfigs);
  if (!numConfigs) {
    LOGW("Unable to initialize EGLConfig");
    return -1;
  }
  EGLConfig *supportedConfigs = (EGLConfig *) new_mem(sizeof(EGLConfig)*numConfigs);
  eglChooseConfig(display, attribs, supportedConfigs, numConfigs, &numConfigs);
  
  for (size_t i = 0, j = 0, k, l; i < numConfigs; i++) {
    EGLint a;
    k = 1;
    if (!eglGetConfigAttrib(display, supportedConfigs[i], EGL_RED_SIZE, &a))
    	continue;
    k += a;
    if (!eglGetConfigAttrib(display, supportedConfigs[i], EGL_BLUE_SIZE, &a))
    	continue;
    k += a;
    if (!eglGetConfigAttrib(display, supportedConfigs[i], EGL_GREEN_SIZE, &a))
    	continue;
    k += a;
    if (j < k) {
    	j = k;
    	config = supportedConfigs[i];
    }
  }
  free_mem(supportedConfigs);


  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
  surface = eglCreateWindowSurface(display, config, engine->app->window, 0);

  /* A version of OpenGL has not been specified here.  This will default to
   * OpenGL 1.0.  You will need to change this if you want to use the newer
   * features of OpenGL like shaders. */
  context = eglCreateContext(display, config, 0, 0);

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
    LOGW("Unable to eglMakeCurrent");
    return -1;
  }

  eglQuerySurface(display, surface, EGL_WIDTH, &w);
  eglQuerySurface(display, surface, EGL_HEIGHT, &h);

  engine->display = display;
  engine->context = context;
  engine->surface = surface;
  engine->width = w;
  engine->height = h;
  engine->state.angle = 0;

  // Initialize GL state.
  glEnable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  return 0;
}

static void engine_term_display(struct Engine* engine) {
  if (engine->display != EGL_NO_DISPLAY) {
    eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (engine->context != EGL_NO_CONTEXT) {
      eglDestroyContext(engine->display, engine->context);
    }
    if (engine->surface != EGL_NO_SURFACE) {
      eglDestroySurface(engine->display, engine->surface);
    }
    eglTerminate(engine->display);
  }
  engine->running_ = 0; 
  engine->display = EGL_NO_DISPLAY;
  engine->context = EGL_NO_CONTEXT;
  engine->surface = EGL_NO_SURFACE;
}

int OnSensorEvent(int UNUSED(fd), int UNUSED(events), void* data) {
  struct Engine* engine = (struct Engine*)data;

  ASensorEvent event;
  while (ASensorEventQueue_getEvents(engine->sensorEventQueue, &event, 1) > 0) {
    LOGI("accelerometer: x=%f y=%f z=%f", event.acceleration.x,
         event.acceleration.y, event.acceleration.z);
  }

  // From the docs:
  //
  // Implementations should return 1 to continue receiving callbacks, or 0 to
  // have this file descriptor and callback unregistered from the looper.
  return 1;
}

JNIEXPORT void JNICALL Java_com_ariasaproject_technowar_MainActivity_insetNative (JNIEnv *UNUSED(a), jobject UNUSED(b), jint UNUSED(c), jint UNUSED(d), jint UNUSED(e), jint UNUSED(f)) {}

static void free_saved_state(struct android_app* android_app) {
    pthread_mutex_lock(&android_app->mutex);
    if (android_app->savedState != NULL) {
        free_mem(android_app->savedState);
        android_app->savedState = NULL;
        android_app->savedStateSize = 0;
    }
    pthread_mutex_unlock(&android_app->mutex);
}

int8_t android_app_read_cmd(struct android_app* android_app) {
  int8_t cmd;
  if (read(android_app->msg_pipe[0], &cmd, sizeof(cmd)) == sizeof(cmd)) {
    switch (cmd) {
    case APP_CMD_SAVE_STATE:
      free_saved_state(android_app);
      break;
    }
    return cmd;
  } else {
    LOGE("No data on command pipe!");
  }
  return -1;
}

static void print_cur_config(struct android_app* app) {
#ifdef NDEBUG
	((void)app);
#else
	AConfiguration_getLanguage(app->config, stemp.chars);
	AConfiguration_getCountry(app->config, stemp.chars + 2);
	
	LOGV("Config: mcc=%d mnc=%d lang=%c%c cnt=%c%c orien=%d touch=%d dens=%d "
        "keys=%d nav=%d keysHid=%d navHid=%d sdk=%d size=%d long=%d "
        "modetype=%d modenight=%d",
        AConfiguration_getMcc(app->config),
        AConfiguration_getMnc(app->config),
        stemp.chars[0], stemp.chars[1], stemp.chars[2], stemp.chars[3],
        AConfiguration_getOrientation(app->config),
        AConfiguration_getTouchscreen(app->config),
        AConfiguration_getDensity(app->config),
        AConfiguration_getKeyboard(app->config),
        AConfiguration_getNavigation(app->config),
        AConfiguration_getKeysHidden(app->config),
        AConfiguration_getNavHidden(app->config),
        AConfiguration_getSdkVersion(app->config),
        AConfiguration_getScreenSize(app->config),
        AConfiguration_getScreenLong(app->config),
        AConfiguration_getUiModeType(app->config),
        AConfiguration_getUiModeNight(app->config));
#endif
}

static void process_input(struct android_app* app, struct android_poll_source *UNUSED(a)) {
  AInputEvent* event = NULL;
  if (AInputQueue_getEvent(app->inputQueue, &event) >= 0) {
    LOGV("New input event: type=%d\n", AInputEvent_getType(event));
    if (AInputQueue_preDispatchEvent(app->inputQueue, event)) {
        return;
    }
    int32_t handled = 0;
    {
  	  struct Engine* engine = (struct Engine*)app->userData;
		  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		    engine->state.x = AMotionEvent_getX(event, 0);
		    engine->state.y = AMotionEvent_getY(event, 0);
		    handled = 1;
		  }
		  handled = 0;
    }
    AInputQueue_finishEvent(app->inputQueue, event, handled);
  } else {
    LOGE("Failure reading next input event: %s\n", strerror(errno));
  }
}

static void process_cmd(struct android_app* app, struct android_poll_source *UNUSED(a)) {
    int8_t cmd = android_app_read_cmd(app);
    // main command
    struct Engine* engine = (struct Engine*)app->userData;
	  switch (cmd) {
      case APP_CMD_INPUT_CHANGED:
        LOGV("APP_CMD_INPUT_CHANGED\n");
        pthread_mutex_lock(&app->mutex);
        if (app->inputQueue != NULL) {
            AInputQueue_detachLooper(app->inputQueue);
        }
        app->inputQueue = app->pendingInputQueue;
        if (app->inputQueue != NULL) {
            LOGV("Attaching input queue to looper");
            AInputQueue_attachLooper(app->inputQueue, app->looper, LOOPER_ID_INPUT, NULL, &app->inputPollSource);
        }
        pthread_cond_broadcast(&app->cond);
        pthread_mutex_unlock(&app->mutex);
        break;
      case APP_CMD_CONFIG_CHANGED:
        LOGV("APP_CMD_CONFIG_CHANGED\n");
        AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
        print_cur_config(app);
        break;
	    case APP_CMD_SAVE_STATE:
	      // The system has asked us to save our current state.  Do so.
	      engine->app->savedState = new_mem(sizeof(struct SavedState));
	      *((struct SavedState*)engine->app->savedState) = engine->state;
	      engine->app->savedStateSize = sizeof(struct SavedState);
	      // tell is done
        LOGV("APP_CMD_SAVE_STATE\n");
        pthread_mutex_lock(&app->mutex);
        app->stateSaved = 1;
        pthread_cond_broadcast(&app->cond);
        pthread_mutex_unlock(&app->mutex);
	      break;
	    case APP_CMD_INIT_WINDOW:
        LOGV("APP_CMD_INIT_WINDOW\n");
        pthread_mutex_lock(&app->mutex);
        app->window = app->pendingWindow;
        pthread_cond_broadcast(&app->cond);
        pthread_mutex_unlock(&app->mutex);
	      // The window is being shown, get it ready.
	      if (engine->app->window) {
	        engine_init_display(engine);
	      }
	      break;

      case APP_CMD_TERM_WINDOW:
        LOGV("APP_CMD_TERM_WINDOW\n");
        pthread_cond_broadcast(&app->cond);
	      // The window is being hidden or closed, clean it up.
	      engine_term_display(engine);
	      // set window to null
        pthread_mutex_lock(&app->mutex);
        app->window = NULL;
        pthread_cond_broadcast(&app->cond);
        pthread_mutex_unlock(&app->mutex);
	      break;
	    case APP_CMD_GAINED_FOCUS:
	      // When our app gains focus, we start monitoring the accelerometer.
	      if (engine->accelerometerSensor) {
	        ASensorEventQueue_enableSensor(engine->sensorEventQueue,
	                                       engine->accelerometerSensor);
	        // We'd like to get 60 events per second (in us).
	        ASensorEventQueue_setEventRate(engine->sensorEventQueue,
	                                       engine->accelerometerSensor,
	                                       (1000L / 60) * 1000);
	      }
	      if (engine->running_) {
				  engine->running_ = 1;
				  AChoreographer_postFrameCallback(AChoreographer_getInstance(), Tick, engine);
	      }
	      break;
	    case APP_CMD_LOST_FOCUS:
	      // When our app loses focus, we stop monitoring the accelerometer.
	      // This is to avoid consuming battery while not being used.
	      if (engine->accelerometerSensor) {
	        ASensorEventQueue_disableSensor(engine->sensorEventQueue,engine->accelerometerSensor);
	      }
	      engine->running_ = 0; 
	      break;
      case APP_CMD_RESUME:
        free_saved_state(app);
      	break;
      case APP_CMD_START:
      case APP_CMD_PAUSE:
      case APP_CMD_STOP:
      	break;
      case APP_CMD_DESTROY:
        LOGV("APP_CMD_DESTROY\n");
        app->destroyRequested = 1;
        break;
	    default:
	      break;
	  }
	  
    LOGV("activityState=%d\n", cmd);
    pthread_mutex_lock(&app->mutex);
    app->activityState = cmd;
    pthread_cond_broadcast(&app->cond);
    pthread_mutex_unlock(&app->mutex);
}

static void* android_app_entry(void* param) {
    struct android_app* app = (struct android_app*)param;

    app->config = AConfiguration_new();
    AConfiguration_fromAssetManager(app->config, app->activity->assetManager);

    print_cur_config(app);

    app->cmdPollSource.id = LOOPER_ID_MAIN;
    app->cmdPollSource.app = app;
    app->cmdPollSource.process = process_cmd;
    app->inputPollSource.id = LOOPER_ID_INPUT;
    app->inputPollSource.app = app;
    app->inputPollSource.process = process_input;

    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(looper, app->msg_pipe[0], LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL, &app->cmdPollSource);
    app->looper = looper;

    pthread_mutex_lock(&app->mutex);
    app->running = 1;
    pthread_cond_broadcast(&app->cond);
    pthread_mutex_unlock(&app->mutex);

    // main loop
		{
		  struct Engine *engine = (struct Engine*)new_imem(sizeof(struct Engine));
		  app->userData = engine;
		  engine->app = app;
		
		  // Prepare to monitor accelerometer
  	  engine->sensorManager = ASensorManager_getInstance();
		  if (engine->sensorManager) {
			  engine->accelerometerSensor = ASensorManager_getDefaultSensor(engine->sensorManager, ASENSOR_TYPE_ACCELEROMETER);
			  engine->sensorEventQueue = ASensorManager_createEventQueue(engine->sensorManager, engine->app->looper, ALOOPER_POLL_CALLBACK, OnSensorEvent, engine);
		  }
		
		  if (app->savedState) {
		    // We are starting with a previous saved state; restore from it.
		    engine->state = *(struct SavedState*)app->savedState;
		  }
		
		  while (!app->destroyRequested) {
		    // Our input, sensor, and update/render logic is all driven by callbacks, so
		    // we don't need to use the non-blocking poll.
		    struct android_poll_source* source = 0;
		    ALooper_pollOnce(-1, 0, 0, (void**)&source);
		
		    if (source) {
		      source->process(engine->app, source);
		    }
		  }
		
		  engine_term_display(engine);
		  free_mem(engine);
		}
		// destroy
    // Can't touch android_app object after this.
	  {
	    LOGV("android_app_destroy!");
	    free_saved_state(app);
	    pthread_mutex_lock(&app->mutex);
	    if (app->inputQueue != NULL) {
	        AInputQueue_detachLooper(app->inputQueue);
	    }
	    AConfiguration_delete(app->config);
	    app->destroyed = 1;
	    pthread_cond_broadcast(&app->cond);
	    pthread_mutex_unlock(&app->mutex);
	  }
    return NULL;
}

// --------------------------------------------------------------------
// Native activity interaction (called from main thread)
// --------------------------------------------------------------------

static void android_app_write_cmd(struct android_app* android_app, int8_t cmd) {
    if (write(android_app->msg_pipe[1], &cmd, sizeof(cmd)) != sizeof(cmd)) {
        LOGE("Failure writing android_app cmd: %s\n", strerror(errno));
    }
}

static void android_app_set_input(struct android_app* android_app, AInputQueue* inputQueue) {
    pthread_mutex_lock(&android_app->mutex);
    android_app->pendingInputQueue = inputQueue;
    android_app_write_cmd(android_app, APP_CMD_INPUT_CHANGED);
    while (android_app->inputQueue != android_app->pendingInputQueue) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }
    pthread_mutex_unlock(&android_app->mutex);
}

static void android_app_set_window(struct android_app* android_app, ANativeWindow* window) {
    pthread_mutex_lock(&android_app->mutex);
    if (android_app->pendingWindow != NULL) {
        android_app_write_cmd(android_app, APP_CMD_TERM_WINDOW);
    }
    android_app->pendingWindow = window;
    if (window != NULL) {
        android_app_write_cmd(android_app, APP_CMD_INIT_WINDOW);
    }
    while (android_app->window != android_app->pendingWindow) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }
    pthread_mutex_unlock(&android_app->mutex);
}

static void android_app_set_activity_state(struct android_app* android_app, int8_t cmd) {
  pthread_mutex_lock(&android_app->mutex);
  android_app_write_cmd(android_app, cmd);
  while (android_app->activityState != cmd) {
    pthread_cond_wait(&android_app->cond, &android_app->mutex);
  }
  pthread_mutex_unlock(&android_app->mutex);
}

static void android_app_free(struct android_app* android_app) {
    pthread_mutex_lock(&android_app->mutex);
    android_app_write_cmd(android_app, APP_CMD_DESTROY);
    while (!android_app->destroyed) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }
    pthread_mutex_unlock(&android_app->mutex);

    close(android_app->msg_pipe[0]);
    close(android_app->msg_pipe[1]);
    pthread_cond_destroy(&android_app->cond);
    pthread_mutex_destroy(&android_app->mutex);
    free_mem(android_app);
}

static void onDestroy(ANativeActivity* activity) {
    LOGV("Destroy: %p\n", activity);
    android_app_free((struct android_app*)activity->instance);
}

static void onStart(ANativeActivity* activity) {
    LOGV("Start: %p\n", activity);
    android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_START);
}

static void onResume(ANativeActivity* activity) {
    LOGV("Resume: %p\n", activity);
    android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_RESUME);
}

static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
    struct android_app* android_app = (struct android_app*)activity->instance;
    void* savedState = NULL;

    LOGV("SaveInstanceState: %p\n", activity);
    pthread_mutex_lock(&android_app->mutex);
    android_app->stateSaved = 0;
    android_app_write_cmd(android_app, APP_CMD_SAVE_STATE);
    while (!android_app->stateSaved) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }

    if (android_app->savedState != NULL) {
        savedState = android_app->savedState;
        *outLen = android_app->savedStateSize;
        android_app->savedState = NULL;
        android_app->savedStateSize = 0;
    }

    pthread_mutex_unlock(&android_app->mutex);

    return savedState;
}

static void onPause(ANativeActivity* activity) {
    LOGV("Pause: %p\n", activity);
    android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_PAUSE);
}

static void onStop(ANativeActivity* activity) {
    LOGV("Stop: %p\n", activity);
    android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_STOP);
}

static void onConfigurationChanged(ANativeActivity* activity) {
    struct android_app* android_app = (struct android_app*)activity->instance;
    LOGV("ConfigurationChanged: %p\n", activity);
    android_app_write_cmd(android_app, APP_CMD_CONFIG_CHANGED);
}

static void onLowMemory(ANativeActivity* activity) {
    struct android_app* android_app = (struct android_app*)activity->instance;
    LOGV("LowMemory: %p\n", activity);
    android_app_write_cmd(android_app, APP_CMD_LOW_MEMORY);
}

static void onWindowFocusChanged(ANativeActivity* activity, int focused) {
    LOGV("WindowFocusChanged: %p -- %d\n", activity, focused);
    android_app_write_cmd((struct android_app*)activity->instance,
            focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}

static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow *window) {
    LOGV("NativeWindowCreated: %p -- %p\n", activity, window);
    android_app_set_window((struct android_app*)activity->instance, window);
}

static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow *window) {
    LOGV("NativeWindowDestroyed: %p -- %p\n", activity, window);
    ((void)window);
    android_app_set_window((struct android_app*)activity->instance, NULL);
}

static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    LOGV("InputQueueCreated: %p -- %p\n", activity, queue);
    android_app_set_input((struct android_app*)activity->instance, queue);
}

static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
    LOGV("InputQueueDestroyed: %p -- %p\n", activity, queue);
    ((void)queue);
    android_app_set_input((struct android_app*)activity->instance, NULL);
}

void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
	// create thread
  LOGV("Creating: %p\n", activity);
	{
	  struct android_app* app = (struct android_app*)new_imem(sizeof(struct android_app));
	  app->activity = activity;
	
	  pthread_mutex_init(&app->mutex, NULL);
	  pthread_cond_init(&app->cond, NULL);
	
	  if (savedState) {
      app->savedState = new_mem(savedStateSize);
      app->savedStateSize = savedStateSize;
      memcpy(app->savedState, savedState, savedStateSize);
	  }
	
	  if (pipe(app->msg_pipe)) {
      LOGE("could not create pipe: %s", strerror(errno));
      return;
	  }
	
	  pthread_attr_t attr; 
	  pthread_attr_init(&attr);
	  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	  pthread_create(&app->thread, &attr, android_app_entry, app);
	
	  // Wait for thread to start.
	  pthread_mutex_lock(&app->mutex);
	  while (!app->running) {
      pthread_cond_wait(&app->cond, &app->mutex);
	  }
	  pthread_mutex_unlock(&app->mutex);
		activity->instance = app;
	}
  LOGV("Created: %p\n", activity);
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
}