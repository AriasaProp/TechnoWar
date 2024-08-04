#ifndef ANDROID_ENGINE_
#define ANDROID_ENGINE_ 1

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

//set all engine funct
extern void init_engine(AAssetManager *, int,ALooper *);
//unset all engine funct
extern void term_engine();

namespace android_input {
  extern void set_input_queue (ALooper *, AInputQueue *);
  extern void attach_sensor ();
  extern void detach_sensor ();
}

namespace android_graphics {
  extern float cur_safe_insets[4];
  // android
  extern void (*onWindowChange) (ANativeWindow *);
  extern void (*onWindowResize) (unsigned char);
  extern bool (*preRender) ();
  extern void (*postRender) (bool);
}

#endif // ANDROID_ENGINE_