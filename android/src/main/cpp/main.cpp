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
#include "translated_opengles.h"
#include "mainListener.h"

struct android_app {
    bool destroyed;
    
    int appCmdState;
    int msgread, msgwrite;
    
    size_t savedStateSize;
    
    ANativeActivity* activity;
    AConfiguration* config;
    void* savedState;
    ALooper* looper;
    
    ANativeWindow* window; //update in mainThread
    AInputQueue* inputQueue;
    
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t thread;
};
enum { 
    LOOPER_ACTIVITY = 1,
    LOOPER_INPUT = 2,
    LOOPER_SENSOR = 3,
};
enum {
    APP_CMD_START,
    APP_CMD_RESUME,
    APP_CMD_INPUT_INIT,
    APP_CMD_INIT_WINDOW,
    APP_CMD_GAINED_FOCUS,
    APP_CMD_WINDOW_RESIZED,
    APP_CMD_CONFIG_CHANGED,
    APP_CMD_LOST_FOCUS,
    APP_CMD_LOW_MEMORY,
    APP_CMD_SAVE_STATE,
    APP_CMD_TERM_WINDOW,
    APP_CMD_INPUT_TERM,
    APP_CMD_PAUSE,
    APP_CMD_STOP,
    APP_CMD_DESTROY,
};

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
struct touch_pointer {
	bool active;
	float xs, ys;
	float x, y;
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
    
    ANativeWindow* window; //used in glThread
    AInputQueue* inputQueue;
    
    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    EGLConfig eConfig;
    saved_state state;
    float accel[3];
    touch_pointer input_pointer_cache[20] = {0};
};

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
  if (!eng->window) return;
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
			eng->surface = eglCreateWindowSurface(eng->display, eng->eConfig, eng->window, nullptr);
  	}
  	eglMakeCurrent(eng->display, eng->surface, eng->surface, eng->context);
  	eglQuerySurface(eng->display, eng->surface, EGL_WIDTH, &eng->width);
  	eglQuerySurface(eng->display, eng->surface, EGL_HEIGHT, &eng->height);
  	
  	if (!eng->created) {
  		tgf = new tgf_gles();
  		eng->created = true;
  		Main::create(eng->width, eng->height);
  		eng->resume = false;
  		eng->resize = false;
  	} else if (newCtx) {
			((tgf_gles*)tgf)->validate();
  	}
  	if (eng->resize) {
  		eng->resize = false;
  		Main::resize(eng->width, eng->height);
  	}
  } else {
  	if (eng->resize) {
			eng->resize = false;
			eglMakeCurrent(eng->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglMakeCurrent(eng->display, eng->surface, eng->surface, eng->context);
			eglQuerySurface(eng->display, eng->surface, EGL_WIDTH, &eng->width);
			eglQuerySurface(eng->display, eng->surface, EGL_HEIGHT, &eng->height);
			Main::resize(eng->width, eng->height);
		}
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
static void process_cmd(const int8_t &cmd, android_app* app, engine *eng) {
		switch (cmd) {
	    case APP_CMD_RESUME:
				eng->resume = true;
	      eng->running = true;
        pthread_mutex_lock(&app->mutex);
			  if (app->savedState != NULL) {
		      free(app->savedState);
		      app->savedState = NULL;
		      app->savedStateSize = 0;
			  }
			  pthread_mutex_unlock(&app->mutex);
        break;
      case APP_CMD_INIT_WINDOW:
        eng->window = app->window;
        break;
	    case APP_CMD_WINDOW_RESIZED:
	    	eng->resize = true;
	    	break;
	    case APP_CMD_GAINED_FOCUS:
	      if (eng->accelerometerSensor) {
	        ASensorEventQueue_enableSensor(eng->sensorEventQueue,eng->accelerometerSensor);
	        ASensorEventQueue_setEventRate(eng->sensorEventQueue,eng->accelerometerSensor,(1000L/60)*1000);
	      }
	      break;
      case APP_CMD_INPUT_INIT:
        	eng->inputQueue = app->inputQueue;
          AInputQueue_attachLooper(eng->inputQueue, app->looper, LOOPER_INPUT, NULL, nullptr);
	      break;
      case APP_CMD_INPUT_TERM:
      	if (eng->inputQueue != NULL) {
        	AInputQueue_detachLooper(eng->inputQueue);
	        eng->inputQueue = NULL;
	        app->inputQueue = NULL;
      	}
        break;
	    case APP_CMD_LOST_FOCUS:
	      if (eng->accelerometerSensor) {
	        ASensorEventQueue_disableSensor(eng->sensorEventQueue,eng->accelerometerSensor);
	      }
	      break;
      case APP_CMD_TERM_WINDOW:
	  		engine_egl_terminate(eng, TERM_EGL_SURFACE);
        app->window = NULL;
        eng->window = NULL;
        break;
      case APP_CMD_CONFIG_CHANGED:
        AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
        break;
      case APP_CMD_SAVE_STATE:
        pthread_mutex_lock(&app->mutex);
			  if (app->savedState != NULL) {
		      free(app->savedState);
		      app->savedState = NULL;
		      app->savedStateSize = 0;
			  }
  			pthread_mutex_unlock(&app->mutex);
	      app->savedState = malloc(sizeof(saved_state));
	      *((saved_state*)app->savedState) = eng->state;
	      app->savedStateSize = sizeof(saved_state);
        break;
      case APP_CMD_PAUSE:
	  		eng->pause = true;
	  		engine_draw(app, eng);
	      eng->running = false;
	      break;
      case APP_CMD_DESTROY:
	  		eng->destroyed = true;
	  		engine_draw(app, eng);
	  		engine_egl_terminate(eng, TERM_EGL_DISPLAY);
        break;
      default:
      		break;
  	}
    pthread_mutex_lock(&app->mutex);
    app->appCmdState = cmd;
    pthread_cond_broadcast(&app->cond);
    pthread_mutex_unlock(&app->mutex);
}
static void* android_app_entry(void* param) {
    android_app *app = (android_app*)param;
    app->config = AConfiguration_new();
    AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
    app->looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(app->looper, app->msgread, LOOPER_ACTIVITY, ALOOPER_EVENT_INPUT, NULL, nullptr);
    
    engine *eng = new engine;
    memset(eng, 0, sizeof(engine));
    eng->sensorManager = ASensorManager_getInstance();
    eng->accelerometerSensor = ASensorManager_getDefaultSensor(eng->sensorManager,ASENSOR_TYPE_ACCELEROMETER);
    eng->sensorEventQueue = ASensorManager_createEventQueue(eng->sensorManager,app->looper, LOOPER_SENSOR , NULL, nullptr);
    if (app->savedState) {
        eng->state = *(saved_state*)app->savedState;
    }
		int8_t cmd;	
    int events;
    ASensorEvent s_event;
    //input env
    AInputEvent* i_event;
    int32_t motion_act;
    int8_t motion_ptr, motion_ptr_act;
    //end input env
    while (!eng->destroyed) {
      switch (ALooper_pollAll(eng->running ? 0 : -1, nullptr, &events, nullptr)) {
      	case LOOPER_ACTIVITY: //android activity queue
			    if (read(app->msgread, &cmd, sizeof(cmd)) == sizeof(cmd))
      			process_cmd(cmd, app, eng);
			    else
		        LOGI("No data on command pipe!");
      		break;
        case LOOPER_INPUT: //input queue
			    if (AInputQueue_getEvent(eng->inputQueue, &i_event) >= 0) {
		        if (!AInputQueue_preDispatchEvent(eng->inputQueue, i_event)) {
			        int32_t handled = 0;
			        switch (AInputEvent_getType(i_event)) {
			        	case AINPUT_EVENT_TYPE_MOTION:
			        		motion_act = AMotionEvent_getAction(i_event);
									motion_ptr = (motion_act&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
									motion_ptr_act = motion_act&AMOTION_EVENT_ACTION_MASK;
									touch_pointer &ip = eng->input_pointer_cache[motion_ptr];
									switch(motion_ptr_act) {
								    case AMOTION_EVENT_ACTION_DOWN:
								    	ip.active = true;
							        ip.xs = ip.x = AMotionEvent_getX(i_event, 0);
							        ip.ys = ip.y = AMotionEvent_getY(i_event, 0);
								    	break;
								    case AMOTION_EVENT_ACTION_MOVE:
							        ip.x = AMotionEvent_getX(i_event, 0);
							        ip.y = AMotionEvent_getY(i_event, 0);
								    	break;
								    case AMOTION_EVENT_ACTION_UP:
								    	ip.active = false;
								    	
								    	break;
								    case AMOTION_EVENT_ACTION_CANCEL:
								    	ip.active = false;
								    	break;
								    case AMOTION_EVENT_ACTION_OUTSIDE:
								    	ip.active = false;
								    	break;
								    case AMOTION_EVENT_ACTION_POINTER_DOWN:
								    	break;
								    case AMOTION_EVENT_ACTION_POINTER_UP:
								    	break;
								    case AMOTION_EVENT_ACTION_SCROLL:
								    	break;
								    case AMOTION_EVENT_ACTION_HOVER_ENTER:
								    	break;
								    case AMOTION_EVENT_ACTION_HOVER_MOVE:
								    	break;
								    case AMOTION_EVENT_ACTION_HOVER_EXIT:
								    	break;
									}
					        handled = 1;
					    }
			        AInputQueue_finishEvent(eng->inputQueue, i_event, handled);
		        }
			    } else {
			    	LOGI("Failure reading next input event: %s\n", strerror(errno));
			    }
        	break;
        case LOOPER_SENSOR: //sensor queue
			    assert(eng->accelerometerSensor);
				  while (ASensorEventQueue_getEvents(eng->sensorEventQueue,&s_event, 1) > 0) {
			  		eng->accel[0] = s_event.acceleration.x/2.f+ 0.5f;
			  		eng->accel[1] = s_event.acceleration.y/2.f + 0.5f;
			  		eng->accel[2] = s_event.acceleration.z/2.f + 0.5f;
				  }
        	break;
        default:
      		engine_draw(app, eng);
        	break;
      }
    }
    if (tgf) delete tgf;
    //no rendering anymore
    delete eng;
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
    app->destroyed = false;
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
    android_app_write_cmd(app, APP_CMD_SAVE_STATE);
    while (app->appCmdState != APP_CMD_SAVE_STATE) {
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
    while (app->appCmdState != APP_CMD_PAUSE) {
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
    android_app *app = (android_app*)activity->instance;
    const int8_t foc = focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS;
    pthread_mutex_lock(&app->mutex);
    android_app_write_cmd(app, foc);
    while (app->appCmdState != foc) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
    android_app *app = (android_app*)activity->instance;
    if (app->window != NULL) { //window should null when window create
      pthread_mutex_lock(&app->mutex);
	    android_app_write_cmd(app, APP_CMD_TERM_WINDOW);
	    while (app->appCmdState != APP_CMD_TERM_WINDOW) {
	        pthread_cond_wait(&app->cond, &app->mutex);
	    }
	    pthread_mutex_unlock(&app->mutex);
    }
    app->window = window;
    pthread_mutex_lock(&app->mutex);
    android_app_write_cmd(app, APP_CMD_INIT_WINDOW);
    while (app->appCmdState != APP_CMD_INIT_WINDOW) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onNativeWindowResized(ANativeActivity* activity, ANativeWindow* window) {
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_WINDOW_RESIZED);
}
static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
    android_app *app = (android_app*)activity->instance;
    if(app->window == NULL) return;
    pthread_mutex_lock(&app->mutex);
    android_app_write_cmd(app, APP_CMD_TERM_WINDOW);
    while (app->appCmdState != APP_CMD_TERM_WINDOW) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    android_app *app = (android_app*)activity->instance;
    if (app->inputQueue != NULL) {
	    pthread_mutex_lock(&app->mutex);
    	android_app_write_cmd(app, APP_CMD_INPUT_TERM);
	    while (app->appCmdState != APP_CMD_INPUT_TERM) {
        pthread_cond_wait(&app->cond, &app->mutex);
	    }
	    pthread_mutex_unlock(&app->mutex);
    }
    app->inputQueue = queue;
    pthread_mutex_lock(&app->mutex);
  	android_app_write_cmd(app, APP_CMD_INPUT_INIT);
    while (app->appCmdState != APP_CMD_INPUT_INIT) {
      pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
    android_app *app = (android_app*)activity->instance;
    if(app->inputQueue == NULL) return;
    pthread_mutex_lock(&app->mutex);
  	android_app_write_cmd(app, APP_CMD_INPUT_TERM);
    while (app->appCmdState != APP_CMD_INPUT_TERM) {
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
    activity->instance = app;
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
}
