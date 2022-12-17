#ifndef _NATIVE_APP_
#define _NATIVE_APP_ 1

#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>

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
    APP_CMD_DESTROY,
};
static void process_input(android_app*);
static void process_cmd(android_app*);
static void process_sensor(android_app*);
extern void android_main(android_app*);
#endif /* _NATIVE_APP_ */


