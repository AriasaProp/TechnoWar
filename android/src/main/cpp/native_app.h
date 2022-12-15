#ifndef NATIVE_APP
#define NATIVE_APP 1

#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>

struct android_app;
struct android_poll_source {
    int32_t id;
    android_app* app;
    void (*process)(android_app*, android_poll_source*);
};
struct android_app {
    void* userData;
    void (*onAppCmd)(android_app*, int32_t);
    int32_t (*onInputEvent)(android_app* app, AInputEvent* event);
    ANativeActivity* activity;
    AConfiguration* config;
    void* savedState;
    size_t savedStateSize;
    AInputQueue* inputQueue;
    ALooper* looper;
    ANativeWindow* window;
    ARect contentRect;
    int activityState;
    int destroyRequested;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int msgread;
    int msgwrite;
    pthread_t thread;
    android_poll_source cmdPollSource;
    android_poll_source inputPollSource;
    int running;
    int stateSaved;
    int destroyed;
    int redrawNeeded;
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
    APP_CMD_INPUT_CHANGED,
    APP_CMD_INIT_WINDOW,
    APP_CMD_TERM_WINDOW,
    APP_CMD_WINDOW_RESIZED,
    APP_CMD_WINDOW_REDRAW_NEEDED,
    APP_CMD_CONTENT_RECT_CHANGED,
		APP_CMD_GAINED_FOCUS,
		APP_CMD_LOST_FOCUS,
		APP_CMD_CONFIG_CHANGED,
		APP_CMD_LOW_MEMORY,
    APP_CMD_START,
    APP_CMD_RESUME,
    APP_CMD_SAVE_STATE,
    APP_CMD_PAUSE,
    APP_CMD_STOP,
    APP_CMD_DESTROY
};
int8_t android_app_read_cmd(android_app*);
void android_app_pre_exec_cmd(android_app*, int8_t);
void android_app_post_exec_cmd(android_app*, int8_t);
void app_dummy();
extern void android_main(android_app*);

#endif /* NATIVE_APP */


