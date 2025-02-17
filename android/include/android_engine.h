#ifndef ANDROID_ENGINE_
#define ANDROID_ENGINE_ 1

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

// set all engine funct
extern void init_engine (AAssetManager *, int, ALooper *);
// unset all engine funct
extern void term_engine ();

extern void android_input_set_input_queue (ALooper *, AInputQueue *);
extern void android_input_attach_sensor ();
extern void android_input_detach_sensor ();

extern float android_graphics_cur_safe_insets[4];
extern void (*android_graphics_onWindowChange) (ANativeWindow *);
extern void (*android_graphics_onWindowResizeDisplay) ();
extern void (*android_graphics_onWindowResize) ();
extern int  (*android_graphics_preRender) ();
extern void (*android_graphics_postRender) (int);


#endif // ANDROID_ENGINE_