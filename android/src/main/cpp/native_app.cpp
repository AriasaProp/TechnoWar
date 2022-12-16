#include <jni.h>

#include <memory>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <sys/resource.h>

#include "native_app.h"
#include "log.h"

static void process_input(android_app* app) {
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
static void process_cmd(android_app* app) {
		int8_t cmd;
    if (read(app->msgread, &cmd, sizeof(cmd)) != sizeof(cmd)) {
      LOGI("No data on command pipe!");
    	return;
    }
  	switch (cmd) {
      case APP_CMD_RESUME:
          break;
      case APP_CMD_START:
          break;
      case APP_CMD_INIT_WINDOW:
          pthread_mutex_lock(&app->mutex);
          app->window = app->pendingWindow;
          pthread_cond_broadcast(&app->cond);
          pthread_mutex_unlock(&app->mutex);
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
      case APP_CMD_INPUT_CHANGED:
          pthread_mutex_lock(&app->mutex);
          if (app->inputQueue != NULL) {
              AInputQueue_detachLooper(app->inputQueue);
          }
          app->inputQueue = app->pendingInputQueue;
          if (app->inputQueue != NULL) {
              AInputQueue_attachLooper(app->inputQueue, app->looper, LOOPER_ID_INPUT, NULL, (void*)&process_input);
          }
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
      case APP_CMD_DESTROY:
          app->destroyRequested = true;
          break;
  	}
    if (app->onAppCmd != NULL) app->onAppCmd(app, cmd);
  	switch (cmd) {
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
      case APP_CMD_TERM_WINDOW:
        pthread_mutex_lock(&app->mutex);
        app->window = NULL;
        pthread_cond_broadcast(&app->cond);
        pthread_mutex_unlock(&app->mutex);
        break;
      default:
      	break;
  	}
}
static void *android_app_entry(void* param) {
    android_app *app = (android_app*)param;
    app->config = AConfiguration_new();
    AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
    app->looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(app->looper, app->msgread, LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL, (void*)&process_cmd);
    pthread_mutex_lock(&app->mutex);
    app->running = true;
    pthread_cond_broadcast(&app->cond);
    pthread_mutex_unlock(&app->mutex);
    
    android_main(app);
    
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
static void android_app_write_cmd(android_app* app, int8_t cmd) {
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
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_CONFIG_CHANGED);
}
static void onLowMemory(ANativeActivity* activity) {
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_LOW_MEMORY);
}
static void onWindowFocusChanged(ANativeActivity* activity, int focused) {
    android_app_write_cmd((android_app*)activity->instance, focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
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
    activity->callbacks->onSaveInstanceState = onSaveInstanceState;
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onConfigurationChanged = onConfigurationChanged;
    activity->callbacks->onLowMemory = onLowMemory;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onDestroy = onDestroy;
    
    android_app *app = new android_app;
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


