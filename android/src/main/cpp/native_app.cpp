#include <jni.h>
#include <memory>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <sys/resource.h>

#include "native_app.h"
#include "log.h"

static void free_saved_state(android_app *app) {
    pthread_mutex_lock(&app->mutex);
    if (app->savedState != NULL) {
        free(app->savedState);
        app->savedState = NULL;
        app->savedStateSize = 0;
    }
    pthread_mutex_unlock(&app->mutex);
}
int8_t android_app_read_cmd(android_app *app) {
    int8_t cmd;
    if (read(app->msgread, &cmd, sizeof(cmd)) == sizeof(cmd)) {
        switch (cmd) {
            case APP_CMD_SAVE_STATE:
                free_saved_state(android_app);
                break;
        }
        return cmd;
    } else {
        LOGI("No data on command pipe!");
    }
    return -1;
}
void android_app_pre_exec_cmd(android_app *app, int8_t cmd) {
    switch (cmd) {
        case APP_CMD_INPUT_CHANGED:
            LOGI("APP_CMD_INPUT_CHANGED\n");
            pthread_mutex_lock(&app->mutex);
            if (app->inputQueue != NULL) {
                AInputQueue_detachLooper(app->inputQueue);
            }
            app->inputQueue = app->pendingInputQueue;
            if (app->inputQueue != NULL) {
                LOGI("Attaching input queue to looper");
                AInputQueue_attachLooper(app->inputQueue,
                        app->looper, LOOPER_ID_INPUT, NULL,
                        &app->inputPollSource);
            }
            pthread_cond_broadcast(&app->cond);
            pthread_mutex_unlock(&app->mutex);
            break;
        case APP_CMD_INIT_WINDOW:
            LOGI("APP_CMD_INIT_WINDOW\n");
            pthread_mutex_lock(&app->mutex);
            app->window = app->pendingWindow;
            pthread_cond_broadcast(&app->cond);
            pthread_mutex_unlock(&app->mutex);
            break;
        case APP_CMD_TERM_WINDOW:
            pthread_mutex_lock(&app->mutex);
            app->window = NULL;
            pthread_cond_broadcast(&app->cond);
            pthread_mutex_unlock(&app->mutex);
            break;
        case APP_CMD_RESUME:
        case APP_CMD_START:
        case APP_CMD_PAUSE:
        case APP_CMD_STOP:
            LOGI("activityState=%d\n", cmd);
            pthread_mutex_lock(&app->mutex);
            app->activityState = cmd;
            pthread_cond_broadcast(&app->cond);
            pthread_mutex_unlock(&app->mutex);
            break;
        case APP_CMD_CONFIG_CHANGED:
            LOGI("APP_CMD_CONFIG_CHANGED\n");
            AConfiguration_fromAssetManager(app->config,app->activity->assetManager);
            break;
        case APP_CMD_DESTROY:
            LOGI("APP_CMD_DESTROY\n");
            app->destroyRequested = 1;
            break;
    }
}
void android_app_post_exec_cmd(android_app *app, int8_t cmd) {
    switch (cmd) {
        case APP_CMD_TERM_WINDOW:
            LOGI("APP_CMD_TERM_WINDOW\n");
            pthread_mutex_lock(&app->mutex);
            app->window = NULL;
            pthread_cond_broadcast(&app->cond);
            pthread_mutex_unlock(&app->mutex);
            break;
        case APP_CMD_SAVE_STATE:
            LOGI("APP_CMD_SAVE_STATE\n");
            pthread_mutex_lock(&app->mutex);
            app->stateSaved = 1;
            pthread_cond_broadcast(&app->cond);
            pthread_mutex_unlock(&app->mutex);
            break;
        case APP_CMD_RESUME:
            free_saved_state(android_app);
            break;
    }
}
void app_dummy() {
}
static void android_app_destroy(android_app *app) {
    free_saved_state(android_app);
    pthread_mutex_lock(&app->mutex);
    if (app->inputQueue != NULL) {
        AInputQueue_detachLooper(app->inputQueue);
    }
    AConfiguration_delete(app->config);
    app->destroyed = 1;
    pthread_cond_broadcast(&app->cond);
    pthread_mutex_unlock(&app->mutex);
    // Can't touch android_app object after this.
}
static void process_input(android_app *app, android_poll_source* source) {
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
static void process_cmd(android_app *app, android_poll_source* source) {
    int8_t cmd = android_app_read_cmd(app);
    android_app_pre_exec_cmd(app, cmd);
    if (app->onAppCmd != NULL) app->onAppCmd(app, cmd);
    android_app_post_exec_cmd(app, cmd);
}
static void* android_app_entry(void* param) {
    android_app *app = (android_app*)param;
    app->config = AConfiguration_new();
    AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
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
    android_main(android_app);
    android_app_destroy(android_app);
    return NULL;
}
static android_app *app_create(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    android_app *app = (android_app*)malloc(sizeof(android_app));
    memset(android_app, 0, sizeof(android_app));
    app->activity = activity;
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
    pthread_attr_t attr; 
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&app->thread, &attr, android_app_entry, android_app);
    // Wait for thread to start.
    pthread_mutex_lock(&app->mutex);
    while (!app->running) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
    return android_app;
}
static void android_app_write_cmd(android_app *app, int8_t cmd) {
    if (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd)) {
        LOGI("Failure writing android_app cmd: %s\n", strerror(errno));
    }
}
static void android_app_set_input(android_app *app, AInputQueue* inputQueue) {
    pthread_mutex_lock(&app->mutex);
    app->pendingInputQueue = inputQueue;
    android_app_write_cmd(android_app, APP_CMD_INPUT_CHANGED);
    while (app->inputQueue != app->pendingInputQueue) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void android_app_set_window(android_app *app, ANativeWindow* window) {
    pthread_mutex_lock(&app->mutex);
    if (app->pendingWindow != NULL) {
        android_app_write_cmd(android_app, APP_CMD_TERM_WINDOW);
    }
    app->pendingWindow = window;
    if (window != NULL) {
        android_app_write_cmd(android_app, APP_CMD_INIT_WINDOW);
    }
    while (app->window != app->pendingWindow) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void android_app_set_activity_state(android_app *app, int8_t cmd) {
    pthread_mutex_lock(&app->mutex);
    android_app_write_cmd(android_app, cmd);
    while (app->activityState != cmd) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void android_app_free(android_app *app) {
    pthread_mutex_lock(&app->mutex);
    android_app_write_cmd(android_app, APP_CMD_DESTROY);
    while (!app->destroyed) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
    close(app->msgread);
    close(app->msgwrite);
    pthread_cond_destroy(&app->cond);
    pthread_mutex_destroy(&app->mutex);
    free(android_app);
}
static void onDestroy(ANativeActivity* activity) {
    android_app_free((android_app*)activity->instance);
}
static void onStart(ANativeActivity* activity) {
    android_app_set_activity_state((android_app*)activity->instance, APP_CMD_START);
}
static void onResume(ANativeActivity* activity) {
    android_app_set_activity_state((android_app*)activity->instance, APP_CMD_RESUME);
}
static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
    android_app *app = (android_app*)activity->instance;
    void* savedState = NULL;
    pthread_mutex_lock(&app->mutex);
    app->stateSaved = 0;
    android_app_write_cmd(android_app, APP_CMD_SAVE_STATE);
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
    android_app_set_activity_state((android_app*)activity->instance, APP_CMD_PAUSE);
}
static void onStop(ANativeActivity* activity) {
    android_app_set_activity_state((android_app*)activity->instance, APP_CMD_STOP);
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
    android_app_set_window((android_app*)activity->instance, window);
}
static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
    android_app_set_window((android_app*)activity->instance, NULL);
}
static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    android_app_set_input((android_app*)activity->instance, queue);
}
static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
    android_app_set_input((android_app*)activity->instance, NULL);
}
void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    activity->callbacks->onDestroy = onDestroy;
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onSaveInstanceState = onSaveInstanceState;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onConfigurationChanged = onConfigurationChanged;
    activity->callbacks->onLowMemory = onLowMemory;
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
    activity->instance = android_app_create(activity, savedState, savedStateSize);
}

