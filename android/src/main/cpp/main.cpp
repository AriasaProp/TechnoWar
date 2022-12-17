#include <initializer_list>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cassert>
#include <unistd.h>
#include <sys/resource.h>
#include <pthread.h>
#include <poll.h>
#include <sched.h>

#include <EGL/egl.h>

#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

#include "log.h"


struct android_app {
    bool destroyRequested;
    bool running;
    bool stateSaved;
    bool destroyed;
    bool redrawNeeded;
    
    int activityState;
    int msgread, msgwrite;
    
    size_t savedStateSize;
    
    void* userData;
    void (*onAppCmd)(android_app*, int32_t);
    int32_t (*onInputEvent)(android_app*, AInputEvent*);
    ANativeActivity* activity;
    AConfiguration* config;
    void* savedState;
    ALooper* looper;
    AInputQueue* inputQueue;
    ANativeWindow* window;
    ARect contentRect;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t thread;
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
    APP_CMD_START,
    APP_CMD_RESUME,
    APP_CMD_INPUT_CHANGED,
    APP_CMD_INIT_WINDOW,
    APP_CMD_GAINED_FOCUS,
    APP_CMD_WINDOW_RESIZED,
    APP_CMD_WINDOW_REDRAW_NEEDED,
    APP_CMD_CONTENT_RECT_CHANGED,
    APP_CMD_CONFIG_CHANGED,
    APP_CMD_LOST_FOCUS,
    APP_CMD_LOW_MEMORY,
    APP_CMD_SAVE_STATE,
    APP_CMD_TERM_WINDOW,
    APP_CMD_PAUSE,
    APP_CMD_STOP,
    APP_CMD_DESTROY,
};
void process_input(android_app*);
void process_cmd(android_app*);
void process_sensor(android_app*);
void android_main(android_app*);

#include "translated_opengles.h"
#include "mainListener.h"

static TranslatedGraphicsFunction *tgf;

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
    	if (tgf) {
    		((tgf_gles*)tgf)->invalidate();
    	}
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
  	bool newCtx = false;
  	if (!eng->context) {
  		newCtx = true;
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
  		tgf = new tgf_gles;
  		eng->created = true;
  		Main::create(tgf, eng->width, eng->height);
  		eng->resume = false;
  		eng->resize = false;
  	} else if (newCtx) {
			((tgf_gles*)tgf)->validate();
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
void process_sensor(android_app* app) {
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
    memset(&eng, 0, sizeof(engine));
    app->userData = &eng;
    app->onAppCmd = poll_cmd;
    app->onInputEvent = poll_input;
    eng.sensorManager = ASensorManager_getInstance();
    eng.accelerometerSensor = ASensorManager_getDefaultSensor(eng.sensorManager,ASENSOR_TYPE_ACCELEROMETER);
    eng.sensorEventQueue = ASensorManager_createEventQueue(eng.sensorManager,app->looper, LOOPER_ID_USER , NULL, nullptr);
    if (app->savedState) {
        eng.state = *(saved_state*)app->savedState;
    }
    int events;
    while (!app->destroyRequested) {
      switch (ALooper_pollAll(eng.running ? 0 : -1, nullptr, &events, nullptr)) {
      	case LOOPER_ID_MAIN:
      		process_cmd(app);
      		break;
        case LOOPER_ID_INPUT:
      		process_input(app);
        	break;
        case LOOPER_ID_USER:
      		process_sensor(app);
        	break;
        default:
      		engine_draw(app, &eng);
        	break;
      }
    }
    if (tgf) delete tgf;
}

void process_input(android_app *app) {
    AInputEvent* event = NULL;
    if (AInputQueue_getEvent(app->inputQueue, &event) >= 0) {
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
void process_cmd(android_app* app) {
    int8_t cmd;
    if (read(app->msgread, &cmd, sizeof(cmd)) != sizeof(cmd)) {
        LOGI("No data on command pipe!");
    		return;
    }
		switch (cmd) {
      case APP_CMD_START:
          break;
      case APP_CMD_RESUME:
          break;
      case APP_CMD_INIT_WINDOW:
          pthread_mutex_lock(&app->mutex);
          app->window = app->pendingWindow;
          pthread_cond_broadcast(&app->cond);
          pthread_mutex_unlock(&app->mutex);
          break;
      case APP_CMD_INPUT_CHANGED:
          pthread_mutex_lock(&app->mutex);
          if (app->inputQueue != NULL) {
              AInputQueue_detachLooper(app->inputQueue);
          }
          app->inputQueue = app->pendingInputQueue;
          if (app->inputQueue != NULL) {
              AInputQueue_attachLooper(app->inputQueue, app->looper, LOOPER_ID_INPUT, NULL, nullptr);
          }
          pthread_cond_broadcast(&app->cond);
          pthread_mutex_unlock(&app->mutex);
          break;
      case APP_CMD_TERM_WINDOW:
          pthread_mutex_lock(&app->mutex);
          app->window = NULL;
          pthread_cond_broadcast(&app->cond);
          pthread_mutex_unlock(&app->mutex);
          break;
      case APP_CMD_PAUSE:
          pthread_mutex_lock(&app->mutex);
          app->activityState = cmd;
          pthread_cond_broadcast(&app->cond);
          pthread_mutex_unlock(&app->mutex);
          break;
      case APP_CMD_STOP:
          break;
      case APP_CMD_SAVE_STATE:
          pthread_mutex_lock(&app->mutex);
				  if (app->savedState != NULL) {
			      free(app->savedState);
			      app->savedState = NULL;
			      app->savedStateSize = 0;
				  }
    			pthread_mutex_unlock(&app->mutex);
          break;
      case APP_CMD_CONFIG_CHANGED:
          AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
          break;
      case APP_CMD_DESTROY:
          app->destroyRequested = true;
          break;
      default:
      		break;
  	}
    if (app->onAppCmd != NULL) app->onAppCmd(app, cmd);
  	switch (cmd) {
      case APP_CMD_TERM_WINDOW:
          pthread_mutex_lock(&app->mutex);
          app->window = NULL;
          pthread_cond_broadcast(&app->cond);
          pthread_mutex_unlock(&app->mutex);
          break;
      case APP_CMD_SAVE_STATE:
          pthread_mutex_lock(&app->mutex);
          app->stateSaved = true;
          pthread_cond_broadcast(&app->cond);
          pthread_mutex_unlock(&app->mutex);
          break;
      case APP_CMD_RESUME:
          pthread_mutex_lock(&app->mutex);
				  if (app->savedState != NULL) {
			      free(app->savedState);
			      app->savedState = NULL;
			      app->savedStateSize = 0;
				  }
				  pthread_mutex_unlock(&app->mutex);
          break;
      default:
      		break;
  	}
}
static void* android_app_entry(void* param) {
    android_app *app = (android_app*)param;
    app->config = AConfiguration_new();
    AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
    app->looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(app->looper, app->msgread, LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL, nullptr);
    pthread_mutex_lock(&app->mutex);
    app->running = true;
    pthread_cond_broadcast(&app->cond);
    pthread_mutex_unlock(&app->mutex);
    android_main(app);
    //destroy
    pthread_mutex_lock(&app->mutex);
    if (app->savedState != NULL) {
      free(app->savedState);
      app->savedState = NULL;
      app->savedStateSize = 0;
    }
    if (app->inputQueue != NULL) {
        AInputQueue_detachLooper(app->inputQueue);
    }
    AConfiguration_delete(app->config);
    app->destroyed = true;
    pthread_cond_broadcast(&app->cond);
    pthread_mutex_unlock(&app->mutex);
    return NULL;
}
static void android_app_write_cmd(android_app *app, int8_t cmd) {
    if (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd)) {
        LOGI("Failure writing android_app cmd: %s\n", strerror(errno));
    }
}
static void onDestroy(ANativeActivity* activity) {
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
static void onStart(ANativeActivity* activity) {
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_START);
}
static void onResume(ANativeActivity* activity) {
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_RESUME);
}
static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
    android_app *app = (android_app*)activity->instance;
    void* savedState = NULL;
    pthread_mutex_lock(&app->mutex);
    app->stateSaved = false;
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
    return savedState;
}
static void onPause(ANativeActivity* activity) {
    android_app *app = (android_app*)activity->instance;
    pthread_mutex_lock(&app->mutex);
    android_app_write_cmd(app, APP_CMD_PAUSE);
    while (app->activityState != APP_CMD_PAUSE) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onStop(ANativeActivity* activity) {
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_STOP);
}
static void onConfigurationChanged(ANativeActivity* activity) {
    android_app *app = (android_app*)activity->instance;
    android_app_write_cmd(app, APP_CMD_CONFIG_CHANGED);
}
static void onLowMemory(ANativeActivity* activity) {
    android_app *app = (android_app*)activity->instance;
    android_app_write_cmd(app, APP_CMD_LOW_MEMORY);
}
static void onWindowFocusChanged(ANativeActivity* activity, int focused) {
    android_app_write_cmd((android_app*)activity->instance,focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}
static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
    android_app *app = (android_app*)activity->instance;
    pthread_mutex_lock(&app->mutex);
    if (app->pendingWindow != NULL) {
        android_app_write_cmd(app, APP_CMD_TERM_WINDOW);
    }
    app->pendingWindow = window;
    android_app_write_cmd(app, APP_CMD_INIT_WINDOW);
    while (app->window != app->pendingWindow) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onNativeWindowResized(ANativeActivity* activity, ANativeWindow* window) {
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_WINDOW_RESIZED);
}
static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
    android_app *app = (android_app*)activity->instance;
    pthread_mutex_lock(&app->mutex);
    if (app->pendingWindow != NULL) {
        android_app_write_cmd(app, APP_CMD_TERM_WINDOW);
    }
    app->pendingWindow = NULL;
    while (app->window != app->pendingWindow) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    android_app *app = (android_app*)activity->instance;
    pthread_mutex_lock(&app->mutex);
    app->pendingInputQueue = queue;
    android_app_write_cmd(app, APP_CMD_INPUT_CHANGED);
    while (app->inputQueue != app->pendingInputQueue) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
    android_app *app = (android_app*)activity->instance;
    pthread_mutex_lock(&app->mutex);
    app->pendingInputQueue = NULL;
    android_app_write_cmd(app, APP_CMD_INPUT_CHANGED);
    while (app->inputQueue != app->pendingInputQueue) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
    activity->callbacks->onNativeWindowResized = onNativeWindowResized;
    activity->callbacks->onConfigurationChanged = onConfigurationChanged;
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onLowMemory = onLowMemory;
    activity->callbacks->onSaveInstanceState = onSaveInstanceState;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onDestroy = onDestroy;
    
    android_app* app = new android_app;
    memset(app, 0, sizeof(android_app));
    app->activity = activity;
    pthread_mutex_init(&app->mutex, NULL);
    pthread_cond_init(&app->cond, NULL);
    if (savedState != NULL) {
        app->savedState = malloc(savedStateSize);
        app->savedStateSize = savedStateSize;
        memcpy(app->savedState, savedState, savedStateSize);
    }
    if (pipe(&app->msgread)) {
        LOGI("could not create pipe: %s", strerror(errno));
    }
    pthread_attr_t attr; 
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&app->thread, &attr, android_app_entry, app);
    pthread_attr_destroy(&attr);
    pthread_mutex_lock(&app->mutex);
    while (!app->running) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
    activity->instance = app;
}
