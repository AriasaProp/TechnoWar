#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <sys/resource.h>
#include <sys/time.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

#include "api_graphics/opengles_graphics.h"
#include "engine.h"
#include "utils/uistage.h"

// macro

#define MAX_TOUCH_POINTERS_COUNT 30
#define MAX_KEYCODE_COUNT 0xffff
#define MAX_SENSOR_COUNT 0xffff
// ~60 Hz
#define SENSOR_EVENT_RATE 1666.666667

/*** variable sections ***/

// all

// assets
static AAssetManager *assetmanager;
static AAsset *AAssetBuffer[256]{};
static uint8_t aasset_id_last = 0;
// info
static int sdk_version;
// input
static struct {
  int32_t id, button;
  float xs, ys;
  float x, y;
} android_android_input_pointer_cache[MAX_TOUCH_POINTERS_COUNT]{};
static int32_t android_input_key_events[MAX_KEYCODE_COUNT]{};
static AInputQueue *android_input_queue;
static AInputEvent *android_input_event;

static struct {
	engine_sensor_value value;
	ASensor *sensor;
} android_input_sensor_cache[MAX_SENSOR_COUNT]{};
static ASensorEvent android_input_sensor_event[2];
static ASensorManager *android_input_sensorManager;
static ASensorEventQueue *android_input_sensorEventQueue;
static int android_input_sensor_enabled;

// graphics
float android_graphics_cur_safe_insets[4];


/*** functions ***/

// all

// assets
static int *android_assets_open_asset (const char *filename) {
  AAssetBuffer[aasset_id_last] = AAssetManager_open (assetmanager, filename, AASSET_MODE_STREAMING);
  return (AAssetBuffer[aasset_id_last]) ? (int)aasset_id_last++ : -1;
}
static int android_assets_read (const int a, void *b, size_t l) {
  return AAsset_read (AAssetBuffer[a], b, l);
}
static void android_assets_seek (const int a, int l) {
  AAsset_seek (AAssetBuffer[a], l, SEEK_CUR);
}
static int android_assets_remain (const int a) {
  return AAsset_getRemainingLength (AAssetBuffer[a]);
}
static void android_assets_close_asset (const int a) {
  AAsset_close (AAssetBuffer[a]);
  AAssetBuffer[a] = nullptr;
}
static void *android_assets_asset_buffer (const char *filename, size_t *o) {
  AAsset *asset = AAssetManager_open (assetmanager, filename, AASSET_MODE_BUFFER);
  size_t *outLen = (o ? o : malloc(sizeof(size_t)));
  *outLen = AAsset_getLength (asset);
  void *result = malloc (*outLen);
  memcpy (result, AAsset_getBuffer (asset), *outLen);
  AAsset_close (asset);
  if (!o) free (outLen);
  return result;
}

// info
static const char *android_info_get_platform_info () {
  static std_string tmp;
  tmp = "Android SDK " + std_to_string (sdk_version);
  return tmp.c_str ();
}
static long android_info_memory () {
  static struct rusage usage;
  if (getrusage (RUSAGE_SELF, &(usage)) < 0)
    return -1;
  return usage.ru_maxrss;
}

// input
static int inline android_input_button_type (int32_t btn) {
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
static int android_input_process_sensor_event (int, int) {
  if (android_input_sensor_enabled) return 1;
  size_t i, j;
  uint8_t asensor_type;
  while ((i = ASensorEventQueue_getEvents (android_input_sensorEventQueue, android_input_sensor_event, MAX_SENSOR_COUNT)) > 0) {
    for (j = 0; j < i; ++j) {
      ASensorEvent e = android_input_sensor_event[j];
      switch (e.type) {
      case ASENSOR_TYPE_ACCELEROMETER:
      	asensor_type = SENSOR_TYPE_ACCELEROMETER;
        break;
      case ASENSOR_TYPE_GYROSCOPE:
      	asensor_type = SENSOR_TYPE_GYROSCOPE;
        break;
      default:
        continue;
      }
      android_input_sensor_cache[asensor_type].value = {
    		.x = e.acceleration.x / 2.f + 0.5f,
    		.y = e.acceleration.y / 2.f + 0.5f,
    		.z = e.acceleration.z / 2.f + 0.5f
      };
    }
  }
  return 1;
}
static int android_input_process_input (int, int, void *) {
  if (!android_input_queue) return 1;
  if (AInputQueue_getEvent (android_input_queue, &android_input_event) < 0) return 1;
  if (AInputQueue_preDispatchEvent (android_input_queue, android_input_event)) return 1;
  int32_t handled = 0;
  switch (AInputEvent_getType (android_input_event)) {
  case AINPUT_EVENT_TYPE_KEY: {
    int32_t keyCode = AKeyEvent_getKeyCode (android_input_event);
    switch (AKeyEvent_getAction (android_input_event)) {
    case AKEY_EVENT_ACTION_DOWN:
    	// put keycode to list
    	for (size_t i = 0; i < MAX_KEYCODE_COUNT; ++i) {
    		if (android_input_key_events[i]) {
    			if (keyCode == android_input_key_events[i]) break;
    		} else {
    			android_input_key_events[i] = keyCode;
    			break;
    		}
    	}
    	break;
    case AKEY_EVENT_ACTION_UP:
    	// remove keycode to list
    	for (size_t i = 0; i < MAX_KEYCODE_COUNT; ++i) {
    		if (android_input_key_events[i]) {
    			if (keyCode == android_input_key_events[i]) {
    				if (i < MAX_KEYCODE_COUNT - 1) {
    					memmove(android_input_key_events + i, android_input_key_events + i + 1, MAX_KEYCODE_COUNT - i - 1);
  						android_input_key_events[MAX_KEYCODE_COUNT - 1] = 0;
    				} else {
  						android_input_key_events[i] = 0;
    				}
    				break;
    			}
    		} else break;
    	}
      break;
    case AKEY_EVENT_ACTION_MULTIPLE:
      break;
    default:
      break;
    }
    break;
  }
  case AINPUT_EVENT_TYPE_MOTION: {
    const int32_t motion = AMotionEvent_getAction (android_input_event);
    switch (motion & AMOTION_EVENT_ACTION_MASK) {
    case AMOTION_EVENT_ACTION_POINTER_DOWN:
    case AMOTION_EVENT_ACTION_DOWN:
      if (!AMotionEvent_getEdgeFlags (android_input_event)) {
        const uint8_t pointer_index = (motion & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        float x = AMotionEvent_getX (android_input_event, pointer_index);
        float y = AMotionEvent_getY (android_input_event, pointer_index);
        engine_graphics_to_flat_coordinate (x, y);
        int32_t b = android_button_type (AMotionEvent_getButtonState (android_input_event));
        android_input_pointer_cache[pointer_index] = {
	        .id = AMotionEvent_getPointerId (android_input_event, pointer_index),
	        .button = b, .x = x, .y = y, .xs = x, .ys = y
        };
        uistage_touchDown (x, y, pointer_index, button);
      }
      break;
    case AMOTION_EVENT_ACTION_MOVE:
      for (size_t i = 0, j = AMotionEvent_getPointerCount (android_input_event); (i < j); ++i) {
        const int32_t pointer_id = AMotionEvent_getPointerId (android_input_event, i);
        for (size_t k = 0; k < MAX_TOUCH_POINTERS_COUNT; ++k) {
        	if (android_input_pointer_cache[k] && android_input_pointer_cache[k].id == pointer_id) {
		        android_input_pointer_cache[k].x = AMotionEvent_getX (android_input_event, i);
		        android_input_pointer_cache[k].y = AMotionEvent_getY (android_input_event, i);
		        engine_graphics_to_flat_coordinate (android_input_pointer_cache[k].x, android_input_pointer_cache[k].y);
		        uistage_touchMove (android_input_pointer_cache[k].x, android_input_pointer_cache[k].y, android_input_pointer_cache[k].xs - android_input_pointer_cache[k].x, android_input_pointer_cache[k].ys - android_input_pointer_cache[k].y, i, android_input_pointer_cache[k].button);
        		break;
        	}
        }
      }
      break;
    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_POINTER_UP:
    case AMOTION_EVENT_ACTION_OUTSIDE: {
      const uint8_t pointer_index = (motion & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
      const int32_t pointer_id = AMotionEvent_getPointerId (android_input_event, pointer_index);
      for (size_t i = 0; i < MAX_TOUCH_POINTERS_COUNT; ++i) {
        if (android_input_pointer_cache[i].id == pointer_id) {
	        float x = AMotionEvent_getX (android_input_event, pointer_index);
	        float y = AMotionEvent_getY (android_input_event, pointer_index);
	        engine_graphics_to_flat_coordinate (x, y);
	        uistage_touchUp (x, y, i, android_input_pointer_cache[i].button);
	        android_input_pointer_cache[i] = 0;
	        break;
        }
      }
    } break;
    case AMOTION_EVENT_ACTION_CANCEL: {
      const uint8_t pointer_index = (motion & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
      const int32_t pointer_id = AMotionEvent_getPointerId (android_input_event, pointer_index);
      for (size_t i = 0; i < MAX_TOUCH_POINTERS_COUNT; ++i) {
        if (android_input_pointer_cache[i].id == pointer_id) {
	        float x = AMotionEvent_getX (android_input_event, pointer_index);
	        float y = AMotionEvent_getY (android_input_event, pointer_index);
	        engine_graphics_to_flat_coordinate (x, y);
	        uistage_touchCanceled (x, y, i, android_input_pointer_cache[i].button);
	        android_input_pointer_cache[i] = 0;
	        break;
        }
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
  AInputQueue_finishEvent (android_input_queue, android_input_event, handled);
  return 1;
}

// funct
static engine_sensor_value android_input_getSensorValue (const char *sensor_name) { return sensors[sensor_name]; }
static void android_input_getPointerPos (float *out, size_t p = 0) {
  out[0] = android_input_pointer_cache[p].x;
  out[1] = android_input_pointer_cache[p].y;
}
static void android_input_getPointerDelta (float *out, size_t p = 0) {
  out[0] = android_input_pointer_cache[p].x - android_input_pointer_cache[p].xs;
  out[1] = android_input_pointer_cache[p].y - android_input_pointer_cache[p].ys;
}
static int android_input_onTouched () {
  
  for (size_t i = 0; i < MAX_TOUCH_POINTERS_COUNT; ++i) {
    if (android_input_pointer_cache[i])
      return 1;
  }
  return 0;
}
static int android_input_isTouched (size_t p = 0) { return android_input_pointer_cache[p] != 0; }
static float android_input_getPressure (size_t) { return 0; }
static int android_input_isButtonPressed (int) { return 0; }
static int android_input_isKeyPressed (int key) {
	int32_t evaluated;
	size_t i;
	do {
		evaluated = android_input_key_events[i];
		if (evaluated == key) return 1;
	} while (evaluated && (i++ < MAX_KEYCODE_COUNT));
	return 0;
}
static void android_input_process_event () {
   // update input
}
// android funct
void android_input_set_input_queue (ALooper *looper, AInputQueue *i) {
  if (android_input_queue)
    AInputQueue_detachLooper (android_input_queue);
  android_input_queue = i;
  if (android_input_queue)
    AInputQueue_attachLooper (android_input_queue, looper, ALOOPER_POLL_CALLBACK, android_input_process_input, (void *)minput);
}
void android_input_attach_sensor () {
  if (android_input_sensor_enabled) return;
  for (size_t i = 0; i < MAX_SENSOR_COUNT; ++i) {
	  ASensorEventQueue_enableSensor (android_input_sensorEventQueue, android_input_sensor_cache[i].sensor);
	  ASensorEventQueue_setEventRate (android_input_sensorEventQueue, android_input_sensor_cache[i].sensor, SENSOR_EVENT_RATE);
  }
  sensor_enabled = 1;
  process_sensor_event (0, 0, 0);
}
void android_input_detach_sensor () {
  if (!sensor_enabled) return;
  ASensorEventQueue_disableSensor (sensorEventQueue, accelerometerSensor);
  for (size_t i = 0; i < MAX_SENSOR_COUNT; ++i) {
	  ASensorEventQueue_disableSensor (android_input_sensorEventQueue, android_input_sensor_cache[i].sensor);
	  android_input_sensor_cache[i].value = 0;
  }
  sensor_enabled = 0;
}


// graphics
// for android only
void (*android_graphics_onWindowChange) (ANativeWindow *);
void (*android_graphics_onWindowResizeDisplay) ();
void (*android_graphics_onWindowResize) ();
int (*android_graphics_preRender) ();
void (*android_graphics_postRender) (int);

// set engine
void init_engine (AAssetManager *mngr, int sdk, ALooper *looper) {
  
  // set engine_assets
  engine_assets_open_asset = android_assets_open_asset;
  engine_assets_read = android_assets_read;
  engine_assets_seek = android_assets_seek;
  engine_assets_remain = android_assets_remain;
  engine_assets_close_asset = android_assets_close_asset;
  engine_assets_asset_buffer = android_assets_asset_buffer;
  aasset_id_last = 0;
  assetmanager = mngr;
  
  
  // set engine_info
  engine_info_get_platform_info = android_info_get_platform_info;
  engine_info_memory = android_info_memory;
  sdk_version = sdk;

  // set engine_input
  engine_input_getSensorValue = android_input_getSensorValue;
  engine_input_getPointerPos = android_input_getPointerPos;
  engine_input_onTouched = android_input_onTouched;
  engine_input_isTouched = android_input_isTouched;
  engine_input_getPressure = android_input_getPressure;
  engine_input_isButtonPressed = android_input_isButtonPressed;
  engine_input_isKeyPressed = android_input_isKeyPressed;
  engine_input_process_event = android_input_process_event;
  
  android_input_sensor_cache = 0;
  android_input_sensorManager = ASensorManager_getInstance ();
  android_input_sensor_cache[SENSOR_TYPE_ACCELEROMETER].sensor = ASensorManager_getDefaultSensor (android_input_sensorManager, ASENSOR_TYPE_ACCELEROMETER);
  android_input_sensor_cache[SENSOR_TYPE_GYROSCOPE].sensor = ASensorManager_getDefaultSensor (android_input_sensorManager, ASENSOR_TYPE_GYROSCOPE);
  android_input_sensorEventQueue = ASensorManager_createEventQueue (android_input_sensorManager, looper, ALOOPER_POLL_CALLBACK, process_sensor_event, 0);
  
  // set engine_graphics from opengles library
  opengles_graphics_init ();
}
// unset engine
void term_engine () {

  // unset engine_assets
  engine_assets_open_asset = nullptr;
  engine_assets_read = nullptr;
  engine_assets_seek =  nullptr;
  engine_assets_remain = nullptr;
  engine_assets_close_asset = nullptr;
  engine_assets_asset_buffer = nullptr;
  aasset_id_last = 0;
  
  // unset engine_info
  engine_info_get_platform_info = nullptr;
  engine_info_memory = nullptr;
  
  // unset engine_input
  engine_input_getSensorValue = nullptr;
  engine_input_getPointerPos = nullptr;
  engine_input_getPointerDelta = nullptr;
  engine_input_onTouched = nullptr;
  engine_input_isTouched = nullptr;
  engine_input_getPressure = nullptr;
  engine_input_isButtonPressed = nullptr;
  engine_input_isKeyPressed = nullptr;
  engine_input_process_event = nullptr;
  android_input_detach_sensor ();
  if (android_input_queue)
    AInputQueue_detachLooper (android_input_queue);
  sensors.clear ();
  key_pressed.clear ();

  // unset engine_graphics from opengles library
  opengles_graphics_term ();
}