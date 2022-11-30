#include <android/configuration.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/sensor.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cassert>
#include <cstring>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <initializer_list>
#include <jni.h>
#include <memory>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>

// make opengles lastest possible version
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

extern "C" {
	struct android_app;
	struct engine;
	typedef void (*source_process)(android_app*, engine*);
	struct android_app {
		ANativeActivity *activity;
    void* userData;
    int32_t (*onInputEvent)(struct android_app* app, AInputEvent* event);
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
    int msgread;
    int msgwrite;
    pthread_t thread;
    source_process cmdPollSource;
    source_process inputPollSource;
    int running;
    int stateSaved;
    int destroyed;
    AInputQueue* pendingInputQueue;
    ANativeWindow* pendingWindow;
    ARect pendingContentRect;
  	ASensorManager* sensorManager;
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
	    APP_CMD_DESTROY
	};
}
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};
struct engine {
    android_app* app;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;
    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    saved_state state;
};
static int engine_init_display(engine* eng) {
  EGLConfig config = nullptr;
  EGLSurface surface;
  EGLContext context;
  
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglInitialize(display, nullptr, nullptr);
  EGLint temp;
  const EGLint configAttr[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
    EGL_ALPHA_SIZE, 0,
    EGL_NONE
  };
  eglChooseConfig(display, configAttr, nullptr,0, &temp);
  assert(temp);
  EGLConfig *configs = (EGLConfig*) alloca(temp*sizeof(EGLConfig));
  assert(configs);
  eglChooseConfig(display, configAttr, configs, temp, &temp);
  assert(temp);
  config = configs[0];
  for (unsigned int i = 0, j = temp, k = 0, l; i < j; i++) {
    EGLConfig& cfg = configs[i];
    eglGetConfigAttrib(display, cfg, EGL_BUFFER_SIZE, &temp);
    l = temp;
    eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &temp);
    l += temp;
    eglGetConfigAttrib(display, cfg, EGL_STENCIL_SIZE, &temp);
    l += temp;
    if (l > k) {
      k = l;
      config = cfg;
    }
  }
  //eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
  const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  surface = eglCreateWindowSurface(display, config, eng->app->window, ctxAttr);
  context = eglCreateContext(display, config, nullptr, nullptr);
  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
      LOGW("Unable to eglMakeCurrent");
      return -1;
  }
  eglQuerySurface(display, surface, EGL_WIDTH, &eng->width);
  eglQuerySurface(display, surface, EGL_HEIGHT, &eng->height);
  eng->display = display;
  eng->context = context;
  eng->surface = surface;
  eng->state.angle = 0;

  return 0;
}
static void engine_draw_frame(engine* eng) {
    if (eng->display == nullptr) {
      return;
    }
    glClearColor(((float)eng->state.x)/eng->width, eng->state.angle,((float)eng->state.y)/eng->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(eng->display, eng->surface);
}
static void engine_term_display(engine* eng) {
  if (eng->display != EGL_NO_DISPLAY) {
    eglMakeCurrent(eng->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (eng->context != EGL_NO_CONTEXT) {
    	eglDestroyContext(eng->display, eng->context);
    }
    if (eng->surface != EGL_NO_SURFACE) {
      eglDestroySurface(eng->display, eng->surface);
    }
    eglTerminate(eng->display);
  }
  eng->animating = 0;
  eng->display = EGL_NO_DISPLAY;
  eng->context = EGL_NO_CONTEXT;
  eng->surface = EGL_NO_SURFACE;
}
static int32_t engine_handle_input(android_app* app, AInputEvent* event) {
    engine* eng = (engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        eng->animating = 1;
        eng->state.x = AMotionEvent_getX(event, 0);
        eng->state.y = AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

static void engine_handle_cmd(android_app* app, int32_t cmd) {
 engine* eng = (engine*)app->userData;
  switch (cmd) {
    case APP_CMD_SAVE_STATE:
      app->savedState = new saved_state;
      *((saved_state*)app->savedState) = eng->state;
      app->savedStateSize = sizeof(saved_state);
      break;
    case APP_CMD_INIT_WINDOW:
      if (app->window != nullptr) {
          engine_init_display(eng);
      }
      break;
    case APP_CMD_TERM_WINDOW:
      engine_term_display(eng);
      break;
    case APP_CMD_GAINED_FOCUS:
      if (eng->accelerometerSensor != nullptr) {
        ASensorEventQueue_enableSensor(eng->sensorEventQueue,eng->accelerometerSensor);
        ASensorEventQueue_setEventRate(eng->sensorEventQueue,eng->accelerometerSensor,(1000L/60)*1000);
      }
      break;
    case APP_CMD_LOST_FOCUS:
      if (eng->accelerometerSensor != nullptr) {
          ASensorEventQueue_disableSensor(eng->sensorEventQueue,eng->accelerometerSensor);
      }
      eng->animating = 0;
      engine_draw_frame(eng);
      break;
    default:
      break;
  }
}

static void android_main(android_app* app) {
    engine eng;
    app->onInputEvent = engine_handle_input;
    eng.app = app;
    eng.accelerometerSensor = ASensorManager_getDefaultSensor(app->sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    eng.sensorEventQueue = ASensorManager_createEventQueue(app->sensorManager, app->looper, LOOPER_ID_USER, nullptr, nullptr);
    if (app->savedState != nullptr) {
      eng.state = *(saved_state*)app->savedState;
    }
    while (true) {
        int ident;
        int events;
        source_process* source;
        while ((ident=ALooper_pollAll(eng.animating ? 0 : -1, nullptr, &events, (void**)&source)) >= 0) {
            if (source != nullptr) {
              (*source)(app, &eng);
            }
            if (ident == LOOPER_ID_USER) {
              if (eng.accelerometerSensor != nullptr) {
                ASensorEvent event;
                while (ASensorEventQueue_getEvents(eng.sensorEventQueue, &event, 1) > 0) {
                  LOGI("accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
                }
              }
            }


            // Check if we are exiting.
            if (app->destroyRequested != 0) {
                engine_term_display(&eng);
                return;
            }
        }


        if (eng.animating) {
            eng.state.angle += .01f;
            if (eng.state.angle > 1) {
                eng.state.angle = 0;
            }
            engine_draw_frame(&eng);
        }
    }
}

static void free_saved_state(android_app* app) {
  pthread_mutex_lock(&app->mutex);
  if (app->savedState != NULL) {
    free(app->savedState);
    app->savedState = NULL;
    app->savedStateSize = 0;
  }
  pthread_mutex_unlock(&app->mutex);
}
static int8_t android_app_read_cmd(android_app* app) {
  int8_t cmd;
  if (read(app->msgread, &cmd, sizeof(cmd)) == sizeof(cmd)) {
    switch (cmd) {
      case APP_CMD_SAVE_STATE:
        free_saved_state(app);
        break;
    }
    return cmd;
  }
  return -1;
}
static void android_app_pre_exec_cmd(android_app* app, int8_t cmd) {
	switch (cmd) {
    case APP_CMD_INPUT_CHANGED:
      pthread_mutex_lock(&app->mutex);
      if (app->inputQueue != NULL) {
        AInputQueue_detachLooper(app->inputQueue);
      }
      app->inputQueue = app->pendingInputQueue;
      if (app->inputQueue != NULL) {
        AInputQueue_attachLooper(app->inputQueue, app->looper, LOOPER_ID_INPUT, NULL, &app->inputPollSource);
      }
      pthread_cond_broadcast(&app->cond);
      pthread_mutex_unlock(&app->mutex);
      break;
    case APP_CMD_INIT_WINDOW:
      LOGI("APP_CMD_INIT_WINDOW\n");
      pthread_mutex_lock(&app->mutex);
      app->window = app->pendingWindow;
      pthread_cond_broadcast(&app->cond);
      pthread_mutex_unlock(&app->mutex);
      break;
    case APP_CMD_TERM_WINDOW:
      LOGI("APP_CMD_TERM_WINDOW\n");
      pthread_mutex_lock(&app->mutex);
      app->window = NULL;
      pthread_cond_broadcast(&app->cond);
      pthread_mutex_unlock(&app->mutex);
      break;
    case APP_CMD_RESUME:
    case APP_CMD_START:
    case APP_CMD_PAUSE:
    case APP_CMD_STOP:
      LOGI("activityState=%d\n", cmd);
      pthread_mutex_lock(&app->mutex);
      app->activityState = cmd;
      pthread_cond_broadcast(&app->cond);
      pthread_mutex_unlock(&app->mutex);
      break;
    case APP_CMD_CONFIG_CHANGED:
      LOGI("APP_CMD_CONFIG_CHANGED\n");
      AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
      break;
    case APP_CMD_DESTROY:
      LOGI("APP_CMD_DESTROY\n");
      app->destroyRequested = 1;
      break;
}
}
static void android_app_post_exec_cmd(android_app* app, int8_t cmd) {
    switch (cmd) {
        case APP_CMD_TERM_WINDOW:
          pthread_mutex_lock(&app->mutex);
          app->window = NULL;
          pthread_cond_broadcast(&app->cond);
          pthread_mutex_unlock(&app->mutex);
          break;
        case APP_CMD_SAVE_STATE:
          pthread_mutex_lock(&app->mutex);
          app->stateSaved = 1;
          pthread_cond_broadcast(&app->cond);
          pthread_mutex_unlock(&app->mutex);
          break;
        case APP_CMD_RESUME:
          free_saved_state(app);
          break;
    }
}

static void android_app_destroy(android_app* app) {
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
static void process_input(android_app* app) {
    AInputEvent* event = NULL;
    if (AInputQueue_getEvent(app->inputQueue, &event) >= 0) {
        LOGI("New input event: type=%d\n", AInputEvent_getType(event));
        if (AInputQueue_preDispatchEvent(app->inputQueue, event)) {
            return;
        }
        int32_t handled = 0;
        if (app->onInputEvent != NULL) handled = app->onInputEvent(app, event);
        AInputQueue_finishEvent(app->inputQueue, event, handled);
    } else {
        LOGI("Failure reading next input event: %s\n", strerror(errno));
    }
}
static void process_cmd(android_app* app) {
    int8_t cmd = android_app_read_cmd(app);
    android_app_pre_exec_cmd(app, cmd);
    engine_handle_cmd(app, cmd);
    android_app_post_exec_cmd(app, cmd);
}
static void* android_app_entry(void* param) {
    android_app* app = (android_app*)param;
    app->config = AConfiguration_new();
    AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
    app->cmdPollSource = process_cmd;
    app->inputPollSource = process_input;
    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(looper, app->msgread, LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL, &app->cmdPollSource);
    app->looper = looper;
    pthread_mutex_lock(&app->mutex);
    app->running = 1;
    pthread_cond_broadcast(&app->cond);
    pthread_mutex_unlock(&app->mutex);
    
    android_main(app);
    
    android_app_destroy(app);
    
    return NULL;
}
static void android_app_write_cmd(android_app* app, int8_t cmd) {
  if (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd)) {
      LOGI("Failure writing android_app cmd: %s\n", strerror(errno));
  }
}
static void android_app_set_activity_state(android_app* app, int8_t cmd) {
  pthread_mutex_lock(&app->mutex);
  android_app_write_cmd(app, cmd);
  while (app->activityState != cmd) {
    pthread_cond_wait(&app->cond, &app->mutex);
  }
  pthread_mutex_unlock(&app->mutex);
}
static void android_app_set_input(android_app* app, AInputQueue* inputQueue) {
    pthread_mutex_lock(&app->mutex);
    app->pendingInputQueue = inputQueue;
    android_app_write_cmd(app, APP_CMD_INPUT_CHANGED);
    while (app->inputQueue != app->pendingInputQueue) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void android_app_set_window(android_app* app, ANativeWindow* window) {
    pthread_mutex_lock(&app->mutex);
    if (app->pendingWindow != NULL) {
        android_app_write_cmd(app, APP_CMD_TERM_WINDOW);
    }
    app->pendingWindow = window;
    if (window != NULL) {
        android_app_write_cmd(app, APP_CMD_INIT_WINDOW);
    }
    while (app->window != app->pendingWindow) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}

/*
#include "translated_opengles.h"
#include "mainListener.h"

bool limitRenderer () {
	const char *r = tgf->renderer();
	return ((bool)strstr(r, "adreno")) || ((bool)strstr(r, "Adreno"));
}
*/

ASensorManager* AcquireASensorManagerInstance(JNIEnv *env, jobject o) {
  typedef ASensorManager *(*PF_GETINSTANCEFORPACKAGE)(const char *name);
  void* androidHandle = dlopen("libandroid.so", RTLD_NOW);
  auto getInstanceForPackageFunc = (PF_GETINSTANCEFORPACKAGE) dlsym(androidHandle, "ASensorManager_getInstanceForPackage");
  if (getInstanceForPackageFunc) {
    jclass android_content_Context = env->GetObjectClass(o);
    jmethodID midGetPackageName = env->GetMethodID(android_content_Context, "getPackageName", "()Ljava/lang/String;");
    auto packageName= (jstring)env->CallObjectMethod(o, midGetPackageName);
    const char *nativePackageName = env->GetStringUTFChars(packageName, nullptr);
    ASensorManager* mgr = getInstanceForPackageFunc(nativePackageName);
    env->ReleaseStringUTFChars(packageName, nativePackageName);
    if (mgr) {
      dlclose(androidHandle);
      return mgr;
    }
  }
  typedef ASensorManager *(*PF_GETINSTANCE)();
  auto getInstanceFunc = (PF_GETINSTANCE) dlsym(androidHandle, "ASensorManager_getInstance");
  assert(getInstanceFunc);
  dlclose(androidHandle);
  return getInstanceFunc();
}

static void *android_app_create (ANativeActivity *activity, void *savedState, size_t savedStateSize) {
	android_app* app = new android_app;
  pthread_mutex_init(&app->mutex, NULL);
  pthread_cond_init(&app->cond, NULL);
  if (savedState != NULL) {
    app->savedState = malloc(savedStateSize);
    app->savedStateSize = savedStateSize;
    memcpy(app->savedState, savedState, savedStateSize);
  }
  int msgpipe[2];
  if (pipe(msgpipe)) {
      LOGI("could not create pipe: %s", strerror(errno));
  }
  app->msgread = msgpipe[0];
  app->msgwrite = msgpipe[1];
  //sensor Manager
  app->sensorManager = AcquireASensorManagerInstance(activity->env, activity->clazz);
  //end
  pthread_attr_t attr; 
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&app->thread, &attr, android_app_entry, app);
  pthread_mutex_lock(&app->mutex);
  while (!app->running) {
    pthread_cond_wait(&app->cond, &app->mutex);
  }
  pthread_mutex_unlock(&app->mutex);
  return app;
}
static void onStart(ANativeActivity *activity) {
  android_app_set_activity_state((android_app*)activity->instance, APP_CMD_START);
}
static void onResume(ANativeActivity *activity) {
  android_app_set_activity_state((android_app*)activity->instance, APP_CMD_RESUME);
}
static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
  android_app* app = (android_app*)activity->instance;
  void* savedState = NULL;
  pthread_mutex_lock(&app->mutex);
  app->stateSaved = 0;
  android_app_write_cmd(app, APP_CMD_SAVE_STATE);
  while (!app->stateSaved) {
    pthread_cond_wait(&app->cond, &app->mutex);
  }
  if (app->savedState != NULL) {
    savedState = app->savedState;
    *outLen = app->savedStateSize;
    app->savedState = NULL;
    app->savedStateSize = 0;
  }
  pthread_mutex_unlock(&app->mutex);
}  
static void onPause(ANativeActivity *activity) {
	android_app_set_activity_state((android_app*)activity->instance, APP_CMD_PAUSE);
}
static void onDestroy(ANativeActivity *activity) {
	android_app *app = (android_app*)activity->instance;
	pthread_mutex_lock(&app->mutex);
  android_app_write_cmd(app, APP_CMD_DESTROY);
  while (!app->destroyed) {
    pthread_cond_wait(&app->cond, &app->mutex);
  }
  pthread_mutex_unlock(&app->mutex);
  close(app->msgread);
  close(app->msgwrite);
  pthread_cond_destroy(&app->cond);
  pthread_mutex_destroy(&app->mutex);
  delete app;
  activity->instance = nullptr;
}
static void onStop(ANativeActivity* activity) {
  android_app_set_activity_state((android_app*)activity->instance, APP_CMD_STOP);
}
static void onConfigurationChanged(ANativeActivity* activity) {
  android_app_write_cmd((android_app*)activity->instance, APP_CMD_CONFIG_CHANGED);
}
static void onLowMemory(ANativeActivity* activity) {
  android_app_write_cmd((android_app*)activity->instance, APP_CMD_LOW_MEMORY);
}
static void onWindowFocusChanged(ANativeActivity* activity, int focused) {
  android_app_write_cmd((android_app*)activity->instance, focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}
static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
  android_app_set_window((android_app*)activity->instance, window);
}
static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
  android_app_set_window((android_app*)activity->instance, NULL);
}
static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
	android_app_set_input((android_app*)activity->instance, queue);
}
static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
  android_app_set_input((android_app*)activity->instance, NULL);
}

void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
  activity->callbacks->onStart = onStart;
  activity->callbacks->onResume = onResume;
  activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
  activity->callbacks->onInputQueueCreated = onInputQueueCreated;
  activity->callbacks->onConfigurationChanged = onConfigurationChanged;
  activity->callbacks->onLowMemory = onLowMemory;
  activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
  activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
  activity->callbacks->onSaveInstanceState = onSaveInstanceState;
  activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
  activity->callbacks->onPause = onPause;
  activity->callbacks->onStop = onStop;
  activity->callbacks->onDestroy = onDestroy;
  //on Create
  activity->instance = android_app_create(activity, savedState, savedStateSize);
}
