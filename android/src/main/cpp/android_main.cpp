#include <algorithm>
#include <cassert>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <string>
#include <sys/resource.h>
#include <unistd.h>

#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

#include "engine.hpp"
#include "log.hpp"
#include "main_game.hpp"
#include <jni.h>

#include "android_asset/android_asset.hpp"
#include "android_info/android_info.hpp"
#include "android_input/android_input.hpp"
#include "api_graphics/android_graphics.hpp"
#include "api_graphics/opengles_graphics.hpp"

static JavaVM *global_jvm;

jint JNI_OnLoad (JavaVM *vm, void *) {
  try {
    JNIEnv *env;
    if (vm->GetEnv ((void **)&env, JNI_VERSION_1_6) != JNI_OK) throw "";

    global_jvm = vm;
  } catch (...) {
    return JNI_ERR;
  }
  return JNI_VERSION_1_6;
}

void JNI_OnUnload (JavaVM *vm, void *) {
  JNIEnv *env;
  if (vm->GetEnv ((void **)&env, JNI_VERSION_1_6) != JNI_OK)
    return;

  global_jvm = nullptr;
}
enum APP_CMD : unsigned char {
  APP_CMD_CREATE = 0,
  APP_CMD_START,
  APP_CMD_RESUME,
  APP_CMD_INPUT_INIT,
  APP_CMD_INIT_WINDOW,
  APP_CMD_WINDOW_RESIZED,
  APP_CMD_CONTENT_RECT_CHANGED,
  APP_CMD_CONFIG_CHANGED,
  APP_CMD_FOCUS_CHANGED,
  APP_CMD_LOW_MEMORY,
  APP_CMD_SAVE_STATE,
  APP_CMD_TERM_WINDOW,
  APP_CMD_INPUT_TERM,
  APP_CMD_PAUSE,
  APP_CMD_STOP,
  APP_CMD_DESTROY,
};
/*
static JavaVMAttachArgs attachArgs{
    .version = JNI_VERSION_1_6,
    .name = "TechnoWar",
    .group = NULL};
*/
struct android_app {
  bool destroyed,
      wait_request = false;
  int msgread,
      msgwrite;
  ANativeActivity *activity;
  ANativeWindow *window = nullptr; // update in mainThread
  AInputQueue *inputQueue;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_t thread;
  android_graphics *graphics;
} *app = nullptr;

static void *android_app_entry (void *) {
	if (!app) return NULL;
	AConfiguration *aconfig = AConfiguration_new ();
	AConfiguration_fromAssetManager (aconfig, app->activity->assetManager);
	app->graphics = new opengles_graphics {};
	bool created = false;
	ALooper *looper = ALooper_prepare (ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
	try {
		bool running = false, started = false, resume = false;
		ALooper_addFd (looper, app->msgread, 1, ALOOPER_EVENT_INPUT, NULL, nullptr);
		android_asset a_asset (app->activity->assetManager);
		android_input a_input (looper);
		android_info inf (app->activity->sdkVersion);
		unsigned char read_cmd[2] {
			APP_CMD_CREATE,
			0
		};
		for (;;) {
			if (ALooper_pollAll ((started && running) ? 0: -1, nullptr, nullptr, nullptr) == 1) {
				if (read (app->msgread, read_cmd, sizeof read_cmd) != sizeof read_cmd) continue;
				switch (read_cmd[0]) {
					case APP_CMD_INIT_WINDOW:
					pthread_mutex_lock (&app->mutex);
					app->graphics->onWindowInit (app->window);
					pthread_mutex_unlock (&app->mutex);
					break;
					case APP_CMD_FOCUS_CHANGED:
					if (read_cmd[1])
					a_input.attach_sensor ();
					else
					a_input.detach_sensor ();
					break;
					case APP_CMD_INPUT_INIT:
					a_input.set_input_queue (looper, app->inputQueue);
					break;
					case APP_CMD_INPUT_TERM:
					if (app->inputQueue) {
						a_input.set_input_queue (looper, NULL);
						app->inputQueue = NULL;
					}
					break;
					case APP_CMD_TERM_WINDOW:
					app->graphics->onWindowTerm ();
					break;
					case APP_CMD_PAUSE:
					if (app->graphics->preRender ()) {
						Main::pause ();
						app->graphics->postRender (false);
						running = false;
					}
					break;
					default:
					break;
				}
				pthread_mutex_lock (&app->mutex);
				app->wait_request = false;
				pthread_cond_broadcast (&app->cond);
				pthread_mutex_unlock (&app->mutex);
				// no need wait
				switch (read_cmd[0]) {
					case APP_CMD_SAVE_STATE:
					break;
					case APP_CMD_CONFIG_CHANGED:
					AConfiguration_fromAssetManager (aconfig, app->activity->assetManager);
					break;
					case APP_CMD_STOP:
					started = false;
					break;
					case APP_CMD_START:
					started = true;
					break;
					case APP_CMD_RESUME:
					running = true;
					resume = true;
					break;
					case APP_CMD_CONTENT_RECT_CHANGED:
					app->graphics->onWindowResize (1);
					break;
					case APP_CMD_WINDOW_RESIZED:
					app->graphics->onWindowResize (2);
					break;
					case APP_CMD_LOW_MEMORY:
					break;
					case APP_CMD_DESTROY:
					throw "";
					default:
					break;
				}
				continue;
			}
			if (app->graphics->ready () &&
				app->graphics->preRender ()) {
					
				a_input.process_event();
				
				if (!created) {
					Main::start ();
					created = true;
					resume = false;
				} else if (resume) {
					Main::resume ();
					resume = false;
				}
				Main::render ();
				app->graphics->postRender (false);
			}
		}
	} catch (...) {
		// when destroy
		if (app->graphics->preRender ())
		Main::end ();
		created = false;
		app->graphics->postRender (true);
	}
	// loop ends
	ALooper_removeFd (looper, app->msgread);
	delete app->graphics;
	app->graphics = nullptr;
	AConfiguration_delete (aconfig);
	pthread_mutex_lock (&app->mutex);
	app->destroyed = true;
	pthread_cond_broadcast (&app->cond);
	pthread_mutex_unlock (&app->mutex);
	return NULL;
}
static void write_android_cmd (android_app *app, unsigned char cmd, unsigned char cmd1 = 0) {
  pthread_mutex_lock (&app->mutex);
  app->wait_request = true;
  pthread_mutex_unlock (&app->mutex);
  static unsigned char write_cmd[2];
  write_cmd[0] = cmd;
  write_cmd[1] = cmd1;
  // loop till msg sended
  while (write (app->msgwrite, write_cmd, sizeof write_cmd) != sizeof write_cmd)
    LOGE ("cannot write on pipe , %s", strerror (errno));
  pthread_mutex_lock (&app->mutex);
  while (app->wait_request)
    pthread_cond_wait (&app->cond, &app->mutex);
  pthread_mutex_unlock (&app->mutex);
}

void ANativeActivity_onCreate (ANativeActivity *activity, void *, size_t) {
  // initialize application
  app = new android_app;
  app->activity = activity;
  pthread_mutex_init (&app->mutex, NULL);
  pthread_cond_init (&app->cond, NULL);
  while (pipe (&app->msgread) == -1) {
    // force loop foor provide pipe
    LOGE ("Failed to create pipe, %s", strerror (errno));
  }

  // initialize lifecycle
  activity->callbacks->onStart = [] (ANativeActivity *) {
    write_android_cmd (app, APP_CMD_START, 0);
  };
  activity->callbacks->onResume = [] (ANativeActivity *) {
    write_android_cmd (app, APP_CMD_RESUME, 0);
  };
  activity->callbacks->onNativeWindowCreated = [] (ANativeActivity *, ANativeWindow *window) {
    pthread_mutex_lock (&app->mutex);
    app->window = window;
    pthread_mutex_unlock (&app->mutex);
    write_android_cmd (app, APP_CMD_INIT_WINDOW, 0);
  };
  activity->callbacks->onInputQueueCreated = [] (ANativeActivity *, AInputQueue *queue) {
    if (app->inputQueue)
      write_android_cmd (app, APP_CMD_INPUT_TERM, 0);
    app->inputQueue = queue;
    write_android_cmd (app, APP_CMD_INPUT_INIT, 0);
  };
  activity->callbacks->onConfigurationChanged = [] (ANativeActivity *) {
    write_android_cmd (app, APP_CMD_CONFIG_CHANGED, 0);
  };
  activity->callbacks->onLowMemory = [] (ANativeActivity *) {
    write_android_cmd (app, APP_CMD_LOW_MEMORY, 0);
  };
  activity->callbacks->onWindowFocusChanged = [] (ANativeActivity *, int focused) {
    write_android_cmd (app, APP_CMD_FOCUS_CHANGED, focused ? 1 : 0);
  };
  activity->callbacks->onNativeWindowResized = [] (ANativeActivity *, ANativeWindow *) {
    write_android_cmd (app, APP_CMD_WINDOW_RESIZED, 0);
  };
  activity->callbacks->onContentRectChanged = [] (ANativeActivity *, const ARect *) {
    write_android_cmd (app, APP_CMD_CONTENT_RECT_CHANGED, 0);
  };
  activity->callbacks->onSaveInstanceState = [] (ANativeActivity *, size_t *outLen) -> void * {
    write_android_cmd (app, APP_CMD_SAVE_STATE, 0);
    *outLen = 0;
    return nullptr;
  };
  activity->callbacks->onNativeWindowDestroyed = [] (ANativeActivity *, ANativeWindow *) {
    pthread_mutex_lock (&app->mutex);
    app->window = nullptr;
    pthread_mutex_unlock (&app->mutex);
    write_android_cmd (app, APP_CMD_TERM_WINDOW, 0);
  };
  activity->callbacks->onInputQueueDestroyed = [] (ANativeActivity *, AInputQueue *) {
    if (!app->inputQueue) return;
    write_android_cmd (app, APP_CMD_INPUT_TERM, 0);
  };
  activity->callbacks->onPause = [] (ANativeActivity *) {
    write_android_cmd (app, APP_CMD_PAUSE, 0);
  };
  activity->callbacks->onStop = [] (ANativeActivity *) {
    write_android_cmd (app, APP_CMD_STOP, 0);
  };
  activity->callbacks->onDestroy = [] (ANativeActivity *) {
    write_android_cmd (app, APP_CMD_DESTROY, 0);
    pthread_mutex_lock (&app->mutex);
    while (!app->destroyed)
      pthread_cond_wait (&app->cond, &app->mutex);
    pthread_mutex_unlock (&app->mutex);
    close (app->msgread);
    close (app->msgwrite);
    pthread_cond_destroy (&app->cond);
    pthread_mutex_destroy (&app->mutex);
    delete app;
  };

  // start thread game
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  pthread_create (&app->thread, &attr, android_app_entry, app);
  pthread_attr_destroy (&attr);
}

// native MainActivity.java

extern "C" JNIEXPORT void Java_com_ariasaproject_technowar_MainActivity_insetNative (JNIEnv *, jobject, jint left, jint top, jint right, jint bottom) {
  if (!app->graphics) return;
  app->graphics->cur_safe_insets[0] = left;
  app->graphics->cur_safe_insets[1] = top;
  app->graphics->cur_safe_insets[2] = right;
  app->graphics->cur_safe_insets[3] = bottom;
}