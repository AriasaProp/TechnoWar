#ifndef MANAGER_
#define MANAGER_

extern void androidAssetManager_init(void *);
extern void androidAssetManager_term();

extern void androidInput_init(void *);
extern void androidInput_createInputQueue (void *);
extern void androidInput_destroyInputQueue ();
extern void androidInput_enableSensor ();
extern void androidInput_disableSensor ();
extern void androidInput_term ();

extern void androidGraphics_init();
extern void androidGraphics_onWindowCreate(void *);
extern void androidGraphics_onWindowDestroy();
extern void androidGraphics_onWindowResizeDisplay();
extern void androidGraphics_onWindowResize();
extern void androidGraphics_resizeInsets (float, float, float, float);
extern int androidGraphics_preRender ();
extern void androidGraphics_postRender ();
extern void androidGraphics_term();


#endif // MANAGER_