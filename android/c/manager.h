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

extern void graphics_onWindowCreate (void *);
extern void graphics_onWindowDestroy (void);
extern void graphics_onWindowResizeDisplay (void);
extern void graphics_onWindowResize (void);
extern void graphics_resizeInsets  (float, float, float, float);
extern int graphics_preRender  (void);
extern void graphics_postRender  (void);
extern void graphics_term (void);

extern int graphics_init(void);

#endif // MANAGER_