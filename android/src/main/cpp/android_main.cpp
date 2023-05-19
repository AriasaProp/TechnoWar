#include <algorithm>
#include <initializer_list>
#include <memory>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/resource.h>
#include <pthread.h>
#include <poll.h>
#include <sched.h>
#include <string>

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
//#include "api_graphics/vulkan_graphics.hpp"

struct android_app {
    bool destroyed;
    int appCmdState;
    int msgread, msgwrite;
    ANativeActivity* activity;
    AConfiguration* config;
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
    APP_CMD_CONTENT_RECT_CHANGED,
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
static android_graphics *a_graphics;
#include <cstdio>
static void* android_app_entry(void* param) {
    android_app *app = (android_app*)param;
    app->config = AConfiguration_new();
    AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
    app->looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(app->looper, app->msgread, 1, ALOOPER_EVENT_INPUT, NULL, nullptr);
    android_input *a_input = new android_input(app->looper);
    a_graphics = new opengles_graphics{};
	  android_asset *a_asset = new android_asset(app->activity->assetManager);
    char cmd;
	  while (a_graphics) {
	    switch (ALooper_pollAll(a_graphics->running ? 0 : -1, nullptr, nullptr, nullptr)) {
	      case 2: //input queue
	      	a_input->process_input();
	      	break;
	      case 3: //sensor queue
	      	a_input->process_sensor();
	      	break;
	    	case 1: //android activity queue
			    if (read(app->msgread, &cmd, sizeof(cmd)) != sizeof(cmd)) break;
					switch (cmd) {
				    case APP_CMD_START:
			        break;
				    case APP_CMD_RESUME:
				    	a_graphics->onResume();
			        break;
			      case APP_CMD_INIT_WINDOW:
			      	a_graphics->onWindowInit(app->window);
			        break;
				    case APP_CMD_CONTENT_RECT_CHANGED:
				    	break;
				    case APP_CMD_WINDOW_RESIZED:
				    	a_graphics->needResize();
				    	break;
				    case APP_CMD_GAINED_FOCUS:
				    	a_input->attach_sensor();
				      break;
			      case APP_CMD_INPUT_INIT:
		        	a_input->set_input_queue(app->looper, app->inputQueue);
				      break;
			      case APP_CMD_INPUT_TERM:
			      	if (app->inputQueue != NULL) {
			        	a_input->set_input_queue(app->looper, NULL);
				        app->inputQueue = NULL;
			      	}
			        break;
				    case APP_CMD_LOST_FOCUS:
				    	a_input->detach_sensor();
				      break;
			      case APP_CMD_TERM_WINDOW:
			      	a_graphics->onWindowTerm();
			        app->window = NULL;
			        break;
			      case APP_CMD_CONFIG_CHANGED:
			        AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
			        break;
			      case APP_CMD_SAVE_STATE:
			        break;
			      case APP_CMD_PAUSE:
			      	a_graphics->onPause();
				      break;
			      case APP_CMD_STOP:
				      break;
			      case APP_CMD_DESTROY:
              delete a_graphics;
          	  a_graphics = nullptr;
			        break;
			      default:
			      	// ?
			      	break;
			  	}
			    pthread_mutex_lock(&app->mutex);
			    app->appCmdState = cmd;
			    pthread_cond_broadcast(&app->cond);
			    pthread_mutex_unlock(&app->mutex);
			  	break;
			  default:
					a_graphics->render();
			  	break;
	    }
	  }
	  delete a_input;
	  delete a_asset;
	  a_input = nullptr;
	  a_asset = nullptr;
    pthread_mutex_lock(&app->mutex);
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
static void onStart(ANativeActivity* activity) {
    android_app *app = (android_app*)activity->instance;
    const char cmd = APP_CMD_START;
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
}
static void onResume(ANativeActivity* activity) {
    android_app *app = (android_app*)activity->instance;
    const char cmd = APP_CMD_RESUME;
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
}
static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
    android_app *app = (android_app*)activity->instance;
    char cmd;
    if (app->window != NULL) { //window should null when window create
      pthread_mutex_lock(&app->mutex);
      cmd = APP_CMD_TERM_WINDOW;
      while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
        LOGE("cannot write on pipe , %s", strerror(errno));
	    while (app->appCmdState != cmd) {
	        pthread_cond_wait(&app->cond, &app->mutex);
	    }
	    pthread_mutex_unlock(&app->mutex);
    }
    app->window = window;
    cmd = APP_CMD_INIT_WINDOW;
    pthread_mutex_lock(&app->mutex);
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
    while (app->appCmdState != cmd) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    android_app *app = (android_app*)activity->instance;
    char cmd;
    if (app->inputQueue != NULL) {
	    pthread_mutex_lock(&app->mutex);
      cmd = APP_CMD_INPUT_TERM;
      while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
        LOGE("cannot write on pipe , %s", strerror(errno));
	    while (app->appCmdState != APP_CMD_INPUT_TERM) {
        pthread_cond_wait(&app->cond, &app->mutex);
	    }
	    pthread_mutex_unlock(&app->mutex);
    }
    app->inputQueue = queue;
    pthread_mutex_lock(&app->mutex);
    cmd = APP_CMD_INPUT_INIT;
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
    while (app->appCmdState != APP_CMD_INPUT_INIT) {
      pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onConfigurationChanged(ANativeActivity* activity) {
    android_app *app = (android_app*)activity->instance;
    const char cmd = APP_CMD_CONFIG_CHANGED;
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
}
static void onLowMemory(ANativeActivity* activity) {
    android_app *app = (android_app*)activity->instance;
    const char cmd = APP_CMD_LOW_MEMORY;
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
}
static void onWindowFocusChanged(ANativeActivity* activity, int focused) {
    android_app *app = (android_app*)activity->instance;
    const char cmd = focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS;
    pthread_mutex_lock(&app->mutex);
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
    while (app->appCmdState != cmd) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onNativeWindowResized(ANativeActivity* activity, ANativeWindow*) {
    android_app *app = (android_app*)activity->instance;
    const char cmd = APP_CMD_WINDOW_RESIZED;
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
}
static void onContentRectChanged(ANativeActivity* activity, const ARect*) {
    android_app *app = (android_app*)activity->instance;
    const char cmd = APP_CMD_CONTENT_RECT_CHANGED;
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
}
//wanna safe setting to Android Bundle safe instance? 
static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
    android_app *app = (android_app*)activity->instance;
    pthread_mutex_lock(&app->mutex);
    const char cmd = APP_CMD_SAVE_STATE;
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
    pthread_mutex_unlock(&app->mutex);
    *outLen = 0;
    return nullptr;
}
static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow*) {
    android_app *app = (android_app*)activity->instance;
    if (app->window == NULL) return;
    pthread_mutex_lock(&app->mutex);
    const char cmd = APP_CMD_TERM_WINDOW;
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
    while (app->appCmdState != APP_CMD_TERM_WINDOW) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue*) {
    android_app *app = (android_app*)activity->instance;
    pthread_mutex_lock(&app->mutex);
    if(app->inputQueue == NULL) return;
    const char cmd = APP_CMD_INPUT_TERM;
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
    while (app->appCmdState != cmd)
      pthread_cond_wait(&app->cond, &app->mutex);
    pthread_mutex_unlock(&app->mutex);
}
static void onPause(ANativeActivity* activity) {
    android_app *app = (android_app*)activity->instance;
    pthread_mutex_lock(&app->mutex);
    const char cmd = APP_CMD_PAUSE;
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
    while (app->appCmdState != cmd) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onStop(ANativeActivity* activity) {
    android_app *app = (android_app*)activity->instance;
    const char cmd = APP_CMD_STOP;
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
}
static void onDestroy(ANativeActivity* activity) {
    android_app *app = (android_app*)activity->instance;
    pthread_mutex_lock(&app->mutex);
    const char cmd = APP_CMD_DESTROY;
    while (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
      LOGE("cannot write on pipe , %s", strerror(errno));
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


void ANativeActivity_onCreate(ANativeActivity* activity, void*, size_t) {
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
    activity->callbacks->onNativeWindowResized = onNativeWindowResized;
    activity->callbacks->onContentRectChanged = onContentRectChanged;
    activity->callbacks->onConfigurationChanged = onConfigurationChanged;
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onLowMemory = onLowMemory;
    activity->callbacks->onSaveInstanceState = onSaveInstanceState;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onDestroy = onDestroy;
    
    android_app* app = new android_app{};
    activity->instance = app;
    app->activity = activity;
    pthread_mutex_init(&app->mutex, NULL);
    pthread_cond_init(&app->cond, NULL);
    while(pipe(&app->msgread))
      LOGE("Failed to create pipe, %s", strerror(errno));
    pthread_attr_t attr; 
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&app->thread, &attr, android_app_entry, app);
    pthread_attr_destroy(&attr);
}

//native MainActivity.java
#include <jni.h>

extern "C" JNIEXPORT void JNICALL Java_com_ariasaproject_technowar_MainActivity_setInsets(JNIEnv *, jclass, jint left, jint top, jint right, jint bottom) {
    if (!a_graphics) return;
    a_graphics->cur_safe_insets.left = left;
    a_graphics->cur_safe_insets.top = top;
    a_graphics->cur_safe_insets.right = right;
    a_graphics->cur_safe_insets.bottom = bottom;
}




