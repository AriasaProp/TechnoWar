#ifndef MANAGER_
#define MANAGER_

extern void androidTimerManager_init(void);
extern void androidTimerManager_onFrame(void);
extern void androidTimerManager_term(void);

extern void androidAssetManager_init(void *);
extern void androidAssetManager_term(void);

extern void androidInput_init(void *);
extern void androidInput_createInputQueue(void *);
extern void androidInput_destroyInputQueue(void);
extern void androidInput_enableSensor(void);
extern void androidInput_disableSensor(void);
extern void androidInput_term(void);

extern void (*androidGraphics_onWindowCreate)(void *);
extern void (*androidGraphics_onWindowDestroy)(void);
extern void (*androidGraphics_onWindowResizeDisplay)(void);
extern void (*androidGraphics_onWindowResize)(void);
extern void (*androidGraphics_resizeInsets)(float, float, float, float);
extern int (*androidGraphics_preRender)(void);
extern void (*androidGraphics_postRender)(void);
extern void (*androidGraphics_term)(void);

extern int opengles_init(void);
extern int vulkan_init(void);

#endif // MANAGER_