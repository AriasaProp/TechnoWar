#ifndef MANAGER_
#define MANAGER_

extern void android_inputManager_init(void *);
extern void android_inputManager_createInputQueue (void *);
extern void android_inputManager_destroyInputQueue ();
extern void android_inputManager_enableSensor ();
extern void android_inputManager_disableSensor ();
extern void android_inputManager_term ();

extern void android_graphicsManager_init();
extern void android_graphicsManager_onWindowCreate(void *);
extern void android_graphicsManager_onWindowDestroy();
extern void android_graphicsManager_onWindowResizeDisplay();
extern void android_graphicsManager_onWindowResize();
extern void android_graphicsManager_resizeInsets (float, float, float, float);
extern int android_graphicsManager_preRender ();
extern void android_graphicsManager_postRender ();
extern void android_graphicsManager_term();

extern void android_assetManager_init(void*);
extern void android_assetManager_term();

#endif // MANAGER_