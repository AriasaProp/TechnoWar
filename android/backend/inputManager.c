#include <android/input.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

#include "engine.h"
#include "util.h"
// ~60 Hz
#define SENSOR_EVENT_RATE 1667

enum inputManagerState {
  INPUT_SENSOR_ENABLED = 1,
};
#define MAX_SENSOR_COUNT 3
#define MAX_POINTER      10
enum {
  SENSOR_ACCELEROMETER = 0,
  SENSOR_GYROSCOPE = 1,
  SENSOR_MAGNETIC_FIELD = 2,
};

static struct android_inputManager {
  ALooper *looper;
  AInputQueue *inputQueue;

  ASensorManager *sensorMngr;
  ASensorEventQueue *sensorQueue;
  struct
  {
    const ASensor *sensor;
    float value[3];
  } sensor_data[MAX_SENSOR_COUNT];

  int flags;
  vec2 pointers[MAX_POINTER];
} *m = NULL;

// core implementation
static vec2 getTouch(size_t p) {
  return m->pointers[p];
}

// processing input
static int androidInput_processInput(int UNUSED_ARG(fd), int UNUSED_ARG(event), void *UNUSED_ARG(data)) {
  static AInputEvent *outEvent;
  if (
    !m->inputQueue ||
    (AInputQueue_getEvent(m->inputQueue, &outEvent) < 0) ||
    AInputQueue_preDispatchEvent(m->inputQueue, outEvent))
    return 1;
  int32_t handled = 0;
  if (AInputEvent_getType(outEvent) == AINPUT_EVENT_TYPE_MOTION) {
    m->pointers[0].x = AMotionEvent_getX(outEvent, 0);
    m->pointers[0].y = AMotionEvent_getY(outEvent, 0);
    handled = 1;
  }
  AInputQueue_finishEvent(m->inputQueue, outEvent, handled);
  return 1;
}
static int androidInput_processSensor(int UNUSED_ARG(fd), int UNUSED_ARG(e), void *UNUSED_ARG(data)) {
  ASensorEvent event[MAX_SENSOR_COUNT];
  size_t j;
  while ((j = ASensorEventQueue_getEvents(m->sensorQueue, event, MAX_SENSOR_COUNT)) > 0) {
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

void androidInput_init(void *looper) {
  m = (struct android_inputManager *)calloc(1, sizeof(struct android_inputManager));
  m->looper = (ALooper *)looper;
  m->sensorMngr = ASensorManager_getInstance();
  m->sensor_data[SENSOR_ACCELEROMETER].sensor = ASensorManager_getDefaultSensor(m->sensorMngr, ASENSOR_TYPE_ACCELEROMETER);
  m->sensor_data[SENSOR_GYROSCOPE].sensor = ASensorManager_getDefaultSensor(m->sensorMngr, ASENSOR_TYPE_GYROSCOPE);
  m->sensor_data[SENSOR_MAGNETIC_FIELD].sensor = ASensorManager_getDefaultSensor(m->sensorMngr, ASENSOR_TYPE_MAGNETIC_FIELD);
  m->sensorQueue = ASensorManager_createEventQueue(m->sensorMngr, m->looper, ALOOPER_POLL_CALLBACK, androidInput_processSensor, m);

  get_engine()->i.getTouch = getTouch;
}
void androidInput_createInputQueue(void *queue) {
  AInputQueue_attachLooper((AInputQueue *)queue, m->looper, ALOOPER_POLL_CALLBACK, androidInput_processInput, (void *)m);
  m->inputQueue = (AInputQueue *)queue;
}
void androidInput_destroyInputQueue() {
  AInputQueue_detachLooper(m->inputQueue);
  m->inputQueue = NULL;
}
void androidInput_enableSensor() {
  if (!(m->flags & INPUT_SENSOR_ENABLED)) {
    // attach
    for (size_t i = 0; i < MAX_SENSOR_COUNT; ++i) {
      ASensorEventQueue_enableSensor(m->sensorQueue, m->sensor_data[i].sensor);
      ASensorEventQueue_setEventRate(m->sensorQueue, m->sensor_data[i].sensor, SENSOR_EVENT_RATE);
    }
    m->flags |= INPUT_SENSOR_ENABLED;
    androidInput_processSensor(0, 0, m);
  }
}
void androidInput_disableSensor() {
  if (m->flags & INPUT_SENSOR_ENABLED) {
    // detach
    for (size_t i = 0; i < MAX_SENSOR_COUNT; ++i) {
      ASensorEventQueue_disableSensor(m->sensorQueue, m->sensor_data[i].sensor);
      m->sensor_data[i].value[0] = 0;
      m->sensor_data[i].value[1] = 0;
      m->sensor_data[i].value[2] = 0;
    }
    m->flags &= ~INPUT_SENSOR_ENABLED;
  }
}
void androidInput_term() {
  // disable sensor
  for (size_t i = 0; i < MAX_SENSOR_COUNT; ++i) {
    ASensorEventQueue_disableSensor(m->sensorQueue, m->sensor_data[i].sensor);
    m->sensor_data[i].value[0] = 0;
    m->sensor_data[i].value[1] = 0;
    m->sensor_data[i].value[2] = 0;
  }
  // disable input
  if (m->inputQueue)
    AInputQueue_detachLooper(m->inputQueue);
  free(m);
}