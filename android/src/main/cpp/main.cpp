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
	struct android_poll_source {
	    int32_t id;
	    struct android_app* app;
	    void (*process)(android_app*, android_poll_source*);
	};
	struct android_app {
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
	    struct android_poll_source cmdPollSource;
	    struct android_poll_source inputPollSource;
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
    struct android_app* app;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
};
static int engine_init_display(struct engine* engine) {
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, format;
    EGLint numConfigs;
    EGLConfig config = nullptr;
    EGLSurface surface;
    EGLContext context;
    
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);
    eglChooseConfig(display, attribs, nullptr,0, &numConfigs);
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    assert(supportedConfigs);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);
    assert(numConfigs);
    auto i = 0;
    for (; i < numConfigs; i++) {
        auto& cfg = supportedConfigs[i];
        EGLint r, g, b, d;
        if (eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r)   &&
          eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g) &&
          eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b)  &&
          eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d) &&
          r == 8 && g == 8 && b == 8 && d == 0 ) {
          config = supportedConfigs[i];
          break;
        }
    }
    if (i == numConfigs) {
        config = supportedConfigs[0];
    }


    if (config == nullptr) {
        LOGW("Unable to initialize EGLConfig");
        return -1;
    }
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    surface = eglCreateWindowSurface(display, config, engine->app->window, nullptr);
    context = eglCreateContext(display, config, nullptr, nullptr);
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

    return 0;
}
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == nullptr) {
        return;
    }


    // Just fill the screen with a color.
    glClearColor(((float)engine->state.x)/engine->width, engine->state.angle,
                 ((float)engine->state.y)/engine->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);


    eglSwapBuffers(engine->display, engine->surface);
}
static void engine_term_display(struct engine* engine) {
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
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    auto* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    auto* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != nullptr) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != nullptr) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                                               engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                                               engine->accelerometerSensor,
                                               (1000L/60)*1000);
            }
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != nullptr) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                                                engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
        default:
            break;
    }
}


extern void android_main(android_app* state) {
    struct engine engine;
    state->userData = &engine;
    state->onInputEvent = engine_handle_input;
    engine.app = state;
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(app->sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(app->sensorManager, state->looper, LOOPER_ID_USER, nullptr, nullptr);
    if (state->savedState != nullptr) {
      engine.state = *(saved_state*)state->savedState;
    }
    while (true) {
        int ident;
        int events;
        android_poll_source* source;
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, nullptr, &events, (void**)&source)) >= 0) {
            if (source != nullptr) {
              source->process(state, source);
            }
            if (ident == LOOPER_ID_USER) {
              if (engine.accelerometerSensor != nullptr) {
                ASensorEvent event;
                while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0) {
                  LOGI("accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
                }
              }
            }


            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }


        if (engine.animating) {
            engine.state.angle += .01f;
            if (engine.state.angle > 1) {
                engine.state.angle = 0;
            }
            engine_draw_frame(&engine);
        }
    }
}

static void free_saved_state(struct android_app* android_app) {
    pthread_mutex_lock(&android_app->mutex);
    if (android_app->savedState != NULL) {
        free(android_app->savedState);
        android_app->savedState = NULL;
        android_app->savedStateSize = 0;
    }
    pthread_mutex_unlock(&android_app->mutex);
}
int8_t android_app_read_cmd(struct android_app* android_app) {
    int8_t cmd;
    if (read(android_app->msgread, &cmd, sizeof(cmd)) == sizeof(cmd)) {
        switch (cmd) {
            case APP_CMD_SAVE_STATE:
                free_saved_state(android_app);
                break;
        }
        return cmd;
    }
    return -1;
}
void android_app_pre_exec_cmd(struct android_app* android_app, int8_t cmd) {
    switch (cmd) {
        case APP_CMD_INPUT_CHANGED:
            LOGI("APP_CMD_INPUT_CHANGED\n");
            pthread_mutex_lock(&android_app->mutex);
            if (android_app->inputQueue != NULL) {
                AInputQueue_detachLooper(android_app->inputQueue);
            }
            android_app->inputQueue = android_app->pendingInputQueue;
            if (android_app->inputQueue != NULL) {
                LOGI("Attaching input queue to looper");
                AInputQueue_attachLooper(android_app->inputQueue,
                        android_app->looper, LOOPER_ID_INPUT, NULL,
                        &android_app->inputPollSource);
            }
            pthread_cond_broadcast(&android_app->cond);
            pthread_mutex_unlock(&android_app->mutex);
            break;
        case APP_CMD_INIT_WINDOW:
            LOGI("APP_CMD_INIT_WINDOW\n");
            pthread_mutex_lock(&android_app->mutex);
            android_app->window = android_app->pendingWindow;
            pthread_cond_broadcast(&android_app->cond);
            pthread_mutex_unlock(&android_app->mutex);
            break;
        case APP_CMD_TERM_WINDOW:
            LOGI("APP_CMD_TERM_WINDOW\n");
            pthread_mutex_lock(&android_app->mutex);
            android_app->window = NULL;
            pthread_cond_broadcast(&android_app->cond);
            pthread_mutex_unlock(&android_app->mutex);
            break;
        case APP_CMD_RESUME:
        case APP_CMD_START:
        case APP_CMD_PAUSE:
        case APP_CMD_STOP:
            LOGI("activityState=%d\n", cmd);
            pthread_mutex_lock(&android_app->mutex);
            android_app->activityState = cmd;
            pthread_cond_broadcast(&android_app->cond);
            pthread_mutex_unlock(&android_app->mutex);
            break;
        case APP_CMD_CONFIG_CHANGED:
            LOGI("APP_CMD_CONFIG_CHANGED\n");
            //AConfiguration_fromAssetManager(android_app->config, android_app->activity->assetManager);
            break;
        case APP_CMD_DESTROY:
            LOGI("APP_CMD_DESTROY\n");
            android_app->destroyRequested = 1;
            break;
    }
}
void android_app_post_exec_cmd(struct android_app* android_app, int8_t cmd) {
    switch (cmd) {
        case APP_CMD_TERM_WINDOW:
            LOGI("APP_CMD_TERM_WINDOW\n");
            pthread_mutex_lock(&android_app->mutex);
            android_app->window = NULL;
            pthread_cond_broadcast(&android_app->cond);
            pthread_mutex_unlock(&android_app->mutex);
            break;
        case APP_CMD_SAVE_STATE:
            LOGI("APP_CMD_SAVE_STATE\n");
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

static void android_app_destroy(struct android_app* android_app) {
    free_saved_state(android_app);
    pthread_mutex_lock(&android_app->mutex);
    if (android_app->inputQueue != NULL) {
        AInputQueue_detachLooper(android_app->inputQueue);
    }
    AConfiguration_delete(android_app->config);
    android_app->destroyed = 1;
    pthread_cond_broadcast(&android_app->cond);
    pthread_mutex_unlock(&android_app->mutex);
}
static void process_input(struct android_app* app, struct android_poll_source* source) {
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
static void process_cmd(android_app* app, struct android_poll_source* source) {
    int8_t cmd = android_app_read_cmd(app);
    android_app_pre_exec_cmd(app, cmd);
    engine_handle_cmd(app, cmd);
    android_app_post_exec_cmd(app, cmd);
}
static void* android_app_entry(void* param) {
    android_app* app = (android_app*)param;
    app->config = AConfiguration_new();
    //AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
    app->cmdPollSource.id = LOOPER_ID_MAIN;
    app->cmdPollSource.app = android_app;
    app->cmdPollSource.process = process_cmd;
    app->inputPollSource.id = LOOPER_ID_INPUT;
    app->inputPollSource.app = android_app;
    app->inputPollSource.process = process_input;
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
static void android_app_write_cmd(struct android_app* android_app, int8_t cmd) {
  if (write(android_app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd)) {
      LOGI("Failure writing android_app cmd: %s\n", strerror(errno));
  }
}
static void android_app_set_activity_state(struct android_app* android_app, int8_t cmd) {
  pthread_mutex_lock(&android_app->mutex);
  android_app_write_cmd(android_app, cmd);
  while (android_app->activityState != cmd) {
    pthread_cond_wait(&android_app->cond, &android_app->mutex);
  }
  pthread_mutex_unlock(&android_app->mutex);
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

#define JEx(R, M) extern "C" JNIEXPORT R JNICALL Java_com_ariasaproject_technowar_AndroidApplication_##M

JEx(jlong, onCreateNative) (JNIEnv *e, jobject o, jbyteArray ss, jint savedStateSize) {
	android_app* app = new android_app;
	char *savedState = (char *) env->GetPrimitiveArrayCritical(ss, 0);
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
  app->sensorManager = AcquireASensorManagerInstance(e, o);
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
  env->ReleasePrimitiveArrayCritical(ss, savedState, 0);
  return app;
}
JEx(void, onStartNative) (JNIEnv *e, jobject o, jlong app) {
  android_app_set_activity_state((android_app*)app, APP_CMD_START);
}
JEx(void, onResumeNative) (JNIEnv *e, jobject o) {
  android_app_set_activity_state((android_app*)app, APP_CMD_RESUME);
}
JEx(jbyteArray, onSaveInstanceStateNative) (JNIEnv *e, jobject o, jlong appPtr) {
	android_app* app = (android_app*)appPtr;
  jbyteArray arr = NULL;
  pthread_mutex_lock(&app->mutex);
  app->stateSaved = 0;
  android_app_write_cmd(app, APP_CMD_SAVE_STATE);
  while (!app->stateSaved) {
    pthread_cond_wait(&app->cond, &app->mutex);
  }
  if (app->savedState != NULL) {
	  arr = env->NewByteArray(app->savedStateSize);
	  env->SetByteArrayRegion(arr, 0, app->savedStateSize, reinterpret_cast<jbyte*>(app->savedState));
    app->savedState = NULL;
    app->savedStateSize = 0;
  }
  pthread_mutex_unlock(&app->mutex);
  return arr;
}
JEx(void, onPauseNative) (JNIEnv *e, jobject o, jlong appPtr, jboolean finish) {
	android_app *app = (android_app*)appPtr;
  if (finish) {
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
  } else {
  	android_app_set_activity_state(app, APP_CMD_PAUSE);
  }
}
JEx(void, onStopNative) (JNIEnv *e, jobject o) {
	android_app_set_activity_state((android_app*)app, APP_CMD_STOP);
}
JEx(void, onConfigurationChangedNative) (JNIEnv *e, jobject o) {
  android_app_write_cmd((android_app*)app, APP_CMD_CONFIG_CHANGED);
}
JEx(void, onLowMemoryNative) (JNIEnv *e, jobject o, jlong app) {
  android_app_write_cmd((android_app*)app, APP_CMD_LOW_MEMORY);
}
JEx(void, onWindowFocusChangedNative) (JNIEnv *e, jobject o, jlong app, jboolean focused) {
  android_app_write_cmd((android_app*)app, focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}
JEx(void, onSurfaceSetNative) (JNIEnv *e, jobject o, jlong appPtr, jobject surface) {
	android_app *app = (android_app*)appPtr;
  pthread_mutex_lock(&app->mutex);
  if (android_app->pendingWindow != NULL) {
    android_app_write_cmd(app, APP_CMD_TERM_WINDOW);
  }
  if (surface) {
  	app->pendingWindow = ANativeWindow_fromSurface(e, surface);
  } else {
  	ANativeWindow_release(app->pendingWindow);
  	app->pendingWindow = NULL;
  }
  if (app->pendingWindow != NULL) {
    android_app_write_cmd(app, APP_CMD_INIT_WINDOW);
  }
  while (app->window != app->pendingWindow) {
    pthread_cond_wait(&app->cond, &app->mutex);
  }
  pthread_mutex_unlock(&app->mutex);
}
JEx(void, onSurfaceChangedNative) (JNIEnv *e, jobject o, jlong app, jint format, jint width, jint height) {
  android_app_write_cmd((android_app*)app, APP_CMD_WINDOW_RESIZED);
}
JEx(void, onInputQueueSetNative) (JNIEnv *e, jobject o, jlong appPtr, jlong queuePtr) {
  android_app *app = (android_app*)appPtr;
  pthread_mutex_lock(&app->mutex);
  android_app->pendingInputQueue = queuePtr!=nullptr?(AInputQueue*)queuePtr:NULL;
  android_app_write_cmd(app, APP_CMD_INPUT_CHANGED);
  while (app->inputQueue != app->pendingInputQueue) {
    pthread_cond_wait(&app->cond, &app->mutex);
  }
  pthread_mutex_unlock(&app->mutex);
}
JEx(void, onContentRectChangedNative) (JNIEnv *e, jobject o, jlong app, jint x, jint y, jint w, jint h) {
  android_app_write_cmd((android_app*)app, APP_CMD_CONTENT_RECT_CHANGED);
}


