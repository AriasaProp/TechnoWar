#ifndef MANAGER_
#define MANAGER_

#include <android/input.h>

extern void android_inputManager_init(ALooper *);
extern void android_inputManager_createInputQueue (AInputQueue *);
extern void android_inputManager_destroyInputQueue ();
extern void android_inputManager_enableSensor ();
extern void android_inputManager_disableSensor ();
extern void android_inputManager_term ();

#endif // MANAGER_