#include <jni.h>

#include <sys/resource.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
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
struct cmd_msg {
	int8_t c;
	void *m;
};
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};
struct android_app {
  ANativeActivity* activity;
  AConfiguration* config;
  ALooper* looper;
  AInputQueue* inputQueue;
  ANativeWindow* window;

  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_t thread;

  int activityState;
  int destroyRequested;
  int msgpipe[2];
  int destroyed;

  ASensorManager* sensorManager;
  const ASensor* accelerometerSensor;
  ASensorEventQueue* sensorEventQueue;

	struct {
	  EGLDisplay display;
	  EGLConfig config;
	  EGLSurface surface;
	  EGLContext context;
	} egl_src;
  
  EGLint width, height;
  struct saved_state state;
};

enum {
  LOOPER_ID_MAIN = 1,
  LOOPER_ID_INPUT = 2,
  LOOPER_ID_USER = 3,
};
enum {
	APP_CMD_NONE = 0,
  APP_CMD_INPUT_CHANGED,
  APP_CMD_INIT_WINDOW,
  APP_CMD_TERM_WINDOW,
  APP_CMD_WINDOW_RESIZED,
  APP_CMD_WINDOW_REDRAW_NEEDED,
  APP_CMD_CONTENT_RECT_CHANGED,
  APP_CMD_FOCUS_CHANGED,
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
static inline int egl_invalid (struct android_app *android_app) {
	return
		(android_app->egl_src->display == EGL_NO_DISPLAY) && 
		(android_app->egl_src->surface == EGL_NO_SURFACE) && 
		(android_app->egl_src->context == EGL_NO_CONTEXT);
}
static int engine_init_egl(struct android_app *android_app) {
	if (android_app->window == NULL) return -1;
	if (android_app->egl_src->display == EGL_NO_DISPLAY) {
	  // initialize EGL display
		EGLint temp[2];
	  android_app->egl_src->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	  if (android_app->egl_src->display == EGL_NO_DISPLAY) {
	  	LOGE("Failed get default egl display");
	  	return -1;
	  }
	  if (!eglInitialize(android_app->egl_src->display, temp, temp+1)) {
	  	switch (eglGetError()) {
	  		case EGL_NOT_INITIALIZED:
	  			LOGE("EGL not initialized!");
	  			return -1;
	  		case EGL_BAD_DISPLAY:
	  			LOGE("EGL Display is bad!");
	  			android_app->egl_src->display = EGL_NO_DISPLAY;
	  			return engine_init_egl(android_app);
	  	}
	  }
	  if (temp[0] < 1 || temp[1] < 2) {
	  	LOGE("EGL version is %d.%d, that lower than 1.2", temp[0], temp[1]);
	  	return -1;
	  }
	  // get config for new display
	  const EGLint attribs[] = {EGL_CONFORMANT, EGL_OPENGL_ES2_BIT, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE};
	  eglChooseConfig(android_app->egl_src->display, attribs, 0,0, temp);
	  if (!temp[0]) {
	  	LOGE("No supported EGLConfig for current display");
	  	return -1;
	  }
	  EGLConfig *supportedConfigs = (EGLConfig*)new_mem(sizeof(EGLConfig) * temp[0]);
	  eglChooseConfig(android_app->egl_src->display, attribs, supportedConfigs, temp[0], temp);
	  android_app->egl_src->config = supportedConfigs[0];
	  const EGLint observe_attribs[] = {EGL_RED_SIZE, EGL_GREEN_SIZE, EGL_BLUE_SIZE, EGL_DEPTH_SIZE };
	  EGLint best_value = -1, current_value, observe;
	  for (EGLint i = 0; i < temp[0]; ++i) {
	  	current_value = 0;
	  	for (EGLint j = 0, k = sizeof(observe_attribs)/sizeof(EGLint); j < k; ++j) {
	      if (eglGetConfigAttrib(android_app->egl_src->display, supportedConfigs[i], observe_attribs[j], &observe))
	      	current_value += observe;
	    }
	    if (best_value < current_value) {
	    	best_value = current_value;
	    	android_app->egl_src->config = supportedConfigs[i];
	    }
	  }
	  free_mem(supportedConfigs);
	  // eglGetConfigAttrib(android_app->egl_src->display, config, EGL_NATIVE_VISUAL_ID, temp);
	}
  // create surface
  if (android_app->egl_src->surface == EGL_NO_SURFACE)
  	android_app->egl_src->surface = eglCreateWindowSurface(android_app->egl_src->display, android_app->egl_src->config, android_app->window, NULL);

  // create context
  if (android_app->egl_src->context == EGL_NO_CONTEXT)
  	android_app->egl_src->context = eglCreateContext(android_app->egl_src->display, android_app->egl_src->config, NULL, NULL);


  if (eglMakeCurrent(android_app->egl_src->display, android_app->egl_src->surface, android_app->egl_src->surface, android_app->egl_src->context) == EGL_FALSE) {
    android_app->egl_src->context = EGL_NO_CONTEXT;
    android_app->egl_src->surface = EGL_NO_SURFACE;
    LOGE("Unable to eglMakeCurrent");
    return -1;
  }
  eglQuerySurface(android_app->egl_src->display, android_app->egl_src->surface, EGL_WIDTH, &android_app->width);
  eglQuerySurface(android_app->egl_src->display, android_app->egl_src->surface, EGL_HEIGHT, &android_app->height);
  android_app->state.angle = 0;

  // Initialize GL state.
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  glEnable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  return 0;
}
static void engine_update_frame(struct android_app *android_app) {

  android_app->state.angle += .01f;
  if (android_app->state.angle > 1) {
      android_app->state.angle = 0;
  }

  glClearColor(((float)android_app->state.x)/android_app->width, android_app->state.angle,
               ((float)android_app->state.y)/android_app->height, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  eglSwapBuffers(android_app->egl_src->display, android_app->egl_src->surface);
}
static inline void engine_term_egl(struct android_app *android_app, int all) {
	if (android_app->egl_src->display == EGL_NO_DISPLAY) return;
  eglMakeCurrent(android_app->egl_src->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  if (android_app->egl_src->context != EGL_NO_CONTEXT) {
    eglDestroyContext(android_app->egl_src->display, android_app->egl_src->context);
  	android_app->egl_src->context = EGL_NO_CONTEXT;
  }
  if (android_app->egl_src->surface != EGL_NO_SURFACE) {
    eglDestroySurface(android_app->egl_src->display, android_app->egl_src->surface);
  	android_app->egl_src->surface = EGL_NO_SURFACE;
  }
    // terminate EGL display
  if (all) {
    eglTerminate(android_app->egl_src->display);
  	android_app->egl_src->display = EGL_NO_DISPLAY;
  }
}

#include <dlfcn.h>
ASensorManager* AcquireASensorManagerInstance(struct android_app* app) {
  if(!app) return 0;
  typedef ASensorManager *(*PF_GETINSTANCEFORPACKAGE)(const char *name);
  void* androidHandle = dlopen("libandroid.so", RTLD_NOW);
  PF_GETINSTANCEFORPACKAGE getInstanceForPackageFunc = (PF_GETINSTANCEFORPACKAGE) dlsym(androidHandle, "ASensorManager_getInstanceForPackage");
  if (getInstanceForPackageFunc) {
    JNIEnv* env = 0;
    (*app->activity->vm)->AttachCurrentThread(app->activity->vm, &env, NULL);

    jclass android_content_Context = (*env)->GetObjectClass(env, app->activity->clazz);
    jmethodID midGetPackageName = (*env)->GetMethodID(env, android_content_Context,"getPackageName", "()Ljava/lang/String;");
    jstring packageName= (jstring)(*env)->CallObjectMethod(env, app->activity->clazz, midGetPackageName);

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
// sensor looper execute
int sensor_exec(int UNUSED(fd), int UNUSED(e), void *data) {
	struct android_app* android_app = (struct android_app*) data;
	((void)android_app);
	return 1;
}
// input looper execute
int input_exec(int UNUSED(fd), int UNUSED(e), void *data) {
	AInputEvent* event = NULL;
	struct android_app* android_app = (struct android_app*) data;
	if (AInputQueue_getEvent(android_app->inputQueue, &event) >= 0) {
	  LOGV("New input event: type=%d\n", AInputEvent_getType(event));
	  if (!AInputQueue_preDispatchEvent(android_app->inputQueue, event)) {
	    int32_t handled = 0;
	    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
	      android_app->state.x = AMotionEvent_getX(event, 0);
	      android_app->state.y = AMotionEvent_getY(event, 0);
	      handled = 1;
	    }
	    AInputQueue_finishEvent(android_app->inputQueue, event, handled);
	  }
	} else {
	  LOGE("Failure reading next input event: %s\n", strerror(errno));
	}
	return 1;
}
// cmd looper execute to read input from fd
int cmd_exec(int fd, int UNUSED(e), void *data) {
	struct android_app* android_app = (struct android_app*) data;
	struct cmd_msg cmd;
	int ret = 1; // allow to update frame
  if (read(fd, &cmd, sizeof(struct cmd_msg)) == sizeof(struct cmd_msg)) {
		switch (cmd.c) {
			case APP_CMD_INPUT_CHANGED:
			  LOGV("APP_CMD_INPUT_CHANGED\n");
			  if (android_app->inputQueue != NULL) {
			    AInputQueue_detachLooper(android_app->inputQueue);
			  }
			  android_app->inputQueue = (AInputQueue*)cmd.m;
			  if (android_app->inputQueue != NULL) {
			    LOGV("Attaching input queue to looper");
			    AInputQueue_attachLooper(android_app->inputQueue, android_app->looper, LOOPER_ID_INPUT, input_exec, android_app);
			  }
			  break;
			
			case APP_CMD_INIT_WINDOW:
			  LOGV("APP_CMD_INIT_WINDOW\n");
			  android_app->window = (ANativeWindow*)cmd.m;
			  // The window is being shown, get it ready.
			  engine_init_egl(android_app);
			  break;
			
			case APP_CMD_TERM_WINDOW:
			  LOGV("APP_CMD_TERM_WINDOW\n");
			  // The window is being hidden or closed, clean it up.
			  engine_term_egl(android_app, 0);
			  // window null
			  LOGV("APP_CMD_TERM_WINDOW\n");
		    android_app->window = NULL;
		    ret = 0;
		    break;
			
			case APP_CMD_CONFIG_CHANGED:
			  LOGV("APP_CMD_CONFIG_CHANGED\n");
			  AConfiguration_fromAssetManager(android_app->config, android_app->activity->assetManager);
			  print_cur_config(android_app);
			  break;
	
			case APP_CMD_FOCUS_CHANGED:
				if ((intptr_t)cmd.m != 0) {
				  if (android_app->accelerometerSensor != NULL) {
				    ASensorEventQueue_enableSensor(android_app->sensorEventQueue, android_app->accelerometerSensor);
				    ASensorEventQueue_setEventRate(android_app->sensorEventQueue, android_app->accelerometerSensor, (1000L/60)*1000);
				  }
				} else {
				  if (android_app->accelerometerSensor != NULL) {
				    ASensorEventQueue_disableSensor(android_app->sensorEventQueue, android_app->accelerometerSensor);
				  }
				}
			  break;
	
			case APP_CMD_DESTROY:
			  LOGV("APP_CMD_DESTROY\n");
			  android_app->destroyRequested = 1;
			  ret = 0;
			  break;
		}
		LOGV("activity state=%d\n", cmd.c);
	  pthread_mutex_lock(&android_app->mutex);
	  android_app->activityState = cmd.c;
	  pthread_cond_broadcast(&android_app->cond);
	  pthread_mutex_unlock(&android_app->mutex);
  } else
    LOGE("No data on command pipe!");
	return ret;
}
// running on other thread
static void* android_app_entry(void* param) {
	struct android_app* android_app = (struct android_app*) param;
	android_app->config = AConfiguration_new();
  AConfiguration_fromAssetManager(android_app->config, android_app->activity->assetManager);
  print_cur_config(android_app);
  ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
  ALooper_addFd(looper, android_app->msgpipe[0], LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, cmd_exec, android_app);
  android_app->looper = looper;

  // Prepare to monitor accelerometer
  android_app->sensorManager = AcquireASensorManagerInstance(android_app);
  android_app->accelerometerSensor = ASensorManager_getDefaultSensor(android_app->sensorManager, ASENSOR_TYPE_ACCELEROMETER);
  android_app->sensorEventQueue = ASensorManager_createEventQueue(android_app->sensorManager, android_app->looper, LOOPER_ID_USER, sensor_exec, android_app);

  // game loop while checking if game requested to exit
  while (!android_app->destroyRequested) {
    if (ALooper_pollOnce(egl_invalid(android_app) * -1, NULL, NULL, NULL)) {
	    if (!egl_invalid(android_app)) {
	      engine_update_frame(android_app);
	    }
    }
  }
  engine_term_egl(android_app, 1);
  LOGV("android_app_destroy!");
  if (android_app->inputQueue != NULL) {
    AInputQueue_detachLooper(android_app->inputQueue);
  }
  AConfiguration_delete(android_app->config);
  pthread_mutex_lock(&android_app->mutex);
  android_app->destroyed = 1;
  pthread_cond_broadcast(&android_app->cond);
  pthread_mutex_unlock(&android_app->mutex);
  // Can't touch android_app object after this.
  return NULL;
}

static inline void android_app_write_cmd(struct android_app* android_app, struct cmd_msg cmd) {
  android_app->activityState = APP_CMD_NONE;
  if (write(android_app->msgpipe[1], &cmd, sizeof(struct cmd_msg)) != sizeof(struct cmd_msg)) {
    LOGE("Failure writing android_app cmd: %s\n", strerror(errno));
  }
  pthread_mutex_lock(&android_app->mutex);
  while (android_app->activityState != cmd.c) {
    pthread_cond_wait(&android_app->cond, &android_app->mutex);
  }
  pthread_mutex_unlock(&android_app->mutex);
}

static void onDestroy(ANativeActivity* activity) {
  LOGV("Destroy: %p\n", activity);
  struct android_app* android_app = (struct android_app*)activity->instance;
  android_app_write_cmd(android_app, (struct cmd_msg){APP_CMD_DESTROY, NULL});
  pthread_mutex_lock(&android_app->mutex);
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
  android_app_write_cmd((struct android_app*)activity->instance, (struct cmd_msg){APP_CMD_START, NULL});
}
static void onResume(ANativeActivity* activity) {
  LOGV("Resume: %p\n", activity);
  android_app_write_cmd((struct android_app*)activity->instance, (struct cmd_msg){APP_CMD_RESUME, NULL});
}
static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
  LOGV("SaveInstanceState: %p\n", activity);
  struct android_app* android_app = (struct android_app*)activity->instance;
  void* savedState = NULL;
  *outLen = sizeof(struct saved_state);

  android_app_write_cmd(android_app, (struct cmd_msg){APP_CMD_SAVE_STATE, NULL});
  pthread_mutex_lock(&android_app->mutex);
  savedState = new_mem(sizeof(struct saved_state));
  memcpy(savedState, &android_app->state, sizeof(struct saved_state));
  pthread_mutex_unlock(&android_app->mutex);

  return savedState;
}
static void onPause(ANativeActivity* activity) {
  LOGV("Pause: %p\n", activity);
  android_app_write_cmd((struct android_app*)activity->instance, (struct cmd_msg){APP_CMD_PAUSE, NULL});
}
static void onStop(ANativeActivity* activity) {
  LOGV("Stop: %p\n", activity);
  android_app_write_cmd((struct android_app*)activity->instance, (struct cmd_msg){APP_CMD_STOP, NULL});
}
static void onConfigurationChanged(ANativeActivity* activity) {
  LOGV("ConfigurationChanged: %p\n", activity);
  android_app_write_cmd((struct android_app*)activity->instance, (struct cmd_msg){APP_CMD_CONFIG_CHANGED, NULL});
}
static void onLowMemory(ANativeActivity* activity) {
  struct android_app* android_app = (struct android_app*)activity->instance;
  LOGV("LowMemory: %p\n", activity);
  android_app_write_cmd(android_app, (struct cmd_msg){APP_CMD_LOW_MEMORY, NULL});
}
static void onContentRectChanged(ANativeActivity* activity, const ARect *UNUSED(rect)) {
  LOGV("LowMemory: %p\n", activity);
  android_app_write_cmd((struct android_app*)activity->instance, (struct cmd_msg){APP_CMD_CONTENT_RECT_CHANGED, NULL});
}
static void onWindowFocusChanged(ANativeActivity* activity, int focused) {
  LOGV("WindowFocusChanged: %p -- %d\n", activity, focused);
  intptr_t f = focused;
  android_app_write_cmd((struct android_app*)activity->instance, (struct cmd_msg){APP_CMD_FOCUS_CHANGED,(void*)f});
}
static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
  LOGV("NativeWindowCreated: %p -- %p\n", activity, window);
	android_app_write_cmd((struct android_app*)activity->instance, (struct cmd_msg){APP_CMD_INIT_WINDOW, window});
}
static void onNativeWindowRedrawNeeded(ANativeActivity* activity, ANativeWindow* window) {
	LOGV("NativeWindowResized: %p -- %p\n", activity, window);
	((void)window);
	android_app_write_cmd((struct android_app*)activity->instance, (struct cmd_msg){APP_CMD_WINDOW_REDRAW_NEEDED, NULL});
}
static void onNativeWindowResized(ANativeActivity* activity, ANativeWindow* window) {
	LOGV("NativeWindowResized: %p -- %p\n", activity, window);
	((void)window);
	android_app_write_cmd((struct android_app*)activity->instance, (struct cmd_msg){APP_CMD_WINDOW_RESIZED, NULL});
}
static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
	LOGV("NativeWindowDestroyed: %p -- %p\n", activity, window);
	((void)window);
	android_app_write_cmd((struct android_app*)activity->instance, (struct cmd_msg){APP_CMD_TERM_WINDOW, NULL});
}
static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
  LOGV("InputQueueCreated: %p -- %p\n", activity, queue);
	android_app_write_cmd((struct android_app*)activity->instance, (struct cmd_msg){APP_CMD_INPUT_CHANGED, queue});
}
static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
	LOGV("InputQueueDestroyed: %p -- %p\n", activity, queue);
	((void)queue);
	android_app_write_cmd((struct android_app*)activity->instance, (struct cmd_msg){APP_CMD_INPUT_CHANGED, NULL});
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
  activity->callbacks->onContentRectChanged = onContentRectChanged;
  activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
  activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
  activity->callbacks->onNativeWindowResized = onNativeWindowResized;
  activity->callbacks->onNativeWindowRedrawNeeded = onNativeWindowRedrawNeeded;
  activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
  activity->callbacks->onInputQueueCreated = onInputQueueCreated;
  activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;

  struct android_app* android_app = (struct android_app*)new_imem(sizeof(struct android_app));
  activity->instance = android_app;
  android_app->activity = activity;

  pthread_mutex_init(&android_app->mutex, NULL);
  pthread_cond_init(&android_app->cond, NULL);

  if (savedState != NULL) {
    memcpy(&android_app->state, savedState, savedStateSize);
  }
  free_mem(savedState);
  if (pipe(android_app->msgpipe)) {
    LOGE("could not create pipe: %s", strerror(errno));
    return;
  }
  
  // start thread
  pthread_attr_t attr; 
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&android_app->thread, &attr, android_app_entry, android_app);
}

// JNI
JNIEXPORT void JNICALL Java_com_ariasaproject_technowar_MainActivity_insetNative (JNIEnv *UNUSED(e), jobject UNUSED(o), jint l, jint t, jint r, jint b) {
	((void)l);
	((void)t);
	((void)r);
	((void)b);
}
