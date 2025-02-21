#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/choreographer.h>
#include <android/configuration.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/set_abort_message.h>

#include <assert.h>
#include <errno.h>
#include <jni.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

#include "log.h"
#include "manager.h"
#include "util.h"

struct msg_pipe {
  int8_t cmd;
  void *data;
};

struct android_app {
  void *userData;
  ANativeActivity *activity;
  AConfiguration *config;
  void *savedState;
  size_t savedStateSize;
  ALooper *looper;
  ANativeWindow *window;
  ARect contentRect;
  
  int8_t cmdState;
  int destroyRequested;

  pthread_mutex_t mutex;
  pthread_cond_t cond;

  int msgread, msgwrite;

  pthread_t thread;

  int running;
  int destroyed;
  int redrawNeeded;
  ARect pendingContentRect;
} *app = NULL;

enum {
  APP_CMD_INPUT_CREATED,
  APP_CMD_INPUT_DESTROYED,
  APP_CMD_WINDOW_CREATED,
  APP_CMD_WINDOW_DESTROYED,
  APP_CMD_WINDOW_RESIZE,
  APP_CMD_WINDOW_REDRAW,
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
struct SavedState {
  float angle;
  int32_t x;
  int32_t y;
};

struct Engine {
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  int32_t width;
  int32_t height;
  struct SavedState state;
  int running_;
};

static void ScheduleNextTick (struct Engine *);
static void Tick (long UNUSED (timeout), void *data) {
  struct Engine *engine = (struct Engine *)data;
  if (engine->running_) return;
  ScheduleNextTick (engine);
  engine->state.angle += .01f;
  if (engine->state.angle > 1) {
    engine->state.angle = 0;
  }
  if (engine->display == NULL) {
    // No display.
    return;
  }

  // Just fill the screen with a color.
  glClearColor (((float)engine->state.x) / engine->width, engine->state.angle, ((float)engine->state.y) / engine->height, 1);
  glClear (GL_COLOR_BUFFER_BIT);

  eglSwapBuffers (engine->display, engine->surface);
}
static void ScheduleNextTick (struct Engine *engine) {
  if (engine->running_) return;
  AChoreographer_postFrameCallback (AChoreographer_getInstance (), Tick, engine);
}
static int engine_init_display (struct Engine *engine) {
  // initialize OpenGL ES and EGL

  /*
   * Here specify the attributes of the desired configuration.
   * Below, we select an EGLConfig with at least 8 bits per color
   * component compatible with on-screen windows
   */
  const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE};
  EGLint w, h, format;
  EGLint numConfigs;
  EGLConfig config = NULL;
  EGLSurface surface;
  EGLContext context;

  EGLDisplay display = eglGetDisplay (EGL_DEFAULT_DISPLAY);

  eglInitialize (display, NULL, NULL);

  /* Here, the application chooses the configuration it desires.
   * find the best match if possible, otherwise use the very first one
   */
  eglChooseConfig (display, attribs, NULL, 0, &numConfigs);
  EGLConfig *supportedConfigs = (EGLConfig *)new_mem (sizeof (EGLConfig) * numConfigs);
  eglChooseConfig (display, attribs, supportedConfigs, numConfigs, &numConfigs);
  if (!numConfigs) {
    LOGW ("Unable to initialize EGLConfig");
    return -1;
  }

  config = supportedConfigs[0];
  for (EGLint i = 0; i < numConfigs; ++i) {
    EGLint r, g, b, d;
    if (eglGetConfigAttrib (display, supportedConfigs[i], EGL_RED_SIZE, &r) &&
        eglGetConfigAttrib (display, supportedConfigs[i], EGL_GREEN_SIZE, &g) &&
        eglGetConfigAttrib (display, supportedConfigs[i], EGL_BLUE_SIZE, &b) &&
        eglGetConfigAttrib (display, supportedConfigs[i], EGL_DEPTH_SIZE, &d) && r == 8 &&
        g == 8 && b == 8 && d == 0) {
      config = supportedConfigs[i];
      break;
    }
  }
  free_mem (supportedConfigs);

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib (display, config, EGL_NATIVE_VISUAL_ID, &format);
  surface =
      eglCreateWindowSurface (display, config, app->window, NULL);

  /* A version of OpenGL has not been specified here.  This will default to
   * OpenGL 1.0.  You will need to change this if you want to use the newer
   * features of OpenGL like shaders. */
  context = eglCreateContext (display, config, NULL, NULL);

  if (eglMakeCurrent (display, surface, surface, context) == EGL_FALSE) {
    LOGW ("Unable to eglMakeCurrent");
    return -1;
  }

  eglQuerySurface (display, surface, EGL_WIDTH, &w);
  eglQuerySurface (display, surface, EGL_HEIGHT, &h);

  engine->display = display;
  engine->context = context;
  engine->surface = surface;
  engine->width = w;
  engine->height = h;
  engine->state.angle = 0;

  // Initialize GL state.
  glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  glEnable (GL_CULL_FACE);
  glDisable (GL_DEPTH_TEST);

  return 0;
}
static void engine_term_display (struct Engine *engine) {
  if (engine->display != EGL_NO_DISPLAY) {
    eglMakeCurrent (engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (engine->context != EGL_NO_CONTEXT) {
      eglDestroyContext (engine->display, engine->context);
    }
    if (engine->surface != EGL_NO_SURFACE) {
      eglDestroySurface (engine->display, engine->surface);
    }
    eglTerminate (engine->display);
  }
  engine->display = EGL_NO_DISPLAY;
  engine->context = EGL_NO_CONTEXT;
  engine->surface = EGL_NO_SURFACE;
}
static int engine_handle_input (AInputEvent *event) {
  struct Engine *engine = (struct Engine *)app->userData;
  if (AInputEvent_getType (event) == AINPUT_EVENT_TYPE_MOTION) {
    engine->state.x = AMotionEvent_getX (event, 0);
    engine->state.y = AMotionEvent_getY (event, 0);
    return 1;
  }
  return 0;
}

static void free_saved_state () {
  pthread_mutex_lock (&app->mutex);
  if (app->savedState != NULL) {
    free_mem (app->savedState);
    app->savedState = NULL;
    app->savedStateSize = 0;
  }
  pthread_mutex_unlock (&app->mutex);
}

static int process_cmd (int fd, int UNUSED (event), void *UNUSED (data)) {
  static struct msg_pipe rmsg;
  if (read (fd, &rmsg, sizeof (struct msg_pipe)) == sizeof (struct msg_pipe)) {
    struct Engine *engine = (struct Engine *)app->userData;
    switch (rmsg.cmd) {
    case APP_CMD_SAVE_STATE:
      // pre
      free_saved_state ();
      // The system has asked us to save our current state.  Do so.
      app->savedState = new_mem (sizeof (struct SavedState));
      *((struct SavedState *)app->savedState) = engine->state;
      app->savedStateSize = sizeof (struct SavedState);
      break;
    case APP_CMD_INPUT_CREATED:
      android_inputManager_createInputQueue ((AInputQueue *)rmsg.data);
      break;
    case APP_CMD_INPUT_DESTROYED:
      android_inputManager_destroyInputQueue ();
      break;
    case APP_CMD_WINDOW_CREATED:
      app->window = (ANativeWindow *)rmsg.data;
      engine_init_display (engine);
      engine->running_ &= ~2;
      ScheduleNextTick (engine);
      break;
    case APP_CMD_WINDOW_DESTROYED:
      engine->running_ |= 2;
      engine_term_display (engine);
      app->window = NULL;
      break;
    case APP_CMD_RESUME:
      // post
      free_saved_state ();
      engine->running_ &= ~1;
      ScheduleNextTick (engine);
      break;
    case APP_CMD_PAUSE:
      engine->running_ |= 1;
      break;
    case APP_CMD_CONTENT_RECT_CHANGED:
      break;
    case APP_CMD_WINDOW_RESIZE:
      break;
    case APP_CMD_WINDOW_REDRAW:
      break;
    case APP_CMD_START:
    case APP_CMD_STOP:
      break;
    case APP_CMD_GAINED_FOCUS:
      android_inputManager_enableSensor ();
      break;
    case APP_CMD_LOST_FOCUS:
      android_inputManager_disableSensor ();
      break;
    case APP_CMD_CONFIG_CHANGED:
      AConfiguration_fromAssetManager (app->config, (AAssetManager *)rmsg.data);
      break;
    case APP_CMD_DESTROY:
      app->destroyRequested = 1;
      break;
    }
    pthread_mutex_lock (&app->mutex);
    app->cmdState = rmsg.cmd;
    pthread_cond_broadcast (&app->cond);
    pthread_mutex_unlock (&app->mutex);
    return 1;
  } else {
    LOGE ("No data on command pipe!");
    return 0;
  }
}

static void *android_app_entry (void *param) {
  app->config = AConfiguration_new ();
  ANativeActivity *activity = (ANativeActivity *)param;
  AConfiguration_fromAssetManager (app->config, activity->assetManager);

  app->looper = ALooper_prepare (0);
  ALooper_addFd (app->looper, app->msgread, 1, ALOOPER_EVENT_INPUT, process_cmd, NULL);

  pthread_mutex_lock (&app->mutex);
  app->running = 1;
  pthread_cond_broadcast (&app->cond);
  pthread_mutex_unlock (&app->mutex);

  struct Engine engine = {0};
  app->userData = &engine;

  android_inputManager_init (app->looper);
  android_inputManager_listener (engine_handle_input);

  if (app->savedState != NULL) {
    engine.state = *(struct SavedState *)app->savedState;
  }

  while (!app->destroyRequested) {
    if (ALooper_pollOnce (-1, NULL, NULL, NULL) == ALOOPER_POLL_ERROR) {
      LOGE ("ALooper_pollOnce returned an error");
    }
  }

  engine_term_display (&engine);
  android_inputManager_term ();

  free_saved_state ();
  AConfiguration_delete (app->config);
  pthread_mutex_lock (&app->mutex);
  app->destroyed = 1;
  pthread_cond_broadcast (&app->cond);
  pthread_mutex_unlock (&app->mutex);
  // Can't touch app object after this.
  return NULL;
}

static void android_app_write_cmd (int8_t cmd, void *data) {
  static struct msg_pipe wmsg;
  wmsg.cmd = cmd;
  wmsg.data = data;
  if (write (app->msgwrite, &wmsg, sizeof (struct msg_pipe)) != sizeof (struct msg_pipe)) {
    LOGE ("Failure writing android_app cmd: %s\n", strerror (errno));
  }
  pthread_mutex_lock (&app->mutex);
  while (app->cmdState != cmd) {
    pthread_cond_wait (&app->cond, &app->mutex);
  }
  pthread_cond_broadcast (&app->cond);
  pthread_mutex_unlock (&app->mutex);
}

static void onDestroy (ANativeActivity *UNUSED (activity)) {
  android_app_write_cmd (APP_CMD_DESTROY, NULL);
  pthread_mutex_lock (&app->mutex);
  while (!app->destroyed) {
    pthread_cond_wait (&app->cond, &app->mutex);
  }
  pthread_mutex_unlock (&app->mutex);

  close (app->msgread);
  close (app->msgwrite);
  pthread_cond_destroy (&app->cond);
  pthread_mutex_destroy (&app->mutex);
  free_mem (app);
  app = NULL;
}
static void onStart (ANativeActivity *UNUSED (activity)) {
  android_app_write_cmd (APP_CMD_START, NULL);
}
static void onResume (ANativeActivity *UNUSED (activity)) {
  android_app_write_cmd (APP_CMD_RESUME, NULL);
}
static void *onSaveInstanceState (ANativeActivity *UNUSED (activity), size_t *outLen) {
  void *savedState = NULL;
  android_app_write_cmd (APP_CMD_SAVE_STATE, NULL);

  if (app->savedState != NULL) {
    savedState = app->savedState;
    *outLen = app->savedStateSize;
    app->savedState = NULL;
    app->savedStateSize = 0;
  }
  return savedState;
}
static void onPause (ANativeActivity *UNUSED (activity)) {
  android_app_write_cmd (APP_CMD_PAUSE, NULL);
}
static void onStop (ANativeActivity *UNUSED (activity)) {
  android_app_write_cmd (APP_CMD_STOP, NULL);
}
static void onConfigurationChanged (ANativeActivity *activity) {
  android_app_write_cmd (APP_CMD_CONFIG_CHANGED, (void *)activity->assetManager);
}
static void onLowMemory (ANativeActivity *UNUSED (activity)) {
  android_app_write_cmd (APP_CMD_LOW_MEMORY, NULL);
}
static void onWindowFocusChanged (ANativeActivity *UNUSED (activity), int focused) {
  android_app_write_cmd (focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS, (void*)((intptr_t)focused));
}
static void onContentRectChanged (ANativeActivity *UNUSED (activity), const ARect *rect) {
  android_app_write_cmd (APP_CMD_CONTENT_RECT_CHANGED, (void*)rect);
}
static void onNativeWindowResized (ANativeActivity *UNUSED (activity), ANativeWindow *UNUSED (window)) {
  android_app_write_cmd (APP_CMD_WINDOW_RESIZE, NULL);
}
static void onNativeWindowRedrawNeeded (ANativeActivity *UNUSED (activity), ANativeWindow *UNUSED (window)) {
  android_app_write_cmd (APP_CMD_WINDOW_REDRAW, NULL);
}
static void onNativeWindowCreated (ANativeActivity *UNUSED (activity), ANativeWindow *window) {
  android_app_write_cmd (APP_CMD_WINDOW_CREATED, (void *)window);
}
static void onNativeWindowDestroyed (ANativeActivity *UNUSED (activity), ANativeWindow *UNUSED (window)) {
  android_app_write_cmd (APP_CMD_WINDOW_DESTROYED, NULL);
}
static void onInputQueueCreated (ANativeActivity *UNUSED (activity), AInputQueue *queue) {
  android_app_write_cmd (APP_CMD_INPUT_CREATED, (void *)queue);
}
static void onInputQueueDestroyed (ANativeActivity *UNUSED (activity), AInputQueue *UNUSED (queue)) {
  android_app_write_cmd (APP_CMD_INPUT_DESTROYED, NULL);
}

void ANativeActivity_onCreate (ANativeActivity *activity, void *savedState, size_t savedStateSize) {
  activity->callbacks->onDestroy = onDestroy;
  activity->callbacks->onStart = onStart;
  activity->callbacks->onResume = onResume;
  activity->callbacks->onSaveInstanceState = onSaveInstanceState;
  activity->callbacks->onPause = onPause;
  activity->callbacks->onStop = onStop;
  activity->callbacks->onConfigurationChanged = onConfigurationChanged;
  activity->callbacks->onLowMemory = onLowMemory;
  activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
  activity->callbacks->onContentRectChanged = onContentRectChanged;
  activity->callbacks->onNativeWindowResized = onNativeWindowResized;
  activity->callbacks->onNativeWindowRedrawNeeded = onNativeWindowRedrawNeeded;
  activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
  activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
  activity->callbacks->onInputQueueCreated = onInputQueueCreated;
  activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;

  app = (struct android_app *)new_imem (sizeof (struct android_app));
  app->activity = activity;

  pthread_mutex_init (&app->mutex, NULL);
  pthread_cond_init (&app->cond, NULL);

  if (savedState != NULL) {
    app->savedState = new_mem (savedStateSize);
    app->savedStateSize = savedStateSize;
    memcpy (app->savedState, savedState, savedStateSize);
  }

  if (pipe (&app->msgread)) {
    LOGE ("could not create pipe: %s", strerror (errno));
    return;
  }

  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  pthread_create (&app->thread, &attr, android_app_entry, activity);

  // Wait for thread to start.
  pthread_mutex_lock (&app->mutex);
  while (!app->running) {
    pthread_cond_wait (&app->cond, &app->mutex);
  }
  pthread_mutex_unlock (&app->mutex);
}

// native MainActivity.java
JNIEXPORT void Java_com_ariasaproject_technowar_MainActivity_insetNative (JNIEnv *UNUSED (env), jobject UNUSED (o), jint left, jint top, jint right, jint bottom) {
  android_graphicsManager_resizeInsets (left, top, right, bottom);
}
