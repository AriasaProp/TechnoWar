#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <jni.h>
#include <limits.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <unistd.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

#include "core.h"
#include "engine.h"
#include "log.h"
#include "manager.h"
#include "util.h"

// #define NOTOAST

#ifdef NOTOAST
jobject ma = NULL;
jmethodID mi;
#endif

enum APP_CMD {
  APP_CMD_CREATE = 0,
  APP_CMD_START,
  APP_CMD_RESUME,
  APP_CMD_INPUT_UPDATE,
  APP_CMD_WINDOW_UPDATE,
  APP_CMD_WINDOW_RESIZED,
  APP_CMD_WINDOW_REDRAW_NEEDED,
  APP_CMD_CONTENT_RECT_CHANGED,
  APP_CMD_CONFIG_CHANGED,
  APP_CMD_FOCUS_CHANGED,
  APP_CMD_LOW_MEMORY,
  APP_CMD_SAVE_STATE,
  APP_CMD_PAUSE,
  APP_CMD_STOP,
  APP_CMD_DESTROY,
  APP_REQ_ACC,
};
enum {
  APP_FLAG_WAITING = 1,
  APP_FLAG_DESTROYED = 2,
};

struct android_app {
  int flags;
  int pipeMain, pipeChild;
  pthread_t thread;
} *app = NULL;

struct msg_pipe {
  enum APP_CMD cmd;
  void *data;
};

enum {
  STATE_CREATE = 1,
  STATE_RUNNING = 2,
  STATE_RESUME = 4,
  STATE_PAUSE = 8,
  STATE_DESTROY = 16,
  STATE_WINDOW_EXIST = 32,
};

static void *android_app_entry (void *n) {
  engine_init ();
  ANativeActivity *act = (ANativeActivity *)n;
  AConfiguration *aconfig = AConfiguration_new ();
  AConfiguration_fromAssetManager (aconfig, act->assetManager);
  ALooper *looper = ALooper_prepare (ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
  ALooper_addFd (looper, app->pipeMain, 1, ALOOPER_EVENT_INPUT, NULL, NULL);
  int StateFlags = STATE_CREATE;
  android_inputManager_init (looper);
  android_graphicsManager_init ();
  struct msg_pipe child_pipe = {APP_CMD_CREATE, NULL};
get_event:
  if (ALooper_pollOnce ((!(StateFlags & STATE_RUNNING) || !(StateFlags & STATE_WINDOW_EXIST)) * -1, NULL, NULL, NULL) == 1) {
    // activity handler
    read (app->pipeChild, &child_pipe, sizeof (struct msg_pipe));
    switch (child_pipe.cmd) {
    case APP_CMD_WINDOW_UPDATE:
      android_graphicsManager_onWindowChange ((ANativeWindow *)child_pipe.data);
      if (child_pipe.data != NULL) {
        StateFlags |= STATE_WINDOW_EXIST;
      } else {
        StateFlags &= ~STATE_WINDOW_EXIST;
      }
      child_pipe.cmd = APP_REQ_ACC;
      child_pipe.data = NULL;
      break;
    case APP_CMD_FOCUS_CHANGED:
      android_inputManager_switchSensor (child_pipe.data);
      child_pipe.cmd = APP_REQ_ACC;
      child_pipe.data = NULL;
      break;
    case APP_CMD_INPUT_UPDATE:
      android_inputManager_setInputQueue (looper, (AInputQueue *)child_pipe.data);
      break;
    case APP_CMD_PAUSE:
      StateFlags |= STATE_PAUSE;
      break;
    case APP_CMD_CONFIG_CHANGED:
      AConfiguration_fromAssetManager (aconfig, (AAssetManager *)child_pipe.data);
      break;
    case APP_CMD_SAVE_STATE:
    case APP_CMD_STOP:
    case APP_CMD_START:
    case APP_CMD_LOW_MEMORY:
    case APP_CMD_WINDOW_REDRAW_NEEDED:
    default:
      break;
    case APP_CMD_RESUME:
      StateFlags |= STATE_RUNNING | STATE_RESUME;
      break;
    case APP_CMD_CONTENT_RECT_CHANGED:
      android_graphicsManager_onWindowResize ();
      break;
    case APP_CMD_WINDOW_RESIZED:
      android_graphicsManager_onWindowResizeDisplay ();
      break;
    case APP_CMD_DESTROY:
      StateFlags |= STATE_DESTROY;
      goto render;
    }
  }
  if (!(StateFlags & STATE_RUNNING) || !(StateFlags & STATE_WINDOW_EXIST))
    goto get_event;
render: // base render
  // engine_input_process_event ();
  if (StateFlags & STATE_CREATE) {
    Main_start ();
    StateFlags &= ~STATE_CREATE;
    StateFlags &= ~STATE_RESUME;
  }
  if (StateFlags & STATE_RESUME) {
    Main_resume ();
    StateFlags &= ~STATE_RESUME;
  }

  if (android_graphicsManager_preRender ())
    Main_update ();

  if (StateFlags & STATE_PAUSE) {
    StateFlags &= ~STATE_PAUSE;
    StateFlags &= ~STATE_RUNNING;
    Main_pause ();
  }
  if (StateFlags & STATE_DESTROY) {
    Main_end ();
    goto end;
  }
  android_graphicsManager_postRender ();
  goto get_event;
end:              // loop ends
  StateFlags = 0; // reset flags
  android_graphicsManager_term ();
  android_inputManager_term ();
  ALooper_removeFd (looper, app->pipeMain);
  AConfiguration_delete (aconfig);
  child_pipe.cmd = APP_REQ_ACC;
  child_pipe.data = NULL;
  write (app->pipeMain, &child_pipe, sizeof (struct msg_pipe));
  return NULL;
}

static struct msg_pipe main_pipe;
static void onStart (ANativeActivity *act) {
  main_pipe.cmd = APP_CMD_START;
  main_pipe.data = NULL;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Start");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
#else
  (void)act;
#endif
}
static void onResume (ANativeActivity *act) {
  main_pipe.cmd = APP_CMD_RESUME;
  main_pipe.data = NULL;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Resume");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
#else
  (void)act;
#endif
}
static void onNativeWindowCreated (ANativeActivity *act, ANativeWindow *window) {
  main_pipe.cmd = APP_CMD_WINDOW_UPDATE;
  main_pipe.data = (void *)window;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Window Create");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
#else
  (void)act;
#endif
}
static void onInputQueueCreated (ANativeActivity *act, AInputQueue *queue) {
  main_pipe.cmd = APP_CMD_INPUT_UPDATE;
  main_pipe.data = (void *)queue;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Input Create");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
#else
  (void)act;
#endif
}
static void onConfigurationChanged (ANativeActivity *act) {
  main_pipe.cmd = APP_CMD_CONFIG_CHANGED;
  main_pipe.data = (void *)act->assetManager;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Config changes");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
#else
  (void)act;
#endif
}
static void onLowMemory (ANativeActivity *act) {
  main_pipe.cmd = APP_CMD_LOW_MEMORY;
  main_pipe.data = NULL;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Low memory");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
#else
  (void)act;
#endif
}
static void onWindowFocusChanged (ANativeActivity *act, int f) {
  main_pipe.cmd = APP_CMD_FOCUS_CHANGED;
  intptr_t focus = f;
  main_pipe.data = (void *)focus;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Focus changes");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
#else
  (void)act;
#endif
}
static void onNativeWindowResized (ANativeActivity *act, ANativeWindow *UNUSED (window)) {
  main_pipe.cmd = APP_CMD_WINDOW_RESIZED;
  main_pipe.data = NULL;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Resize Window");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
#else
  (void)act;
#endif
}
static void onNativeWindowRedrawNeeded (ANativeActivity *act, ANativeWindow *UNUSED (window)) {
  main_pipe.cmd = APP_CMD_WINDOW_REDRAW_NEEDED;
  main_pipe.data = NULL;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Redraw");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
#else
  (void)act;
#endif
}
static void onContentRectChanged (ANativeActivity *act, const ARect *UNUSED (r)) {
  main_pipe.cmd = APP_CMD_CONTENT_RECT_CHANGED;
  main_pipe.data = NULL;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Rect changes");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
#else
  (void)act;
#endif
}
static void *onSaveInstanceState (ANativeActivity *act, size_t *outLen) {
  main_pipe.cmd = APP_CMD_SAVE_STATE;
  main_pipe.data = NULL;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
  *outLen = 0;
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Saved state");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
#else
  (void)act;
#endif
  return NULL;
}
static void onNativeWindowDestroyed (ANativeActivity *act, ANativeWindow *UNUSED (window)) {
  main_pipe.cmd = APP_CMD_WINDOW_UPDATE;
  main_pipe.data = NULL;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Window lost");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
#else
  (void)act;
#endif
}
static void onInputQueueDestroyed (ANativeActivity *act, AInputQueue *UNUSED (queue)) {
  main_pipe.cmd = APP_CMD_INPUT_UPDATE;
  main_pipe.data = NULL;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Input Lost");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
}
static void onPause (ANativeActivity *act) {
  main_pipe.cmd = APP_CMD_PAUSE;
  main_pipe.data = NULL;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Pause");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
#else
  (void)act;
#endif
}
static void onStop (ANativeActivity *act) {
  main_pipe.cmd = APP_CMD_STOP;
  main_pipe.data = NULL;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Stop");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
#else
  (void)act;
#endif
}
static void onDestroy (ANativeActivity *act) {
  main_pipe.cmd = APP_CMD_DESTROY;
  main_pipe.data = NULL;
  write (app->pipeChild, &main_pipe, sizeof (struct msg_pipe));
  read (app->pipeMain, &main_pipe, sizeof (struct msg_pipe));
  close (app->pipeMain);
  close (app->pipeChild);
  free_mem (app);
  app = NULL;
#ifdef NOTOAST
  jstring msg = (*act->env)->NewStringUTF (act->env, "Destroyed");
  (*act->env)->CallVoidMethod (act->env, ma, mi, msg);
  (*act->env)->DeleteGlobalRef (act->env, ma);
  ma = NULL;
#else
  (void)act;
#endif
}

void ANativeActivity_onCreate (ANativeActivity *activity, void *UNUSED (savedata), size_t UNUSED (save_len)) {
  // initialize application
  app = (struct android_app *)new_imem (sizeof (struct android_app));
  while (socketpair (AF_UNIX, SOCK_STREAM, 0, &app->pipeMain) == -1) {
    // force loop to provide pipe
    LOGE ("Failed to create pipe, %s", strerror (errno));
  }

  // initialize lifecycle
  activity->callbacks->onStart = onStart;
  activity->callbacks->onResume = onResume;
  activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
  activity->callbacks->onInputQueueCreated = onInputQueueCreated;
  activity->callbacks->onConfigurationChanged = onConfigurationChanged;
  activity->callbacks->onLowMemory = onLowMemory;
  activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
  activity->callbacks->onNativeWindowResized = onNativeWindowResized;
  activity->callbacks->onNativeWindowRedrawNeeded = onNativeWindowRedrawNeeded;
  activity->callbacks->onContentRectChanged = onContentRectChanged;
  activity->callbacks->onSaveInstanceState = onSaveInstanceState;
  activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
  activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
  activity->callbacks->onPause = onPause;
  activity->callbacks->onStop = onStop;
  activity->callbacks->onDestroy = onDestroy;

  // start thread game
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  pthread_create (&app->thread, &attr, android_app_entry, activity);
  pthread_attr_destroy (&attr);
}

// native MainActivity.java
JNIEXPORT void Java_com_ariasaproject_technowar_MainActivity_insetNative (JNIEnv *env, jobject o, jint left, jint top, jint right, jint bottom) {
#ifdef NOTOAST
  if (ma == NULL) {
    ma = (*env)->NewGlobalRef (env, o);
    jclass jc = (*env)->FindClass (env, "com/ariasaproject/technowar/MainActivity");
    mi = (*env)->GetMethodID (env, jc, "showToast", "(Ljava/lang/String;)V");
  }
#else
  (void)env;
  (void)o;
#endif
  if (app == NULL) return;
  android_graphicsManager_resizeInsets (left, top, right, bottom);
}
