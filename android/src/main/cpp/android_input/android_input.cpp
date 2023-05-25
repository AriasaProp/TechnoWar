#include "android_input.hpp"

#include <cstring>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct touch_pointer {
  bool active;
  float xs, ys;
  float x, y;
};
struct key_event {
  int keyCode;
  enum event {
    KEY_UP,
    KEY_DOWN
  } type;
};
struct ainput {
  touch_pointer input_pointer_cache[MAX_TOUCH_POINTERS_COUNT]{};
  std::unordered_set<key_event *> key_events;
  std::unordered_map<std::string, engine::sensor_value> sensors;
  std::unordered_set<int> justKey_pressed;
  std::unordered_set<int> key_pressed;
  AInputQueue *inputQueue;
  ASensorEvent s_event[2];
  ASensorManager *sensorManager;
  const ASensor *accelerometerSensor;
  const ASensor *gyroscopeSensor;
  ASensorEventQueue *sensorEventQueue;
  // cache only
  AInputEvent *i_event;
  bool sensor_enabled;
};
// funct
engine::sensor_value android_input::getSensorValue (const char *sensor_name) const { return minput->sensors[sensor_name]; }
int android_input::getX (unsigned int p = 0) { return minput->input_pointer_cache[p].x; }
int android_input::getDeltaX (unsigned int p = 0) { return minput->input_pointer_cache[p].x - minput->input_pointer_cache[p].xs; }
int android_input::getY (unsigned int p = 0) { return minput->input_pointer_cache[p].y; }
int android_input::getDeltaY (unsigned int p = 0) { return minput->input_pointer_cache[p].y - minput->input_pointer_cache[p].ys; }
bool android_input::justTouched () { return false; }
bool android_input::isTouched (unsigned int p = 0) { return minput->input_pointer_cache[p].active; }
float android_input::getPressure (unsigned int) { return false; }
bool android_input::isButtonPressed (int) { return false; }
bool android_input::isButtonJustPressed (int) { return false; }
bool android_input::isKeyPressed (int key) { return minput->key_pressed.find (key) != minput->key_pressed.end (); }
bool android_input::isKeyJustPressed (int key) { return minput->justKey_pressed.find (key) != minput->justKey_pressed.end (); }
void android_input::process_event () {
  if (minput->justKey_pressed.size () > 0) {
    minput->justKey_pressed.clear ();
  }
  for (const key_event *k : minput->key_events) {
    switch (k->type) {
    case key_event::event::KEY_UP: {
    } break;
    case key_event::event::KEY_DOWN: {
      std::unordered_set<int>::iterator key = minput->justKey_pressed.find (k->keyCode);
      if (key != minput->justKey_pressed.end ()) {
        minput->justKey_pressed.insert (k->keyCode);
      }
    } break;
    }
    delete k;
  }
  minput->key_events.clear ();
}
void android_input::set_input_queue (ALooper *looper, AInputQueue *i) {
  if (minput->inputQueue)
    AInputQueue_detachLooper (minput->inputQueue);
  minput->inputQueue = i;
  if (minput->inputQueue)
    AInputQueue_attachLooper (minput->inputQueue, looper, 2, NULL, nullptr);
}
void android_input::process_input () {
  if (!minput->inputQueue) return;
  if (AInputQueue_getEvent (minput->inputQueue, &minput->i_event) < 0) return;
  if (AInputQueue_preDispatchEvent (minput->inputQueue, minput->i_event) != 0) return;
  int32_t handled = 0;
  switch (AInputEvent_getType (minput->i_event)) {
  case AINPUT_EVENT_TYPE_KEY: {
    int32_t keyCode = AKeyEvent_getKeyCode (minput->i_event);
    std::unordered_set<int>::iterator key = minput->key_pressed.find (keyCode);
    switch (AKeyEvent_getAction (minput->i_event)) {
    case AKEY_EVENT_ACTION_DOWN:
      if (key != minput->key_pressed.end ()) {
        minput->key_pressed.insert (keyCode);
      }
      minput->key_events.insert (new key_event{.keyCode = keyCode, .type = key_event::event::KEY_DOWN});
      break;
    case AKEY_EVENT_ACTION_UP:
      if (key != minput->key_pressed.end ()) {
        minput->key_pressed.erase (key);
      }
      minput->key_events.insert (new key_event{.keyCode = keyCode, .type = key_event::event::KEY_UP});
      break;
    case AKEY_EVENT_ACTION_MULTIPLE:
      break;
    }
    break;
  }
  case AINPUT_EVENT_TYPE_MOTION: {
    const int32_t motion = AMotionEvent_getAction (minput->i_event);
    switch (motion & AMOTION_EVENT_ACTION_MASK) {
    case AMOTION_EVENT_ACTION_POINTER_DOWN:
    case AMOTION_EVENT_ACTION_DOWN:
      if (AMotionEvent_getEdgeFlags (minput->i_event) == 0) {
        const int8_t pointer_index = (motion & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        if (pointer_index >= MAX_TOUCH_POINTERS_COUNT)
          break;
        touch_pointer &ip = minput->input_pointer_cache[pointer_index];
        ip.active = true;
        ip.xs = ip.x = AMotionEvent_getX (minput->i_event, pointer_index);
        ip.ys = ip.y = AMotionEvent_getY (minput->i_event, pointer_index);
      }
      break;
    case AMOTION_EVENT_ACTION_MOVE:
      for (size_t i = 0, j = AMotionEvent_getPointerCount (minput->i_event); (i < j) && (i < MAX_TOUCH_POINTERS_COUNT); i++) {
        touch_pointer &ip = minput->input_pointer_cache[i];
        if (!ip.active) continue;
        ip.x = AMotionEvent_getX (minput->i_event, i);
        ip.y = AMotionEvent_getY (minput->i_event, i);
      }
      break;
    case AMOTION_EVENT_ACTION_POINTER_UP:
    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_OUTSIDE: {
      const int8_t pointer_index = (motion & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
      if (pointer_index >= MAX_TOUCH_POINTERS_COUNT)
        break;
      touch_pointer &ip = minput->input_pointer_cache[pointer_index];
      if (!ip.active) break;
      ip.active = false;
      ip.x = AMotionEvent_getX (minput->i_event, pointer_index);
      ip.y = AMotionEvent_getY (minput->i_event, pointer_index);
    } break;
    case AMOTION_EVENT_ACTION_CANCEL:
      for (touch_pointer &input_pointer_item : minput->input_pointer_cache)
        input_pointer_item = touch_pointer{};
      break;
    case AMOTION_EVENT_ACTION_SCROLL:
    case AMOTION_EVENT_ACTION_HOVER_ENTER:
    case AMOTION_EVENT_ACTION_HOVER_MOVE:
    case AMOTION_EVENT_ACTION_HOVER_EXIT:
      break;
    }
    handled = 1;
  } break;
  }
  AInputQueue_finishEvent (minput->inputQueue, minput->i_event, handled);
}
void android_input::attach_sensor () {
  if (minput->sensor_enabled) return;
  ASensorEventQueue_enableSensor (minput->sensorEventQueue, minput->accelerometerSensor);
  ASensorEventQueue_setEventRate (minput->sensorEventQueue, minput->accelerometerSensor, 50000 / 3);
  ASensorEventQueue_enableSensor (minput->sensorEventQueue, minput->gyroscopeSensor);
  ASensorEventQueue_setEventRate (minput->sensorEventQueue, minput->gyroscopeSensor, 50000 / 3);
  minput->sensor_enabled = true;
}
void android_input::process_sensor () {
  if (!minput->sensor_enabled) return;
  unsigned int i, j;
  while ((i = ASensorEventQueue_getEvents (minput->sensorEventQueue, minput->s_event, 2)) > 0) {
    for (j = 0; j < i; j++) {
      ASensorEvent &e = minput->s_event[j];
      switch (e.type) {
      case ASENSOR_TYPE_ACCELEROMETER: {
        engine::sensor_value &t = minput->sensors["accelerometer"];
        t.x = e.acceleration.x / 2.f + 0.5f;
        t.y = e.acceleration.y / 2.f + 0.5f;
        t.z = e.acceleration.z / 2.f + 0.5f;
        break;
      }
      case ASENSOR_TYPE_GYROSCOPE: {
        engine::sensor_value &t = minput->sensors["gyroscope"];
        t.x = e.acceleration.x / 2.f + 0.5f;
        t.y = e.acceleration.y / 2.f + 0.5f;
        t.z = e.acceleration.z / 2.f + 0.5f;
        break;
      }
      default:
        break;
      }
    }
  }
}
void android_input::detach_sensor () {
  if (!minput->sensor_enabled) return;
  for (auto i = minput->sensors.begin (), n = minput->sensors.end (); i != n; i++)
    i->second = engine::sensor_value{};
  ASensorEventQueue_disableSensor (minput->sensorEventQueue, minput->accelerometerSensor);
  ASensorEventQueue_disableSensor (minput->sensorEventQueue, minput->gyroscopeSensor);
  minput->sensor_enabled = false;
}
android_input::android_input (ALooper *looper) {
  minput = new ainput{};
  minput->sensors["accelerometer"] = {0, 0, 0};
  minput->sensors["gyroscope"] = {0, 0, 0};
  minput->sensorManager = ASensorManager_getInstance ();
  minput->accelerometerSensor = ASensorManager_getDefaultSensor (minput->sensorManager, ASENSOR_TYPE_ACCELEROMETER);
  minput->gyroscopeSensor = ASensorManager_getDefaultSensor (minput->sensorManager, ASENSOR_TYPE_GYROSCOPE);
  minput->sensorEventQueue = ASensorManager_createEventQueue (minput->sensorManager, looper, 3, NULL, nullptr);
  // input
  engine::inpt = this;
}
android_input::~android_input () {
  detach_sensor ();
  if (minput->inputQueue)
    AInputQueue_detachLooper (minput->inputQueue);
  minput->sensors.clear ();
  minput->key_pressed.clear ();
  delete minput;
  engine::inpt = nullptr;
}
