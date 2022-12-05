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

#include "mainListener.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};
enum {
	TERM_EGL_SURFACE = 1,
	TERM_EGL_CONTEXT = 2,
	TERM_EGL_DISPLAY = 4
};
struct engine {
    bool created;
    bool resize;
    bool resume;
    bool running;
    bool pause;
    bool destroyed;
    int32_t width;
    int32_t height;
    android_app* app;
    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    EGLConfig eConfig;
    saved_state state;
    float accel[3];
};
static int32_t poll_input(android_app* app, AInputEvent* event) {
    engine* eng = (engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        eng->state.x = AMotionEvent_getX(event, 0);
        eng->state.y = AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

static void engine_egl_terminate(engine *eng, const unsigned int term) {
  if (!term) return;
	if (eng->display) {
		eglMakeCurrent(eng->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (eng->context && (term & (TERM_EGL_CONTEXT|TERM_EGL_DISPLAY))) {
    	eglDestroyContext(eng->display, eng->context);
    	eng->context = EGL_NO_CONTEXT;
    }
    if (eng->surface && (term & (TERM_EGL_SURFACE|TERM_EGL_DISPLAY))) {
      eglDestroySurface(eng->display, eng->surface);
    	eng->surface = EGL_NO_SURFACE;
    }
    if (term & TERM_EGL_DISPLAY) {
  		eglTerminate(eng->display);
    	eng->display = EGL_NO_DISPLAY;
    }
	}
}
static void engine_draw(android_app *app, engine *eng) {
  if (!app->window) return;
  if (!eng->display || !eng->context || !eng->surface) {
  	if (!eng->display) {
    	eng->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    	eglInitialize(eng->display, nullptr, nullptr);
    	eng->eConfig = nullptr;
  	}
  	if (!eng->eConfig) {
		  EGLint temp;
		  const EGLint configAttr[] = {
		    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		    EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
		    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		    EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
		    EGL_ALPHA_SIZE, 0,
		    EGL_NONE
		  };
		  eglChooseConfig(eng->display, configAttr, nullptr,0, &temp);
		  assert(temp);
		  EGLConfig *configs = (EGLConfig*) alloca(temp*sizeof(EGLConfig));
		  assert(configs);
		  eglChooseConfig(eng->display, configAttr, configs, temp, &temp);
		  assert(temp);
		  eng->eConfig = configs[0];
		  for (unsigned int i = 0, j = temp, k = 0, l; i < j; i++) {
		    EGLConfig& cfg = configs[i];
		    eglGetConfigAttrib(eng->display, cfg, EGL_BUFFER_SIZE, &temp);
		    l = temp;
		    eglGetConfigAttrib(eng->display, cfg, EGL_DEPTH_SIZE, &temp);
		    l += temp;
		    eglGetConfigAttrib(eng->display, cfg, EGL_STENCIL_SIZE, &temp);
		    l += temp;
		    if (l > k) {
		      k = l;
		      eng->eConfig = cfg;
		    }
		  }
  	}
  	if (!eng->context) {
  		const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  		eng->context = eglCreateContext(eng->display, eng->eConfig, nullptr, ctxAttr);
  	}
  	if (!eng->surface) {
			eng->surface = eglCreateWindowSurface(eng->display, eng->eConfig, app->window, nullptr);
  	}
  	eglMakeCurrent(eng->display, eng->surface, eng->surface, eng->context);
  	eglQuerySurface(eng->display, eng->surface, EGL_WIDTH, &eng->width);
  	eglQuerySurface(eng->display, eng->surface, EGL_HEIGHT, &eng->height);
  	
  	if (!eng->created) {
  		eng->created = true;
  		Main::create(eng->width, eng->height);
  		eng->resume = false;
  		eng->resize = false;
  	}
  	if (eng->resize) {
  		eng->resize = false;
  		Main::resize(eng->width, eng->height);
  	}
  }
  if (eng->resize) {
  	eglMakeCurrent(eng->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  	eglMakeCurrent(eng->display, eng->surface, eng->surface, eng->context);
  	eglQuerySurface(eng->display, eng->surface, EGL_WIDTH, &eng->width);
  	eglQuerySurface(eng->display, eng->surface, EGL_HEIGHT, &eng->height);
  	eng->resize = false;
		Main::resize(eng->width, eng->height);
  }
  if (eng->resume) {
  	Main::resume();
		eng->resume = false;
  }
  if (!eng->running) return;
  eng->state.angle += .01f;
  if (eng->state.angle > 1) {
      eng->state.angle = 0;
  }
  Main::render(1.f/60.f);
  if (eng->pause) {
  	Main::pause();
		eng->pause = false;
  }
  if (eng->destroyed) {
  	Main::destroy();
		eng->created = false;
		eng->destroyed = false;
  }
	if (!eglSwapBuffers(eng->display, eng->surface)) {
		switch (eglGetError()) {
  		case EGL_BAD_SURFACE:
  		case EGL_BAD_NATIVE_WINDOW:
  		case EGL_BAD_CURRENT_SURFACE:
  			engine_egl_terminate(eng, TERM_EGL_SURFACE);
  			break;
  		case EGL_BAD_CONTEXT:
  		case EGL_CONTEXT_LOST:
  			engine_egl_terminate(eng, TERM_EGL_CONTEXT);
  			break;
  		case EGL_NOT_INITIALIZED:
  		case EGL_BAD_DISPLAY:
  			engine_egl_terminate(eng, TERM_EGL_DISPLAY);
  			break;
  		default:
				break;
		}
	}
}
static void poll_cmd(android_app* app, int32_t cmd) {
  engine* eng = (engine*)app->userData;
	switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      break;
    case APP_CMD_RESUME:
  		if (eng->created)
				eng->resume = true;
      eng->running = true;
      break;
    case APP_CMD_GAINED_FOCUS:
      if (eng->accelerometerSensor) {
        ASensorEventQueue_enableSensor(eng->sensorEventQueue,eng->accelerometerSensor);
        ASensorEventQueue_setEventRate(eng->sensorEventQueue,eng->accelerometerSensor,(1000L/60)*1000);
      }
      break;
    case APP_CMD_WINDOW_RESIZED:
    	eng->resize = true;
    	break;
    case APP_CMD_SAVE_STATE:
      app->savedState = malloc(sizeof(saved_state));
      *((saved_state*)app->savedState) = eng->state;
      app->savedStateSize = sizeof(saved_state);
      break;
    case APP_CMD_LOST_FOCUS:
      if (eng->accelerometerSensor) {
        ASensorEventQueue_disableSensor(eng->sensorEventQueue,eng->accelerometerSensor);
      }
      break;
    case APP_CMD_PAUSE:
  		eng->pause = true;
  		engine_draw(app, eng);
      eng->running = false;
      break;
    case APP_CMD_TERM_WINDOW:
  		engine_egl_terminate(eng, TERM_EGL_SURFACE);
      break;
    case APP_CMD_DESTROY:
  		eng->destroyed = true;
  		engine_draw(app, eng);
  		engine_egl_terminate(eng, TERM_EGL_DISPLAY);
      break;
    default:
      break;
	}
}
static void sensor_process(android_app* app, android_poll_source* src) {
    engine* eng = (engine*)app->userData;
		if (eng->accelerometerSensor) {
	      ASensorEvent event;
	      while (ASensorEventQueue_getEvents(eng->sensorEventQueue,&event, 1) > 0) {
	      		eng->accel[0] = event.acceleration.x/2.f+ 0.5f;
	      		eng->accel[1] = event.acceleration.y/2.f + 0.5f;
	      		eng->accel[2] = event.acceleration.z/2.f + 0.5f;
	      }
	  }
}
void android_main(android_app* app) {
    engine eng;
    memset(&eng, 0, sizeof(eng));
    app->userData = &eng;
    app->onAppCmd = poll_cmd;
    app->onInputEvent = poll_input;
    eng.app = app;
    eng.sensorManager = ASensorManager_getInstance();
    eng.accelerometerSensor = ASensorManager_getDefaultSensor(eng.sensorManager,ASENSOR_TYPE_ACCELEROMETER);
    //input sensor event on loop
    android_poll_source snsr;
    snsr.id = LOOPER_ID_USER;
    snsr.app = app;
    snsr.process = sensor_process;
    eng.sensorEventQueue = ASensorManager_createEventQueue(eng.sensorManager,app->looper, snsr.id ,nullptr, &snsr);
    if (app->savedState) {
        eng.state = *(saved_state*)app->savedState;
    }
    int ident;
    int events;
    while (!app->destroyRequested) {
    	android_poll_source* source;
      if ((ident=ALooper_pollAll(eng.running ? 0 : -1, nullptr, &events,(void**)&source)) >= 0)
        if (source) source->process(app, source);
      else
      	engine_draw(app, &eng);
    }
}