#ifndef MANAGER_
#define MANAGER_

#include <android/looper.h>
#include <android/sensor.h>


enum inputManagerState {
	INPUT_SENSOR_ENABLED = 1,
};
#define MAX_SENSOR_COUNT 3
enum {
	SENSOR_ACCELEROMETER = 0,
	SENSOR_GYROSCOPE = 1,
	SENSOR_MAGNETIC_FIELD = 2,
};

struct android_inputManager {
	AInputQueue *inputQueue;
	
	ASensorManager *sensorMngr;
	ASensorEventQueue *sensorQueue;
	struct {
		const ASensor *sensor;
		float value[3];
	} sensor_data[MAX_SENSOR_COUNT];
	
	int flags;
};

extern struct android_inputManager *android_inputManager_init(ALooper *);
extern void android_inputManager_setInputQueue (struct android_inputManager*, ALooper *, AInputQueue *);
extern void android_inputManager_switchSensor (struct android_inputManager*, void*);
extern void android_inputManager_term (struct android_inputManager*);

struct android_graphicsManager {
	
};


#endif // MANAGER_