#include <jni.h>

#include <sys/resource.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/configuration.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

#include "util.h"

#define LOG_TAG "TechnoWar Activity"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#ifndef NDEBUG
#  define LOGV(...)  ((void)__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#else
#  define LOGV(...)  ((void)0)
#endif

struct android_app {
    void* userData;
    ANativeActivity* activity;
    AConfiguration* config;
    void* savedState;
    size_t savedStateSize;
    ALooper* looper;
    AInputQueue* inputQueue;
    ANativeWindow* window;
    ARect contentRect;
    int activityState;
    int destroyRequested;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    int msgpipe[2];

    pthread_t thread;

    int stateSaved;
    int destroyed;
    int redrawNeeded;
    AInputQueue* pendingInputQueue;
    ANativeWindow* pendingWindow;
    ARect pendingContentRect;
};
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};
struct engine {
  struct android_app* app;

  ASensorManager* sensorManager;
  const ASensor* accelerometerSensor;
  ASensorEventQueue* sensorEventQueue;

  EGLDisplay display;
  EGLConfig config;
  EGLSurface surface;
  EGLContext context;
  
  EGLint width, height;
  struct saved_state state;
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


// engine gate
static inline int egl_invalid (struct engine *engine) {
	return
		(engine->display == EGL_NO_DISPLAY) && 
		(engine->surface == EGL_NO_SURFACE) && 
		(engine->context == EGL_NO_CONTEXT);
}
static int engine_init_egl(struct engine* engine) {
	if (engine->app->window == NULL) return -1;
	if (engine->display == EGL_NO_DISPLAY) {
	  // initialize EGL display
		EGLint temp[2];
	  engine->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	  if (engine->display == EGL_NO_DISPLAY) {
	  	LOGE("Failed get default egl display");
	  	return -1;
	  }
	  eglInitialize(engine->display, temp, temp+1);
	  if (temp[0] < 1 || temp[1] < 2) {
	  	LOGE("EGL version is %d.%d, that lower than 1.2", temp[0], temp[1]);
	  	return -1;
	  }
	  // get config for new display
	  const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE};
	  eglChooseConfig(engine->display, attribs, 0,0, temp);
	  if (!temp[0]) {
	  	LOGE("No supported EGLConfig for current display");
	  	return -1;
	  }
	  EGLConfig *supportedConfigs = (EGLConfig*)new_mem(sizeof(EGLConfig) * temp[0]);
	  eglChooseConfig(engine->display, attribs, supportedConfigs, temp[0], temp);
	  engine->config = supportedConfigs[0];
	  const EGLint observe_attribs[] = {EGL_RED_SIZE, EGL_GREEN_SIZE, EGL_BLUE_SIZE, EGL_DEPTH_SIZE };
	  EGLint best_value = -1, current_value, observe;
	  for (EGLint i = 0; i < temp[0]; ++i) {
	  	current_value = 0;
	  	for (EGLint j = 0, k = sizeof(observe_attribs)/sizeof(EGLint); j < k; ++j) {
	      if (eglGetConfigAttrib(engine->display, supportedConfigs[i], observe_attribs[j], &observe))
	      	current_value += observe;
	    }
	    if (best_value < current_value) {
	    	best_value = current_value;
	    	engine->config = supportedConfigs[i];
	    }
	  }
	  free_mem(supportedConfigs);
	  // eglGetConfigAttrib(engine->display, config, EGL_NATIVE_VISUAL_ID, temp);
	}
  // create surface
  if (engine->surface == EGL_NO_SURFACE)
  	engine->surface = eglCreateWindowSurface(engine->display, engine->config, engine->app->window, NULL);

  // create context
  if (engine->context == EGL_NO_CONTEXT)
  	engine->context = eglCreateContext(engine->display, engine->config, NULL, NULL);


  if (eglMakeCurrent(engine->display, engine->surface, engine->surface, engine->context) == EGL_FALSE) {
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
    LOGE("Unable to eglMakeCurrent");
    return -1;
  }
  eglQuerySurface(engine->display, engine->surface, EGL_WIDTH, &engine->width);
  eglQuerySurface(engine->display, engine->surface, EGL_HEIGHT, &engine->height);
  engine->state.angle = 0;

  // Initialize GL state.
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  glEnable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  return 0;
}
static void engine_draw_frame(struct engine* engine) {
    // Just fill the screen with a color.
    glClearColor(((float)engine->state.x)/engine->width, engine->state.angle,
                 ((float)engine->state.y)/engine->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(engine->display, engine->surface);
}
static inline void engine_term_egl(struct engine* engine, int all) {
	if (engine->display == EGL_NO_DISPLAY) return;
  eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  if (engine->context != EGL_NO_CONTEXT) {
    eglDestroyContext(engine->display, engine->context);
  	engine->context = EGL_NO_CONTEXT;
  }
  if (engine->surface != EGL_NO_SURFACE) {
    eglDestroySurface(engine->display, engine->surface);
  	engine->surface = EGL_NO_SURFACE;
  }
    // terminate EGL display
  if (all) {
    eglTerminate(engine->display);
  	engine->display = EGL_NO_DISPLAY;
  }
}
static void engine_handle_cmd(struct android_app* app, int8_t cmd) {
	struct engine* engine = (struct engine*)app->userData;
	switch (cmd) {
	case APP_CMD_SAVE_STATE:
	  // The system has asked us to save our current state.  Do so.
	  engine->app->savedState = new_mem(sizeof(struct saved_state));
	  *((struct saved_state*)engine->app->savedState) = engine->state;
	  engine->app->savedStateSize = sizeof(struct saved_state);
	  break;
	case APP_CMD_INIT_WINDOW:
	  // The window is being shown, get it ready.
	  engine_init_egl(engine);
	  break;
	case APP_CMD_TERM_WINDOW:
	  // The window is being hidden or closed, clean it up.
	  engine_term_egl(engine, 0);
	  break;
	case APP_CMD_GAINED_FOCUS:
	  // When our app gains focus, we start monitoring the accelerometer.
	  if (engine->accelerometerSensor != NULL) {
	      ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
	      // We'd like to get 60 events per second (in us).
	      ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->accelerometerSensor, (1000L/60)*1000);
	  }
	  break;
	case APP_CMD_LOST_FOCUS:
	  // When our app loses focus, we stop monitoring the accelerometer.
	  // This is to avoid consuming battery while not being used.
	  if (engine->accelerometerSensor != NULL) {
	      ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
	  }
	  break;
	}
}
#include <dlfcn.h>
ASensorManager* AcquireASensorManagerInstance(struct android_app* app) {

  if(!app)
    return 0;

  typedef ASensorManager *(*PF_GETINSTANCEFORPACKAGE)(const char *name);
  void* androidHandle = dlopen("libandroid.so", RTLD_NOW);
  PF_GETINSTANCEFORPACKAGE getInstanceForPackageFunc = (PF_GETINSTANCEFORPACKAGE)
      dlsym(androidHandle, "ASensorManager_getInstanceForPackage");
  if (getInstanceForPackageFunc) {
    JNIEnv* env = 0;
    (*app->activity->vm)->AttachCurrentThread(app->activity->vm, &env, NULL);

    jclass android_content_Context = (*env)->GetObjectClass(env, app->activity->clazz);
    jmethodID midGetPackageName = (*env)->GetMethodID(env, android_content_Context,
                                                   "getPackageName",
                                                   "()Ljava/lang/String;");
    jstring packageName= (jstring)(*env)->CallObjectMethod(env, app->activity->clazz,
                                                        midGetPackageName);

    const char *nativePackageName = (*env)->GetStringUTFChars(env, packageName, 0);
    ASensorManager* mgr = getInstanceForPackageFunc(nativePackageName);
    (*env)->ReleaseStringUTFChars(env, packageName, nativePackageName);
    (*app->activity->vm)->DetachCurrentThread(app->activity->vm);
    if (mgr) {
      dlclose(androidHandle);
      return mgr;
    }
  }

  typedef ASensorManager *(*PF_GETINSTANCE)();
  PF_GETINSTANCE getInstanceFunc = (PF_GETINSTANCE)
      dlsym(androidHandle, "ASensorManager_getInstance");
  // by all means at this point, ASensorManager_getInstance should be available
  dlclose(androidHandle);

  return getInstanceFunc();
}
// engine gate end

static void free_saved_state(struct android_app* android_app) {
    pthread_mutex_lock(&android_app->mutex);
    if (android_app->savedState != NULL) {
        free_mem(android_app->savedState);
        android_app->savedState = NULL;
        android_app->savedStateSize = 0;
    }
    pthread_mutex_unlock(&android_app->mutex);
}
static inline void print_cur_config(struct android_app* android_app) {
    AConfiguration_getLanguage(android_app->config, stemp.chars);
    AConfiguration_getCountry(android_app->config, stemp.chars + 2);

    LOGV("Config: mcc=%d mnc=%d lang=%c%c cnt=%c%c orien=%d touch=%d dens=%d "
            "keys=%d nav=%d keysHid=%d navHid=%d sdk=%d size=%d long=%d "
            "modetype=%d modenight=%d",
            AConfiguration_getMcc(android_app->config),
            AConfiguration_getMnc(android_app->config),
            stemp.chars[0], stemp.chars[1], stemp.chars[2], stemp.chars[3],
            AConfiguration_getOrientation(android_app->config),
            AConfiguration_getTouchscreen(android_app->config),
            AConfiguration_getDensity(android_app->config),
            AConfiguration_getKeyboard(android_app->config),
            AConfiguration_getNavigation(android_app->config),
            AConfiguration_getKeysHidden(android_app->config),
            AConfiguration_getNavHidden(android_app->config),
            AConfiguration_getSdkVersion(android_app->config),
            AConfiguration_getScreenSize(android_app->config),
            AConfiguration_getScreenLong(android_app->config),
            AConfiguration_getUiModeType(android_app->config),
            AConfiguration_getUiModeNight(android_app->config));
}
void android_app_pre_exec_cmd(struct android_app* android_app, int8_t cmd) {
  switch (cmd) {
  case APP_CMD_INPUT_CHANGED:
    LOGV("APP_CMD_INPUT_CHANGED\n");
    pthread_mutex_lock(&android_app->mutex);
    if (android_app->inputQueue != NULL) {
        AInputQueue_detachLooper(android_app->inputQueue);
    }
    android_app->inputQueue = android_app->pendingInputQueue;
    if (android_app->inputQueue != NULL) {
        LOGV("Attaching input queue to looper");
        AInputQueue_attachLooper(android_app->inputQueue,
                android_app->looper, LOOPER_ID_INPUT, NULL,
                &android_app->inputPollSource);
    }
    pthread_cond_broadcast(&android_app->cond);
    pthread_mutex_unlock(&android_app->mutex);
    break;

  case APP_CMD_INIT_WINDOW:
    LOGV("APP_CMD_INIT_WINDOW\n");
    pthread_mutex_lock(&android_app->mutex);
    android_app->window = android_app->pendingWindow;
    pthread_cond_broadcast(&android_app->cond);
    pthread_mutex_unlock(&android_app->mutex);
    break;

  case APP_CMD_TERM_WINDOW:
    LOGV("APP_CMD_TERM_WINDOW\n");
    pthread_cond_broadcast(&android_app->cond);
    break;

  case APP_CMD_SAVE_STATE:
    free_saved_state(android_app);
    break;

  case APP_CMD_RESUME:
  case APP_CMD_START:
  case APP_CMD_PAUSE:
  case APP_CMD_STOP:
    LOGV("activityState=%d\n", cmd);
    pthread_mutex_lock(&android_app->mutex);
    android_app->activityState = cmd;
    pthread_cond_broadcast(&android_app->cond);
    pthread_mutex_unlock(&android_app->mutex);
    break;

  case APP_CMD_CONFIG_CHANGED:
    LOGV("APP_CMD_CONFIG_CHANGED\n");
    AConfiguration_fromAssetManager(android_app->config, android_app->activity->assetManager);
    print_cur_config(android_app);
    break;

  case APP_CMD_DESTROY:
    LOGV("APP_CMD_DESTROY\n");
    android_app->destroyRequested = 1;
    break;
  }
}
void android_app_post_exec_cmd(struct android_app* android_app, int8_t cmd) {
    switch (cmd) {
        case APP_CMD_TERM_WINDOW:
            LOGV("APP_CMD_TERM_WINDOW\n");
            pthread_mutex_lock(&android_app->mutex);
            android_app->window = NULL;
            pthread_cond_broadcast(&android_app->cond);
            pthread_mutex_unlock(&android_app->mutex);
            break;

        case APP_CMD_SAVE_STATE:
            LOGV("APP_CMD_SAVE_STATE\n");
            pthread_mutex_lock(&android_app->mutex);
            android_app->stateSaved = 1;
            pthread_cond_broadcast(&android_app->cond);
            pthread_mutex_unlock(&android_app->mutex);
            break;

        case APP_CMD_RESUME:
            free_saved_state(android_app);
            break;
    }
}

// running on other thread
static void* android_app_entry(void* param) {
	struct android_app* android_app = (struct android_app*) param;
	android_app->config = AConfiguration_new();
  AConfiguration_fromAssetManager(android_app->config, android_app->activity->assetManager);
  print_cur_config(android_app);
  ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
  ALooper_addFd(looper, android_app->msgpipe[0], LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL, &android_app->cmdPollSource);
  android_app->looper = looper;
  
  struct engine engine;
  memset(&engine, 0, sizeof(engine));
  android_app->userData = &engine;
  engine.app = android_app;

  // Prepare to monitor accelerometer
  engine.sensorManager = AcquireASensorManagerInstance(android_app);
  engine.accelerometerSensor = ASensorManager_getDefaultSensor( engine.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
  engine.sensorEventQueue = ASensorManager_createEventQueue( engine.sensorManager, android_app->looper, LOOPER_ID_USER, NULL, NULL);

  if (android_app->savedState != NULL) {
    // We are starting with a previous saved state; restore from it.
    engine.state = *(struct saved_state*)android_app->savedState;
  }
  
	int8_t cmd;
	AInputEvent* event = NULL;
  // game loop while checking if game requested to exit
  while (!android_app->destroyRequested) {
    switch (ALooper_pollOnce(egl_invalid(&engine) * -1, NULL, NULL, NULL)) {
    	case LOOPER_ID_MAIN:
			  if (read(android_app->msgpipe[0], &cmd, sizeof(cmd)) != sizeof(cmd)) {
			    LOGE("No data on command pipe!");
    			break;
			  }
			  android_app_pre_exec_cmd(android_app, cmd);
			  engine_handle_cmd(android_app, cmd);
			  android_app_post_exec_cmd(android_app, cmd);
    		break;
    	case LOOPER_ID_INPUT:
			  if (AInputQueue_getEvent(android_app->inputQueue, &event) >= 0) {
			    LOGV("New input event: type=%d\n", AInputEvent_getType(event));
			    if (!AInputQueue_preDispatchEvent(android_app->inputQueue, event)) {
				    int32_t handled = 0;
				    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
				      engine.state.x = AMotionEvent_getX(event, 0);
				      engine.state.y = AMotionEvent_getY(event, 0);
				      handled = 1;
				    }
				    AInputQueue_finishEvent(android_app->inputQueue, event, handled);
			    }
			  } else {
			    LOGE("Failure reading next input event: %s\n", strerror(errno));
			  }
    		break;
    	case LOOPER_ID_USER:
    		break;
    }
    if (!egl_invalid(&engine)) {
      // Done with events; draw next animation frame.
      engine.state.angle += .01f;
      if (engine.state.angle > 1) {
          engine.state.angle = 0;
      }

      // Drawing is throttled to the screen update rate, so there
      // is no need to do timing here.
      engine_draw_frame(&engine);
    }
  }
  engine_term_egl(&engine, 1);

  LOGV("android_app_destroy!");
  free_saved_state(android_app);
  pthread_mutex_lock(&android_app->mutex);
  if (android_app->inputQueue != NULL) {
      AInputQueue_detachLooper(android_app->inputQueue);
  }
  AConfiguration_delete(android_app->config);
  android_app->destroyed = 1;
  pthread_cond_broadcast(&android_app->cond);
  pthread_mutex_unlock(&android_app->mutex);
  // Can't touch android_app object after this.
  return NULL;
}
static struct android_app* android_app_create(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
  struct android_app* android_app = (struct android_app*)new_imem(sizeof(struct android_app));
  android_app->activity = activity;

  pthread_mutex_init(&android_app->mutex, NULL);
  pthread_cond_init(&android_app->cond, NULL);

  if (savedState != NULL) {
    android_app->savedState = new_mem(savedStateSize);
    android_app->savedStateSize = savedStateSize;
    memcpy(android_app->savedState, savedState, savedStateSize);
  }
  if (pipe(android_app->msgpipe)) {
    LOGE("could not create pipe: %s", strerror(errno));
    return NULL;
  }
  
  // start thread
  pthread_attr_t attr; 
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&android_app->thread, &attr, android_app_entry, android_app);

  return android_app;
}

static void android_app_write_cmd(struct android_app* android_app, int8_t cmd) {
    if (write(android_app->msgpipe[1], &cmd, sizeof(cmd)) != sizeof(cmd)) {
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

static void onDestroy(ANativeActivity* activity) {
  LOGV("Destroy: %p\n", activity);
  struct android_app* android_app = (struct android_app*)activity->instance;
  pthread_mutex_lock(&android_app->mutex);
  android_app_write_cmd(android_app, APP_CMD_DESTROY);
  while (!android_app->destroyed) {
      pthread_cond_wait(&android_app->cond, &android_app->mutex);
  }
  pthread_mutex_unlock(&android_app->mutex);

  close(android_app->msgpipe[0]);
  close(android_app->msgpipe[1]);
  pthread_cond_destroy(&android_app->cond);
  pthread_mutex_destroy(&android_app->mutex);
  free_mem(android_app);
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
static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
  LOGV("NativeWindowCreated: %p -- %p\n", activity, window);
  android_app_set_window((struct android_app*)activity->instance, window);
}
static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
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
    LOGV("Creating: %p\n", activity);
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

    activity->instance = android_app_create(activity, savedState, savedStateSize);
}

// JNI
JNIEXPORT void JNICALL Java_com_ariasaproject_technowar_MainActivity_insetNative (JNIEnv *UNUSED(e), jobject UNUSED(o), jint l, jint t, jint r, jint b) {
	((void)l);
	((void)t);
	((void)r);
	((void)b);
}
