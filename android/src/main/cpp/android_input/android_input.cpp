#include "android_input.hpp"
#include "../api_graphics/android_graphics.hpp"
#include "../utils/uistage.hpp"

#include <cstring>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct touch_pointer {
  bool active;
  int32_t id, button;
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
void android_input::getPointerPos (float *out, unsigned int p = 0) {
  out[0] = minput->input_pointer_cache[p].x;
  out[1] = minput->input_pointer_cache[p].y;
}
void android_input::getPointerDelta (float *out, unsigned int p = 0) {
  out[0] = minput->input_pointer_cache[p].x - minput->input_pointer_cache[p].xs;
  out[1] = minput->input_pointer_cache[p].y - minput->input_pointer_cache[p].ys;
}
bool android_input::justTouched () { return false; }
bool android_input::onTouched () {
  for (size_t i = 0; i < MAX_TOUCH_POINTERS_COUNT; ++i) {
    if (minput->input_pointer_cache[i].active)
      return true;
  }
  return false;
}
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
static int process_input (int, int, void *data) {
  ainput *m = (ainput *)data;
  if (!m->inputQueue) return 0;
  if (AInputQueue_getEvent (m->inputQueue, &m->i_event) < 0) return 0;
  if (AInputQueue_preDispatchEvent (m->inputQueue, m->i_event) != 0) return 0;
  int32_t handled = 0;
  switch (AInputEvent_getType (m->i_event)) {
  case AINPUT_EVENT_TYPE_KEY: {
    int32_t keyCode = AKeyEvent_getKeyCode (m->i_event);
    std::unordered_set<int>::iterator key = m->key_pressed.find (keyCode);
    switch (AKeyEvent_getAction (m->i_event)) {
    case AKEY_EVENT_ACTION_DOWN:
      if (key != m->key_pressed.end ()) {
        m->key_pressed.insert (keyCode);
      }
      m->key_events.insert (new key_event{.keyCode = keyCode, .type = key_event::event::KEY_DOWN});
      break;
    case AKEY_EVENT_ACTION_UP:
      if (key != m->key_pressed.end ()) {
        m->key_pressed.erase (key);
      }
      m->key_events.insert (new key_event{.keyCode = keyCode, .type = key_event::event::KEY_UP});
      break;
    case AKEY_EVENT_ACTION_MULTIPLE:
      break;
    default:
      break;
    }
    break;
  }
  case AINPUT_EVENT_TYPE_MOTION: {
    const int32_t motion = AMotionEvent_getAction (m->i_event);
    switch (motion & AMOTION_EVENT_ACTION_MASK) {
    case AMOTION_EVENT_ACTION_POINTER_DOWN:
    case AMOTION_EVENT_ACTION_DOWN:
      if (AMotionEvent_getEdgeFlags (m->i_event) == 0) {
        const uint8_t pointer_index = (motion & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        touch_pointer &ip = m->input_pointer_cache[pointer_index];
        ip.active = true;
        ip.id = AMotionEvent_getPointerId (m->i_event, pointer_index);
        ip.button = android_button_type (AMotionEvent_getButtonState (m->i_event));
        ip.x = AMotionEvent_getX (m->i_event, pointer_index);
        ip.y = AMotionEvent_getY (m->i_event, pointer_index);
        engine::graph->to_flat_coordinate (ip.x, ip.y);
        ip.xs = ip.x;
        ip.ys = ip.y;
        uistage::touchDown (ip.x, ip.y, pointer_index, ip.button);
      }
      break;
    case AMOTION_EVENT_ACTION_MOVE:
      for (size_t i = 0, j = AMotionEvent_getPointerCount (m->i_event); (i < j) && (i < MAX_TOUCH_POINTERS_COUNT); i++) {
        touch_pointer &ip = m->input_pointer_cache[i];
        const int32_t pointer_id = AMotionEvent_getPointerId (m->i_event, i);
        if (!ip.active || (ip.id != pointer_id)) continue;
        ip.x = AMotionEvent_getX (m->i_event, i);
        ip.y = AMotionEvent_getY (m->i_event, i);
        engine::graph->to_flat_coordinate (ip.x, ip.y);
        uistage::touchMove (ip.x, ip.y, ip.xs - ip.x, ip.ys - ip.y, i, ip.button);
      }
      break;
    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_POINTER_UP:
    case AMOTION_EVENT_ACTION_OUTSIDE: {
      const uint8_t pointer_index = (motion & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
      const int32_t pointer_id = AMotionEvent_getPointerId (m->i_event, pointer_index);
      for (size_t i = 0; i < MAX_TOUCH_POINTERS_COUNT; ++i) {
        touch_pointer &ip = m->input_pointer_cache[i];
        if (ip.id != pointer_id) continue;
        ip.active = false;
        ip.x = AMotionEvent_getX (m->i_event, i);
        ip.y = AMotionEvent_getY (m->i_event, i);
        engine::graph->to_flat_coordinate (ip.x, ip.y);
        uistage::touchUp (ip.x, ip.y, i, ip.button);
        break;
      }
    } break;
    case AMOTION_EVENT_ACTION_CANCEL: {
      const uint8_t pointer_index = (motion & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
      const int32_t pointer_id = AMotionEvent_getPointerId (m->i_event, pointer_index);
      for (size_t i = 0; i < MAX_TOUCH_POINTERS_COUNT; ++i) {
        touch_pointer &ip = m->input_pointer_cache[i];
        if (ip.id != pointer_id) continue;
        ip.active = false;
        ip.x = AMotionEvent_getX (m->i_event, i);
        ip.y = AMotionEvent_getY (m->i_event, i);
        engine::graph->to_flat_coordinate (ip.x, ip.y);
        uistage::touchCanceled (ip.x, ip.y, i, ip.button);
        break;
      }
    } break;
    case AMOTION_EVENT_ACTION_SCROLL:
    case AMOTION_EVENT_ACTION_HOVER_ENTER:
    case AMOTION_EVENT_ACTION_HOVER_MOVE:
    case AMOTION_EVENT_ACTION_HOVER_EXIT:
      break;
    case AMOTION_EVENT_ACTION_BUTTON_PRESS:
    case AMOTION_EVENT_ACTION_BUTTON_RELEASE:
      break;
    }
    handled = 1;
    break;
  }
  }
  AInputQueue_finishEvent (m->inputQueue, minput->i_event, handled);
  return 0;
}
void android_input::set_input_queue (ALooper *looper, AInputQueue *i) {
  if (minput->inputQueue)
    AInputQueue_detachLooper (minput->inputQueue);
  minput->inputQueue = i;
  if (minput->inputQueue)
    AInputQueue_attachLooper (m->inputQueue, looper, ALOOPER_POLL_CALLBACK, process_input, (void *)minput);
}
static int inline android_button_type (int32_t btn) {
  switch (btn) {
  case AMOTION_EVENT_BUTTON_PRIMARY:
  case AMOTION_EVENT_BUTTON_SECONDARY:
    return 0;
  case AMOTION_EVENT_BUTTON_TERTIARY:
    return 1;
  case AMOTION_EVENT_BUTTON_BACK:
    return 2;
  case AMOTION_EVENT_BUTTON_FORWARD:
    return 3;
  case AMOTION_EVENT_BUTTON_STYLUS_PRIMARY:
    return 4;
  case AMOTION_EVENT_BUTTON_STYLUS_SECONDARY:
    return 5;
  default:
    return -1;
  }
}

void android_input::attach_sensor () {
  if (minput->sensor_enabled) return;
  ASensorEventQueue_enableSensor (minput->sensorEventQueue, minput->accelerometerSensor);
  ASensorEventQueue_setEventRate (minput->sensorEventQueue, minput->accelerometerSensor, 50000 / 3);
  ASensorEventQueue_enableSensor (minput->sensorEventQueue, minput->gyroscopeSensor);
  ASensorEventQueue_setEventRate (minput->sensorEventQueue, minput->gyroscopeSensor, 50000 / 3);
  minput->sensor_enabled = true;
  process_sensor_event (0, 0, (void *)minput);
}
void android_input::detach_sensor () {
  if (!minput->sensor_enabled) return;
  for (auto i = minput->sensors.begin (), n = minput->sensors.end (); i != n; i++)
    i->second = engine::sensor_value{};
  ASensorEventQueue_disableSensor (minput->sensorEventQueue, minput->accelerometerSensor);
  ASensorEventQueue_disableSensor (minput->sensorEventQueue, minput->gyroscopeSensor);
  minput->sensor_enabled = false;
}
static int process_sensor_event (int, int, void *data) {
  ainput *m = (ainput *)data;
  if (!m->sensor_enabled) return 0;
  unsigned int i, j;
  while ((i = ASensorEventQueue_getEvents (m->sensorEventQueue, m->s_event, 2)) > 0) {
    for (j = 0; j < i; j++) {
      ASensorEvent &e = m->s_event[j];
      switch (e.type) {
      case ASENSOR_TYPE_ACCELEROMETER: {
        engine::sensor_value &t = m->sensors["accelerometer"];
        t.x = e.acceleration.x / 2.f + 0.5f;
        t.y = e.acceleration.y / 2.f + 0.5f;
        t.z = e.acceleration.z / 2.f + 0.5f;
        break;
      }
      case ASENSOR_TYPE_GYROSCOPE: {
        engine::sensor_value &t = m->sensors["gyroscope"];
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
  return 0;
}
android_input::android_input (ALooper *looper) {
  minput = new ainput{};
  minput->sensors["accelerometer"] = {0, 0, 0};
  minput->sensors["gyroscope"] = {0, 0, 0};
  minput->sensorManager = ASensorManager_getInstance ();
  minput->accelerometerSensor = ASensorManager_getDefaultSensor (minput->sensorManager, ASENSOR_TYPE_ACCELEROMETER);
  minput->gyroscopeSensor = ASensorManager_getDefaultSensor (minput->sensorManager, ASENSOR_TYPE_GYROSCOPE);
  minput->sensorEventQueue = ASensorManager_createEventQueue (minput->sensorManager, looper, ALOOPER_POLL_CALLBACK, process_sensor_event, (void *)minput);
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
