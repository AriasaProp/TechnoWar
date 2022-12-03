#include <initializer_list>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <jni.h>
#include <cerrno>
#include <cassert>


#include <EGL/egl.h>
#include <GLES3/gl3.h>


#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

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
    bool animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
};

static int engine_init_display(struct engine* engine) {
    EGLint numConfigs;
    EGLConfig config = nullptr;
    EGLSurface surface;
    EGLContext context;
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    eglChooseConfig(display, attribs, nullptr,0, &numConfigs);
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    assert(supportedConfigs);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);
    assert(numConfigs);
    auto i = 0;
    for (; i < numConfigs; i++) {
        EGLConfig& cfg = supportedConfigs[i];
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
    surface = eglCreateWindowSurface(display, config, engine->app->window, nullptr);
    context = eglCreateContext(display, config, nullptr, nullptr);
    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }
    eglQuerySurface(display, surface, EGL_WIDTH, &engine->width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &engine->height);
    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->state.angle = 0;

    return 0;
}

static void engine_draw_frame(engine* engine) {
    if (!engine->display) {
        return;
    }
    glClearColor(((float)engine->state.x)/engine->width, engine->state.angle ((float)engine->state.y)/engine->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);
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
    engine->animating = false;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    engine* engine = (engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = true;
        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    engine* engine = (engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            app->savedState = malloc(sizeof(saved_state));
            *((saved_state*)app->savedState) = engine->state;
            app->savedStateSize = sizeof(saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            if (engine->app->window) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            if (engine->accelerometerSensor) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,engine->accelerometerSensor);
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,engine->accelerometerSensor,(1000L/60)*1000);
            }
            break;
        case APP_CMD_LOST_FOCUS:
            if (engine->accelerometerSensor) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,engine->accelerometerSensor);
            }
            engine->animating = false;
            engine_draw_frame(engine);
            break;
        default:
            break;
    }
}
/*
#include <dlfcn.h>
ASensorManager* AcquireASensorManagerInstance(android_app* app) {
  if(!app)
    return nullptr;
  typedef ASensorManager *(*PF_GETINSTANCEFORPACKAGE)(const char *name);
  void* androidHandle = dlopen("libandroid.so", RTLD_NOW);
  auto getInstanceForPackageFunc = (PF_GETINSTANCEFORPACKAGE)dlsym(androidHandle, "ASensorManager_getInstanceForPackage");
  if (getInstanceForPackageFunc) {
    JNIEnv* env = nullptr;
    app->activity->vm->AttachCurrentThread(&env, nullptr);
    jclass android_content_Context = env->GetObjectClass(app->activity->clazz);
    jmethodID midGetPackageName = env->GetMethodID(android_content_Context,"getPackageName","()Ljava/lang/String;");
    auto packageName= (jstring)env->CallObjectMethod(app->activity->clazz,midGetPackageName);
    const char *nativePackageName = env->GetStringUTFChars(packageName, nullptr);
    ASensorManager* mgr = getInstanceForPackageFunc(nativePackageName);
    env->ReleaseStringUTFChars(packageName, nativePackageName);
    app->activity->vm->DetachCurrentThread();
    if (mgr) {
      dlclose(androidHandle);
      return mgr;
    }
  }


  typedef ASensorManager *(*PF_GETINSTANCE)();
  auto getInstanceFunc = (PF_GETINSTANCE)
      dlsym(androidHandle, "ASensorManager_getInstance");
  // by all means at this point, ASensorManager_getInstance should be available
  assert(getInstanceFunc);
  dlclose(androidHandle);


  return getInstanceFunc();
}
*/
void android_main(struct android_app* state) {
    struct engine engine{};
    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;
    engine.sensorManager = ASensorManager_getInstance();//AcquireASensorManagerInstance(state);
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,state->looper, LOOPER_ID_USER,nullptr, nullptr);
    if (state->savedState) {
        engine.state = *(saved_state*)state->savedState;
    }
    for (;;) {
        int ident;
        int events;
        struct android_poll_source* source;
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, nullptr, &events,(void**)&source)) >= 0) {
            if (source) {
                source->process(state, source);
            }
            if (ident == LOOPER_ID_USER) {
                if (engine.accelerometerSensor) {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue,&event, 1) > 0) {
                        LOGI("accelerometer: x=%f y=%f z=%f",event.acceleration.x, event.acceleration.y,event.acceleration.z);
                    }
                }
            }
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }
        if (engine.animating) 
        engine.state.angle += .01f;
        if (engine.state.angle > 1) {
            engine.state.angle = 0;
        }
        engine_draw_frame(&engine);
    		eglSwapBuffers(engine->display, engine->surface);
    }
}