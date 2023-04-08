#include <algorithm>
#include <initializer_list>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cassert>
#include <unistd.h>
#include <unordered_set>
#include <unordered_map>
#include <sys/resource.h>
#include <pthread.h>
#include <poll.h>
#include <sched.h>
#include <string>

#include <EGL/egl.h>
#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

#include "log.hpp"
#include "engine.hpp"
#include "main_game.hpp"

#include "android_input/android_input.hpp"
#include "android_asset/android_asset.hpp"
#include "api_graphics/android_graphics.hpp"
#include "api_graphics/opengles_graphics.hpp"
#include "api_graphics/vulkan_graphics.hpp"

android_asset *aasset;

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

static void* android_app_entry(void* param) {
    android_app *app = (android_app*)param;
    app->config = AConfiguration_new();
    AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
    app->looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(app->looper, app->msgread, 1, ALOOPER_EVENT_INPUT, NULL, nullptr);
    android_graphics *g  = new vulkan_graphics;
	  android_input *inpt = new android_input(app->looper);
	  if (app->savedState) {
	      g->state = *(saved_state*)app->savedState;
	  }
	  while (!g->destroyed) {
	    switch (ALooper_pollAll(g->running ? 0 : -1, nullptr, nullptr, nullptr)) {
	      case 2: //input queue
	      	inpt->process_input();
	      	break;
	      case 3: //sensor queue
	      	inpt->process_sensor();
	      	break;
	    	case 1: //android activity queue
					int8_t cmd;
			    if (read(app->msgread, &cmd, sizeof(cmd)) != sizeof(cmd)) break;
					switch (cmd) {
				    case APP_CMD_RESUME:
				    	g->onResume();
			        pthread_mutex_lock(&app->mutex);
						  if (app->savedState != NULL) {
					      free(app->savedState);
					      app->savedState = NULL;
					      app->savedStateSize = 0;
						  }
						  pthread_mutex_unlock(&app->mutex);
			        break;
			      case APP_CMD_INIT_WINDOW:
			      	g->onWindowInit(app->window);
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
			        break;
				    case APP_CMD_WINDOW_RESIZED:
				    	g->needResize();
				    	break;
				    case APP_CMD_GAINED_FOCUS:
				    	inpt->attach_sensor();
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
				      break;
			      case APP_CMD_INPUT_INIT:
		        	inpt->set_input_queue(app->looper, app->inputQueue);
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
				      break;
			      case APP_CMD_INPUT_TERM:
			      	if (app->inputQueue != NULL) {
			        	inpt->set_input_queue(app->looper, NULL);
				        app->inputQueue = NULL;
			      	}
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
			        break;
				    case APP_CMD_LOST_FOCUS:
				    	inpt->detach_sensor();
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
				      break;
			      case APP_CMD_TERM_WINDOW:
			      	g->onWindowTerm();
			        app->window = NULL;
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
				      *((saved_state*)app->savedState) = g->state;
				      app->savedStateSize = sizeof(saved_state);
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
			        break;
			      case APP_CMD_PAUSE:
			      	g->onPause();
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
				      break;
			      case APP_CMD_DESTROY:
				  		g->onDestroy();
			        break;
			      default:
			      	// ?
			      	break;
			  	}
			  	break;
			  default:
					g->render();
			  	break;
	    }
	  }
	  delete g;
	  delete inpt;
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
	  delete aasset;
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
	  aasset = new android_asset(activity->assetManager);
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




