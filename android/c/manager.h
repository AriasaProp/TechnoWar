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

typedef struct {
  void (*onWindowCreate) (void *);
  void (*onWindowDestroy) (void);
  void (*onWindowResizeDisplay) (void);
  void (*onWindowResize) (void);
  void (*resizeInsets)  (float, float, float, float);
  int (*preRender)  (void);
  void (*postRender)  (void);
  void (*term) (void);
}  AndroidGraphicsAPI;
extern AndroidGraphicsAPI gapi;

extern int opengles_init(void);

#endif // MANAGER_