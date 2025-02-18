#ifndef MANAGER_
#define MANAGER_

#include <android/looper.h>
#include <android/native_window.h>
#include <android/input.h>

extern void android_inputManager_init(ALooper *);
extern void android_inputManager_setInputQueue (ALooper *, AInputQueue *);
extern void android_inputManager_switchSensor (void*);
extern void android_inputManager_term ();

extern void android_graphicsManager_init();
extern void android_graphicsManager_onWindowChange(ANativeWindow *);
extern void android_graphicsManager_onWindowResizeDisplay();
extern void android_graphicsManager_resizeInsets (float, float, float, float);
extern void android_graphicsManager_onWindowResize();
extern int android_graphicsManager_preRender ();
extern void android_graphicsManager_postRender ();
extern void android_graphicsManager_term();


#endif // MANAGER_