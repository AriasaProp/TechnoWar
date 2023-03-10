#include <initializer_list>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cassert>
#include <unistd.h>
#include <unordered_set>
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
#include "engine.h"
#include "main_game.h"

namespace core_set {
	void define_core_set(ALooper *);
	//graphics Android
	void validate();
	void invalidate();
	//input Android
	void set_input_queue(AInputQueue *);
	void process_input();
	void attach_sensor();
	void process_sensor();
	void detach_sensor();
	
	void undefine_core_set();
}


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

#define TERM_EGL_SURFACE 1
#define TERM_EGL_CONTEXT 2
#define TERM_EGL_DISPLAY 4

struct m_egl {
  bool resize, resume, running, pause, destroyed;
  saved_state state;
  ANativeWindow* window; //used in glThread
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  EGLConfig eConfig;
	void egl_terminate(const unsigned int term) {
  	if (!term || !display) return;
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (context && (term & (TERM_EGL_CONTEXT|TERM_EGL_DISPLAY))) {
    	core_set::invalidate();
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
static Main *m_Main = nullptr;
static void engine_draw(m_egl *eng) {
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
    int32_t width, height;
  	eglQuerySurface(eng->display, eng->surface, EGL_WIDTH, &width);
  	eglQuerySurface(eng->display, eng->surface, EGL_HEIGHT, &height);
  	
  	if (!m_Main) {
  		m_Main = new Main;
  		m_Main->create();
  		eng->resume = false;
  	}
  	if (newCtx) {
			core_set::validate();
  	}
		setsresize_viewport(width, height);
		eng->resize = false;
  }
	if (eng->resize) {
		eng->resize = false;
		eglMakeCurrent(eng->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglMakeCurrent(eng->display, eng->surface, eng->surface, eng->context);
    int32_t width, height;
		eglQuerySurface(eng->display, eng->surface, EGL_WIDTH, &width);
		eglQuerySurface(eng->display, eng->surface, EGL_HEIGHT, &height);
		core_set::resize_viewport(width, height);
	}
  if (eng->resume) {
  	m_Main->resume();
		eng->resume = false;
  }
  eng->state.angle += .01f;
  if (eng->state.angle > 1) {
      eng->state.angle = 0;
  }
  m_Main->render(1.f/60.f);
  if (eng->pause) {
  	m_Main->pause();
		eng->pause = false;
  }
  if (eng->destroyed) {
  	m_Main->destroy();
  	delete(m_Main);
  	m_Main = nullptr;
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
    ALooper_addFd(app->looper, app->msgread, 1, ALOOPER_EVENT_INPUT, NULL, nullptr);
	  m_egl *eng = new m_egl;
	  memset(eng,0,sizeof(m_egl));
	  core_set::define_core_set(app->looper);
	  if (app->savedState) {
	      eng->state = *(saved_state*)app->savedState;
	  }
	  while (!eng->destroyed) {
	    switch (ALooper_pollAll(eng->running ? 0 : -1, nullptr, nullptr, nullptr)) {
	      case 2: //input queue
	      	core_set::process_input();
	      	break;
	      case 3: //sensor queue
	      	core_set::process_sensor();
	      	break;
	    	case 1: //android activity queue
					int8_t cmd;
			    if (read(app->msgread, &cmd, sizeof(cmd)) != sizeof(cmd)) break;
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
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
			        break;
				    case APP_CMD_WINDOW_RESIZED:
				    	eng->resize = true;
				    	break;
				    case APP_CMD_GAINED_FOCUS:
				    	core_set::attach_sensor();
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
				      break;
			      case APP_CMD_INPUT_INIT:
		        	core_set::set_input_queue(app->inputQueue);
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
				      break;
			      case APP_CMD_INPUT_TERM:
			      	if (app->inputQueue != NULL) {
			        	core_set::set_input_queue(NULL);
				        app->inputQueue = NULL;
			      	}
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
			        break;
				    case APP_CMD_LOST_FOCUS:
				    	core_set::detach_sensor();
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
				      break;
			      case APP_CMD_TERM_WINDOW:
				  		eng->egl_terminate(TERM_EGL_SURFACE);
			        app->window = eng->window = NULL;
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
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
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
			        break;
			      case APP_CMD_PAUSE:
				  		eng->pause = true;
				  		engine_draw(eng);
				      eng->running = false;
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
				      break;
			      case APP_CMD_DESTROY:
				  		eng->destroyed = true;
				  		engine_draw(eng);
				  		eng->egl_terminate(TERM_EGL_DISPLAY);
			        break;
			      default:
			      	// ?
			      	break;
			  	}
			  	break;
			  default:
					engine_draw(eng);
			  	break;
	    }
	  }
	  core_set::undefine_core_set();
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
static void onNativeWindowResized(ANativeActivity* activity, ANativeWindow*) {
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_WINDOW_RESIZED);
}
static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow*) {
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
static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue*) {
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

//define engine extern

#define MAX_TOUCH_POINTERS_COUNT 30

struct touch_pointer {
	bool active;
	float xs, ys;
	float x, y;
} *input_pointer_cache;
struct key_event {
	int keyCode;
	enum event {
		KEY_UP,
		KEY_DOWN
	} type;
};
std::unordered_set<key_event*> key_events;
float *m_accelerometer;
float *m_gyroscope;
std::unordered_set<int> key_pressed;
std::unordered_set<int> just_key_pressed;
ASensorEvent *s_event;
ASensorManager* sensorManager;
const ASensor* accelerometerSensor;
const ASensor* gyroscopeSensor;
ASensorEventQueue* sensorEventQueue;
ALooper *m_looper;

float	getAccelerometerX() { return m_accelerometer[0]; }
float getAccelerometerY() { return m_accelerometer[1]; }
float getAccelerometerZ() { return m_accelerometer[2];}
float getGyroscopeX() {	return m_gyroscope[0]; }
float getGyroscopeY() {	return m_gyroscope[1]; }
float getGyroscopeZ() {	return m_gyroscope[2]; }
int getX(unsigned int p = 0) {	return input_pointer_cache[p].x; }
int getDeltaX(unsigned int p = 0) {	return input_pointer_cache[p].x - input_pointer_cache[p].xs; }
int getY(unsigned int p = 0) {	return input_pointer_cache[p].y; }
int getDeltaY(unsigned int p = 0) {	return input_pointer_cache[p].y - input_pointer_cache[p].ys; }
bool justTouched() {	return false; }
bool isTouched(unsigned int p = 0) { return input_pointer_cache[p].active; }
float getPressure(unsigned int p = 0) {
	(void) p;
	return false;
}
bool isButtonPressed(int button) {
	(void) button;
	return false;
}
bool isButtonJustPressed(int button) {
	(void) button;
	return false;
}
bool isKeyPressed(int key) {return key_pressed.find(key) != key_pressed.end(); }
bool isKeyJustPressed(int key) {return just_key_pressed.find(key) != just_key_pressed.end(); }
void process_event() {
	if(just_key_pressed.size() > 0) {
		just_key_pressed.clear();
	}
	for (key_event *k : key_events) {
		switch(k->type) {
			case key_event::event::KEY_UP: {
			}
				break;
			case key_event::event::KEY_DOWN:{
				std::unordered_set<int>::iterator key = just_key_pressed.find(k->keyCode);
				if(key != just_key_pressed.end()) {
					just_key_pressed.insert(k->keyCode);
				}
			}
				break;
		}
		delete k;
	}
	key_events.clear();
}
AInputQueue *inputQueue = NULL;
void core_set::set_input_queue(AInputQueue *i) {
	if (inputQueue)
		AInputQueue_detachLooper(inputQueue);
	inputQueue = i;
	if (inputQueue)
		AInputQueue_attachLooper(inputQueue, m_looper, 2, NULL, nullptr);
}
AInputEvent* i_event;
void core_set::process_input() {
	if (inputQueue == NULL) return;
	if (AInputQueue_getEvent(inputQueue, &i_event) < 0) return;
  if (AInputQueue_preDispatchEvent(inputQueue, i_event) != 0) return;
  int32_t handled = 0;
	switch (AInputEvent_getType(i_event)) {
		case AINPUT_EVENT_TYPE_KEY: {
			int32_t keyCode = AKeyEvent_getKeyCode(i_event);
			switch (AKeyEvent_getAction(i_event)) {
				case AKEY_EVENT_ACTION_DOWN: {
					std::unordered_set<int>::iterator key = key_pressed.find(keyCode);
					if(key != key_pressed.end()) {
						key_pressed.insert(keyCode);
					}
					key_events.insert(new key_event{keyCode,key_event::event::KEY_DOWN});
				}
					break;
				case AKEY_EVENT_ACTION_UP: {
					std::unordered_set<int>::iterator key = key_pressed.find(keyCode);
					if(key != key_pressed.end()) {
						key_pressed.erase(key);
					}
					key_events.insert(new key_event{keyCode,key_event::event::KEY_UP});
				}
					break;
				case AKEY_EVENT_ACTION_MULTIPLE:
					break;
			}
		}
			break;
		case AINPUT_EVENT_TYPE_MOTION: {
			const int32_t motion = AMotionEvent_getAction(i_event);
			switch(motion&AMOTION_EVENT_ACTION_MASK) {
		    case AMOTION_EVENT_ACTION_POINTER_DOWN:
		    case AMOTION_EVENT_ACTION_DOWN:
					if (AMotionEvent_getEdgeFlags(i_event) != 0)
						break;
		    {
					const int8_t pointer_index = (motion&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
					if (pointer_index >= MAX_TOUCH_POINTERS_COUNT)
						break;
					touch_pointer &ip = input_pointer_cache[pointer_index];
		    	ip.active = true;
	        ip.xs = ip.x = AMotionEvent_getX(i_event, pointer_index);
	        ip.ys = ip.y = AMotionEvent_getY(i_event, pointer_index);
		    }
		    	break;
		    case AMOTION_EVENT_ACTION_MOVE:
		    	for (size_t i = 0, j = AMotionEvent_getPointerCount(i_event); (i<j) && (i < MAX_TOUCH_POINTERS_COUNT); i++) {
						touch_pointer &ip = input_pointer_cache[i];
						if (!ip.active) continue;
		        ip.x = AMotionEvent_getX(i_event, i);
		        ip.y = AMotionEvent_getY(i_event, i);
		    	}
		    	break;
		    case AMOTION_EVENT_ACTION_POINTER_UP:
		    case AMOTION_EVENT_ACTION_UP:
		    case AMOTION_EVENT_ACTION_OUTSIDE:
		    {
					const int8_t pointer_index = (motion&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
					if (pointer_index >= MAX_TOUCH_POINTERS_COUNT)
						break;
					touch_pointer &ip = input_pointer_cache[pointer_index];
					if (!ip.active) break;
		    	ip.active = false;
	        ip.x = AMotionEvent_getX(i_event, pointer_index);
	        ip.y = AMotionEvent_getY(i_event, pointer_index);
		    }
		    	break;
		    case AMOTION_EVENT_ACTION_CANCEL:
		    	memset(input_pointer_cache, 0, MAX_TOUCH_POINTERS_COUNT*sizeof(touch_pointer));
		    	break;
		    case AMOTION_EVENT_ACTION_SCROLL:
		    case AMOTION_EVENT_ACTION_HOVER_ENTER:
		    case AMOTION_EVENT_ACTION_HOVER_MOVE:
		    case AMOTION_EVENT_ACTION_HOVER_EXIT:
		    	break;
			}
	    handled = 1;
		}
	    break;
	}
  AInputQueue_finishEvent(inputQueue, i_event, handled);
}
bool sensor_enabled = false;
void core_set::attach_sensor() {
	if (sensor_enabled) return;
  ASensorEventQueue_enableSensor(sensorEventQueue,accelerometerSensor);
  ASensorEventQueue_setEventRate(sensorEventQueue,accelerometerSensor,50000/3);
  ASensorEventQueue_enableSensor(sensorEventQueue,gyroscopeSensor);
  ASensorEventQueue_setEventRate(sensorEventQueue,gyroscopeSensor,50000/3);
	sensor_enabled = true;
}
void core_set::process_sensor() {
	if (!sensor_enabled) return;
	unsigned int i, j;
	while ((i = ASensorEventQueue_getEvents(sensorEventQueue,s_event, 2)) > 0) {
		for (j = 0; j < i; j++) {
			ASensorEvent &e = s_event[j];
			switch (e.type) {
				case ASENSOR_TYPE_ACCELEROMETER:
					m_accelerometer[0] = e.acceleration.x/2.f + 0.5f;
					m_accelerometer[1] = e.acceleration.y/2.f + 0.5f;
					m_accelerometer[2] = e.acceleration.z/2.f + 0.5f;
					break;
				case ASENSOR_TYPE_GYROSCOPE:
					m_gyroscope[0] = e.acceleration.x/2.f + 0.5f;
					m_gyroscope[1] = e.acceleration.y/2.f + 0.5f;
					m_gyroscope[2] = e.acceleration.z/2.f + 0.5f;
					break;
				default:
					break;
			}
		}
	}
}
void core_set::detach_sensor() {
	if (!sensor_enabled) return;
	memset(m_accelerometer, 0, 3*sizeof(float));
	memset(m_gyroscope, 0, 3*sizeof(float));
  ASensorEventQueue_disableSensor(sensorEventQueue, accelerometerSensor);
  ASensorEventQueue_disableSensor(sensorEventQueue, gyroscopeSensor);
	sensor_enabled = false;
}

core_set::define_core_set(ALooper *looper) {
	//input
	m_looper = _looper;
	m_accelerometer = new float[3]{};
	m_gyroscope = new float[3]{};
	input_pointer_cache = new touch_pointer[MAX_TOUCH_POINTERS_COUNT]{};
	s_event = new ASensorEvent[2];
  sensorManager = ASensorManager_getInstance();
  accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_ACCELEROMETER);
  gyroscopeSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_GYROSCOPE);
	sensorEventQueue = ASensorManager_createEventQueue(sensorManager, _looper, 3 , NULL, nullptr);
	
  //graphics
  engine::getWidth = getWidth;
  engine::getHeight = getHeight;
  engine::clear = clear;
  engine::clearcolor = clearcolor;
  engine::gen_texture = gen_texture;
  engine::bind_texture = bind_texture;
  engine::set_texture_param = set_texture_param;
  engine::delete_texture = delete_texture;
  engine::flat_render = flat_render;
  engine::gen_mesh = gen_mesh;
  engine::mesh_render = mesh_render;
  engine::delete_mesh = delete_mesh;
  
  //input
  engine::getAccelerometerX = getAccelerometerX;
  engine::getAccelerometerY = getAccelerometerY;
  engine::getAccelerometerZ = getAccelerometerZ;
  engine::getGyroscopeX = getGyroscopeX;
  engine::getGyroscopeY = getGyroscopeY;
  engine::getGyroscopeZ = getGyroscopeZ;
  engine::getX = getX;
  engine::getDeltaX = getDeltaX;
  engine::getY = getY;
  engine::getDeltaY = getDeltaY;
  engine::justTouched = justTouched;
  engine::isTouched = isTouched;
  engine::getPressure = getPressure;
  engine::isButtonPressed = isButtonPressed;
  engine::isButtonJustPressed = isButtonJustPressed;
  engine::isKeyPressed = isKeyPressed;
  engine::isKeyJustPressed = isKeyJustPressed;
  engine::process_event = process_event;

}
core_set::undefine_core_set() {
	//input
	detach_sensor();
	set_input_queue(NULL);
	delete[] m_accelerometer;
	delete[] m_gyroscope;
	delete[] input_pointer_cache;
	delete[] s_event;
	key_pressed.clear();
	
	//unused engine anymore
	
}






