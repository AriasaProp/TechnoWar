#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <sys/resource.h>
#include <sys/time.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

#include "api_graphics/opengles_graphics.hpp"
#include "engine.hpp"
#include "utils/uistage.hpp"

namespace android_asset {
static AAssetManager *assetmanager;

struct a_asset : public engine::asset_core {
  AAsset *asset;
  a_asset (AAsset *a) : asset (a) {}
  size_t read (void *buff, size_t len) override {
    return AAsset_read (asset, buff, len);
  }
  void seek (int n) override {
    AAsset_seek (asset, n, SEEK_CUR);
  }
  bool eof () override {
    return AAsset_getRemainingLength (asset) <= 0;
  }
  ~a_asset () {
    AAsset_close (asset);
  }
};
static engine::asset_core *open_asset (const char *filename) {
  return new a_asset (AAssetManager_open (assetmanager, filename, AASSET_MODE_STREAMING));
}
static void *asset_buffer (const char *filename, unsigned int *o) {
  AAsset *asset = AAssetManager_open (assetmanager, filename, AASSET_MODE_BUFFER);
  unsigned int *outLen = (o ? o : new unsigned int);
  *outLen = AAsset_getLength (asset);
  void *result = malloc (*outLen);
  memcpy (result, AAsset_getBuffer (asset), *outLen);
  AAsset_close (asset);
  if (!o) delete outLen;
  return result;
}
static void init (AAssetManager *mngr) {
  assetmanager = mngr;

  // set engine;:assets
  engine::assets::open_asset = open_asset;
  engine::assets::asset_buffer = asset_buffer;
}
static void term () {

  // unset engine;:assets
  engine::assets::open_asset = nullptr;
  engine::assets::asset_buffer = nullptr;
}
} // namespace android_asset

namespace android_info {

static int sdk_version;

static const char *get_platform_info () {
  static std::string tmp;
  tmp = "Android SDK " + std::to_string (sdk_version);
  return tmp.c_str ();
}

static long memory () {
  static struct rusage usage;
  if (getrusage (RUSAGE_SELF, &(usage)) < 0)
    return -1;
  return usage.ru_maxrss;
}

static void init (int sdk) {
  sdk_version = sdk;

  // set engine::info
  engine::info::get_platform_info = get_platform_info;
  engine::info::memory = memory;
}
static void term () {

  // set engine::info
  engine::info::get_platform_info = nullptr;
  engine::info::memory = nullptr;
}
} // namespace android_info

namespace android_input {
#define MAX_TOUCH_POINTERS_COUNT 30
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
static struct ainput {
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
} *minput = nullptr;
//
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
static int process_sensor_event (int, int, void *data) {
  ainput *m = (ainput *)data;
  if (!m->sensor_enabled) return 1;
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
  return 1;
}
static int process_input (int, int, void *data) {
  ainput *m = (ainput *)data;
  if (!m->inputQueue) return 1;
  if (AInputQueue_getEvent (m->inputQueue, &m->i_event) < 0) return 1;
  if (AInputQueue_preDispatchEvent (m->inputQueue, m->i_event) != 0) return 1;
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
  AInputQueue_finishEvent (m->inputQueue, m->i_event, handled);
  return 1;
}

// funct
static engine::sensor_value getSensorValue (const char *sensor_name) const { return minput->sensors[sensor_name]; }
static void getPointerPos (float *out, unsigned int p = 0) {
  out[0] = minput->input_pointer_cache[p].x;
  out[1] = minput->input_pointer_cache[p].y;
}
static void getPointerDelta (float *out, unsigned int p = 0) {
  out[0] = minput->input_pointer_cache[p].x - minput->input_pointer_cache[p].xs;
  out[1] = minput->input_pointer_cache[p].y - minput->input_pointer_cache[p].ys;
}
static bool justTouched () { return false; }
static bool onTouched () {
  for (size_t i = 0; i < MAX_TOUCH_POINTERS_COUNT; ++i) {
    if (minput->input_pointer_cache[i].active)
      return true;
  }
  return false;
}
static bool isTouched (unsigned int p = 0) { return minput->input_pointer_cache[p].active; }
static float getPressure (unsigned int) { return false; }
static bool isButtonPressed (int) { return false; }
static bool isButtonJustPressed (int) { return false; }
static bool isKeyPressed (int key) { return minput->key_pressed.find (key) != minput->key_pressed.end (); }
static bool isKeyJustPressed (int key) { return minput->justKey_pressed.find (key) != minput->justKey_pressed.end (); }
static void process_event () {
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

// android funct
void set_input_queue (ALooper *looper, AInputQueue *i) {
  if (minput->inputQueue)
    AInputQueue_detachLooper (minput->inputQueue);
  minput->inputQueue = i;
  if (minput->inputQueue)
    AInputQueue_attachLooper (minput->inputQueue, looper, ALOOPER_POLL_CALLBACK, process_input, (void *)minput);
}

void attach_sensor () {
  if (minput->sensor_enabled) return;
  ASensorEventQueue_enableSensor (minput->sensorEventQueue, minput->accelerometerSensor);
  ASensorEventQueue_setEventRate (minput->sensorEventQueue, minput->accelerometerSensor, 50000 / 3);
  ASensorEventQueue_enableSensor (minput->sensorEventQueue, minput->gyroscopeSensor);
  ASensorEventQueue_setEventRate (minput->sensorEventQueue, minput->gyroscopeSensor, 50000 / 3);
  minput->sensor_enabled = true;
  process_sensor_event (0, 0, (void *)minput);
}
void detach_sensor () {
  if (!minput->sensor_enabled) return;
  for (auto i = minput->sensors.begin (), n = minput->sensors.end (); i != n; i++)
    i->second = engine::sensor_value{};
  ASensorEventQueue_disableSensor (minput->sensorEventQueue, minput->accelerometerSensor);
  ASensorEventQueue_disableSensor (minput->sensorEventQueue, minput->gyroscopeSensor);
  minput->sensor_enabled = false;
}

static void init (ALooper *looper) {
  minput = new ainput{};
  minput->sensors["accelerometer"] = {0, 0, 0};
  minput->sensors["gyroscope"] = {0, 0, 0};
  minput->sensorManager = ASensorManager_getInstance ();
  minput->accelerometerSensor = ASensorManager_getDefaultSensor (minput->sensorManager, ASENSOR_TYPE_ACCELEROMETER);
  minput->gyroscopeSensor = ASensorManager_getDefaultSensor (minput->sensorManager, ASENSOR_TYPE_GYROSCOPE);
  minput->sensorEventQueue = ASensorManager_createEventQueue (minput->sensorManager, looper, ALOOPER_POLL_CALLBACK, process_sensor_event, (void *)minput);

  // set engine::input
  engine::input::getSensorValue = getSensorValue;
  engine::input::getPointerPos = getPointerPos;
  engine::input::getPointerDelta = getPointerDelta;
  engine::input::justTouched = justTouched;
  engine::input::onTouched = onTouched;
  engine::input::isTouched = isTouched;
  engine::input::getPressure = getPressure;
  engine::input::isButtonPressed = isButtonPressed;
  engine::input::isButtonJustPressed = isButtonJustPressed;
  engine::input::isKeyPressed = isKeyPressed;
  engine::input::isKeyJustPressed = isKeyJustPressed;
  engine::input::process_event = process_event;
}
static void term () {
  detach_sensor ();
  if (minput->inputQueue)
    AInputQueue_detachLooper (minput->inputQueue);
  minput->sensors.clear ();
  minput->key_pressed.clear ();
  delete minput;
  minput = nullptr;

  // unset engine::input
  engine::input::getSensorValue = nullptr;
  engine::input::getPointerPos = nullptr;
  engine::input::getPointerDelta = nullptr;
  engine::input::justTouched = nullptr;
  engine::input::onTouched = nullptr;
  engine::input::isTouched = nullptr;
  engine::input::getPressure = nullptr;
  engine::input::isButtonPressed = nullptr;
  engine::input::isButtonJustPressed = nullptr;
  engine::input::isKeyPressed = nullptr;
  engine::input::isKeyJustPressed = nullptr;
  engine::input::process_event = nullptr;
}
#undef MAX_TOUCH_POINTERS_COUNT
} // namespace android_input

namespace android_graphics {
float cur_safe_insets[4];
// android
void (*onWindowChange) (ANativeWindow *);
void (*onWindowResize) (unsigned char);
bool (*preRender) ();
void (*postRender) (bool);

// api
// opengles
void init () {
  opengles_graphics::init ();
}
void term () {
  opengles_graphics::term ();
}
} // namespace android_graphics
// at last

// set engine
void init_engine (AAssetManager *mngr, int sdk, ALooper *looper) {
  android_asset::init (mngr);
  android_info::init (sdk);
  android_input::init (looper);
  android_graphics::init ();
}
// unset engine
void term_engine () {

  android_asset::term ();
  android_info::term ();
  android_input::term ();
  android_graphics::term ();
}