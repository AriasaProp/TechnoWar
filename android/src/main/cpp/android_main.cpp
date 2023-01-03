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
#include "android_input.h"
#include "translated_opengles.h"
#include "main_game.h"

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
struct engine {
    saved_state state;
    bool created;
    bool resize;
    bool resume;
    bool running;
    bool pause;
    bool destroyed;
    int32_t width;
    int32_t height;
    
    ANativeWindow* window; //used in glThread
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    EGLConfig eConfig;
		void egl_terminate(const unsigned int term) {
		  	if (!term || !display) return;
				eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
				if (context && (term & (TERM_EGL_CONTEXT|TERM_EGL_DISPLAY))) {
		    	if (tgf) {
		    		((tgf_gles*)tgf)->invalidate();
		    	}
		    	eglDestroyContext(display, context);
		    	context = EGL_NO_CONTEXT;
		    }
		    if (surface && (term & (TERM_EGL_SURFACE|TERM_EGL_DISPLAY))) {
		      eglDestroySurface(display, surface);
		    	surface = EGL_NO_SURFACE;
		    }
		    if (term & TERM_EGL_DISPLAY) {
		  		eglTerminate(display);
		    	display = EGL_NO_DISPLAY;
		    }
		}
};
static void engine_draw(android_app *app, engine *eng) {
  if (!eng->window || !eng->running) return;
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
  	
  	if (!tgf) {
  		tgf = new tgf_gles();
  	}
  	
  	if (!eng->created) {
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
  }
	if (eng->resize) {
		eng->resize = false;
		eglMakeCurrent(eng->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglMakeCurrent(eng->display, eng->surface, eng->surface, eng->context);
		eglQuerySurface(eng->display, eng->surface, EGL_WIDTH, &eng->width);
		eglQuerySurface(eng->display, eng->surface, EGL_HEIGHT, &eng->height);
		Main::resize(eng->width, eng->height);
	}
  if (eng->resume) {
  	Main::resume();
		eng->resume = false;
  }
  //if (!eng->running) return;
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
  			eng->egl_terminate(TERM_EGL_SURFACE);
  			break;
  		case EGL_BAD_CONTEXT:
  		case EGL_CONTEXT_LOST:
  			eng->egl_terminate(TERM_EGL_CONTEXT);
  			break;
  		case EGL_NOT_INITIALIZED:
  		case EGL_BAD_DISPLAY:
  			eng->egl_terminate(TERM_EGL_DISPLAY);
  			break;
  		default:
				break;
		}
	}
}
static void* android_app_entry(void* param) {
    android_app *app = (android_app*)param;
    app->config = AConfiguration_new();
    AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
    app->looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(app->looper, app->msgread, LOOPER_ACTIVITY, ALOOPER_EVENT_INPUT, NULL, nullptr);
    
    engine *eng = new engine;
    android_input *m_input = new android_input;
    
    memset(eng, 0, sizeof(engine));
    m_input->sensorEventQueue = ASensorManager_createEventQueue(m_input->sensorManager,app->looper, LOOPER_SENSOR , NULL, nullptr);
    if (app->savedState) {
        eng->state = *(saved_state*)app->savedState;
    }
		int8_t cmd;	
    int events;
    //end input env
    while (!eng->destroyed) {
      switch (ALooper_pollAll(eng->running ? 0 : -1, nullptr, &events, nullptr)) {
      	case LOOPER_ACTIVITY: //android activity queue
			    if (read(app->msgread, &cmd, sizeof(cmd)) != sizeof(cmd)) {
		        LOGI("No data on command pipe!");
			    } else {
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
					    	m_input->attach_sensor();
					      break;
				      case APP_CMD_INPUT_INIT:
				          AInputQueue_attachLooper(app->inputQueue, app->looper, LOOPER_INPUT, NULL, nullptr);
				        	m_input->set_input_queue(app->inputQueue);
					      break;
				      case APP_CMD_INPUT_TERM:
				      	if (app->inputQueue != NULL) {
				        	m_input->set_input_queue(NULL);
				        	AInputQueue_detachLooper(app->inputQueue);
					        app->inputQueue = NULL;
				      	}
				        break;
					    case APP_CMD_LOST_FOCUS:
					    	m_input->detach_sensor();
					      break;
				      case APP_CMD_TERM_WINDOW:
					  		eng->egl_terminate(TERM_EGL_SURFACE);
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
					  		eng->egl_terminate(TERM_EGL_DISPLAY);
				        break;
				      default:
				      	break;
				  	}
				    pthread_mutex_lock(&app->mutex);
				    app->appCmdState = cmd;
				    pthread_cond_broadcast(&app->cond);
				    pthread_mutex_unlock(&app->mutex);
			    }
      		break;
        case LOOPER_INPUT: //input queue
        	m_input->input_sensor();
        	break;
        case LOOPER_SENSOR: //sensor queue
        	m_input->process_sensor();
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
        app->inputQueue = NULL;
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
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_CONFIG_CHANGED);
}
static void onLowMemory(ANativeActivity* activity) {
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_LOW_MEMORY);
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
