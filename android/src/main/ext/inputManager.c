#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

#include "manager.h"
#include "util.h"
// ~60 Hz
#define SENSOR_EVENT_RATE 1667

enum inputManagerState {
  INPUT_SENSOR_ENABLED = 1,
};
#define MAX_SENSOR_COUNT 3
enum {
  SENSOR_ACCELEROMETER = 0,
  SENSOR_GYROSCOPE = 1,
  SENSOR_MAGNETIC_FIELD = 2,
};

static struct android_inputManager {
  AInputQueue *inputQueue;

  ASensorManager *sensorMngr;
  ASensorEventQueue *sensorQueue;
  struct {
    const ASensor *sensor;
    float value[3];
  } sensor_data[MAX_SENSOR_COUNT];

  int flags;
} *m = NULL;

// processing input
static int android_inputManager_processInput (int UNUSED (fd), int UNUSED (e), void *UNUSED (data)) {
  AInputEvent *outEvent;
  if (!m->inputQueue) return 1;
  if (AInputQueue_getEvent (m->inputQueue, &outEvent) < 0) return 1;
  if (AInputQueue_preDispatchEvent (m->inputQueue, outEvent)) return 1;
  int32_t handled = 0;
  AInputQueue_finishEvent (m->inputQueue, outEvent, handled);
  return 1;
}
static int android_inputManager_processSensor (int UNUSED (fd), int UNUSED (e), void *UNUSED (data)) {
  ASensorEvent event[MAX_SENSOR_COUNT];
  size_t j;
  while ((j = ASensorEventQueue_getEvents (m->sensorQueue, event, MAX_SENSOR_COUNT)) > 0) {
    for (size_t i = 0; i < j; ++i) {
      switch (event[i].type) {
      case ASENSOR_TYPE_ACCELEROMETER:
        m->sensor_data[SENSOR_ACCELEROMETER].value[0] = event[i].acceleration.x;
        m->sensor_data[SENSOR_ACCELEROMETER].value[1] = event[i].acceleration.y;
        m->sensor_data[SENSOR_ACCELEROMETER].value[2] = event[i].acceleration.z;
        break;
      case ASENSOR_TYPE_GYROSCOPE:
        m->sensor_data[SENSOR_GYROSCOPE].value[0] = event[i].vector.x;
        m->sensor_data[SENSOR_GYROSCOPE].value[1] = event[i].vector.y;
        m->sensor_data[SENSOR_GYROSCOPE].value[2] = event[i].vector.z;
        break;
      case ASENSOR_TYPE_MAGNETIC_FIELD:
        m->sensor_data[SENSOR_MAGNETIC_FIELD].value[0] = event[i].magnetic.x;
        m->sensor_data[SENSOR_MAGNETIC_FIELD].value[1] = event[i].magnetic.y;
        m->sensor_data[SENSOR_MAGNETIC_FIELD].value[2] = event[i].magnetic.z;
        break;
      default:
        break;
      }
    }
  }
  return 1;
}

void android_inputManager_init (ALooper *looper) {
  m = (struct android_inputManager *)new_imem (sizeof (struct android_inputManager));
  while (m->sensorMngr == NULL) {
    m->sensorMngr = ASensorManager_getInstance ();
  }
  m->sensor_data[SENSOR_ACCELEROMETER].sensor = ASensorManager_getDefaultSensor (m->sensorMngr, ASENSOR_TYPE_ACCELEROMETER);
  m->sensor_data[SENSOR_GYROSCOPE].sensor = ASensorManager_getDefaultSensor (m->sensorMngr, ASENSOR_TYPE_GYROSCOPE);
  m->sensor_data[SENSOR_MAGNETIC_FIELD].sensor = ASensorManager_getDefaultSensor (m->sensorMngr, ASENSOR_TYPE_MAGNETIC_FIELD);
  m->sensorQueue = ASensorManager_createEventQueue (m->sensorMngr, looper, ALOOPER_POLL_CALLBACK, android_inputManager_processSensor, m);
}
void android_inputManager_setInputQueue (ALooper *looper, AInputQueue *queue) {
  if (m->inputQueue)
    AInputQueue_detachLooper (m->inputQueue);
  m->inputQueue = queue;
  if (m->inputQueue)
    AInputQueue_attachLooper (m->inputQueue, looper, ALOOPER_POLL_CALLBACK, android_inputManager_processInput, (void *)m);
}
void android_inputManager_switchSensor (void *s) {
  if (!s && (m->flags & INPUT_SENSOR_ENABLED)) {
    // detach
    for (size_t i = 0; i < MAX_SENSOR_COUNT; ++i) {
      ASensorEventQueue_disableSensor (m->sensorQueue, m->sensor_data[i].sensor);
      m->sensor_data[i].value[0] = 0;
      m->sensor_data[i].value[1] = 0;
      m->sensor_data[i].value[2] = 0;
    }
    m->flags &= ~INPUT_SENSOR_ENABLED;
  } else if (s && !(m->flags & INPUT_SENSOR_ENABLED)) {
    // attach
    for (size_t i = 0; i < MAX_SENSOR_COUNT; ++i) {
      ASensorEventQueue_enableSensor (m->sensorQueue, m->sensor_data[i].sensor);
      ASensorEventQueue_setEventRate (m->sensorQueue, m->sensor_data[i].sensor, SENSOR_EVENT_RATE);
    }
    m->flags |= INPUT_SENSOR_ENABLED;
    android_inputManager_processSensor (0, 0, m);
  }
}
void android_inputManager_term () {
  if (m->inputQueue)
    AInputQueue_detachLooper (m->inputQueue);
  for (size_t i = 0; i < MAX_SENSOR_COUNT; ++i) {
    ASensorEventQueue_disableSensor (m->sensorQueue, m->sensor_data[i].sensor);
    m->sensor_data[i].value[0] = 0;
    m->sensor_data[i].value[1] = 0;
    m->sensor_data[i].value[2] = 0;
  }
  free_mem (m);
}