#include "android_input.hpp"

#include <unordered_set>
#include <unordered_map>
#include <cstring>
#include <string>

struct touch_pointer {
	bool active;
	float xs, ys;
	float x, y;
} *input_pointer_cache;
struct key_event {
	int keyCode;
	enum event {
		KEY_UP,
		KEY_DOWN
	} type;
};
struct ainput {
  std::unordered_set<key_event*> key_events;
  std::unordered_map<std::string, float*> _sensor;
  AInputQueue *inputQueue;
  //cache only
  AInputEvent* i_event;
  
  bool sensor_enabled;
};
static std::unordered_set<int> key_pressed;
static std::unordered_set<int> just_key_pressed;
static ASensorEvent *s_event;
static ASensorManager* sensorManager;
static const ASensor* accelerometerSensor;
static const ASensor* gyroscopeSensor;
static ASensorEventQueue* sensorEventQueue;
//funct
float	*android_input::getSensorValue(const char *sensor_name) {	return minput->_sensor[sensor_name]; }
int android_input::getX(unsigned int p = 0) {	return input_pointer_cache[p].x; }
int android_input::getDeltaX(unsigned int p = 0) {	return input_pointer_cache[p].x - input_pointer_cache[p].xs; }
int android_input::getY(unsigned int p = 0) {	return input_pointer_cache[p].y; }
int android_input::getDeltaY(unsigned int p = 0) {	return input_pointer_cache[p].y - input_pointer_cache[p].ys; }
bool android_input::justTouched() {	return false; }
bool android_input::isTouched(unsigned int p = 0) { return input_pointer_cache[p].active; }
float android_input::getPressure(unsigned int) { return false; }
bool android_input::isButtonPressed(int) { return false; }
bool android_input::isButtonJustPressed(int) { return false; }
bool android_input::isKeyPressed(int key) {return key_pressed.find(key) != key_pressed.end(); }
bool android_input::isKeyJustPressed(int key) {return just_key_pressed.find(key) != just_key_pressed.end(); }
void android_input::process_event() {
	if(just_key_pressed.size() > 0) {
		just_key_pressed.clear();
	}
	for (const key_event *k : minput->key_events) {
		switch(k->type) {
			case key_event::event::KEY_UP:{
			}
				break;
			case key_event::event::KEY_DOWN:{
				std::unordered_set<int>::iterator key = just_key_pressed.find(k->keyCode);
				if(key != just_key_pressed.end()) {
					just_key_pressed.insert(k->keyCode);
				}
			}
				break;
		}
		delete k;
	}
	minput->key_events.clear();
}
void android_input::set_input_queue(ALooper *looper, AInputQueue *i) {
	if (minput->inputQueue)
		AInputQueue_detachLooper(minput->inputQueue);
	minput->inputQueue = i;
	if (minput->inputQueue)
		AInputQueue_attachLooper(minput->inputQueue, looper, 2, NULL, nullptr);
}
void android_input::process_input() {
	if (!minput->inputQueue) return;
	if (AInputQueue_getEvent(minput->inputQueue, &minput->i_event) < 0) return;
  if (AInputQueue_preDispatchEvent(minput->inputQueue, minput->i_event) != 0) return;
  int32_t handled = 0;
	switch (AInputEvent_getType(minput->i_event)) {
		case AINPUT_EVENT_TYPE_KEY: {
			int32_t keyCode = AKeyEvent_getKeyCode(minput->i_event);
			switch (AKeyEvent_getAction(minput->i_event)) {
				case AKEY_EVENT_ACTION_DOWN: {
					std::unordered_set<int>::iterator key = key_pressed.find(keyCode);
					if(key != key_pressed.end()) {
						key_pressed.insert(keyCode);
					}
					minput->key_events.insert(new key_event{.keyCode = keyCode,.type = key_event::event::KEY_DOWN});
				}
					break;
				case AKEY_EVENT_ACTION_UP: {
					std::unordered_set<int>::iterator key = key_pressed.find(keyCode);
					if(key != key_pressed.end()) {
						key_pressed.erase(key);
					}
					minput->key_events.insert(new key_event{.keyCode = keyCode,.type = key_event::event::KEY_UP});
				}
					break;
				case AKEY_EVENT_ACTION_MULTIPLE:
					break;
			}
		}
			break;
		case AINPUT_EVENT_TYPE_MOTION: {
			const int32_t motion = AMotionEvent_getAction(minput->i_event);
			switch(motion&AMOTION_EVENT_ACTION_MASK) {
		    case AMOTION_EVENT_ACTION_POINTER_DOWN:
		    case AMOTION_EVENT_ACTION_DOWN:
					if (AMotionEvent_getEdgeFlags(minput->i_event) == 0) {
  					const int8_t pointer_index = (motion&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
  					if (pointer_index >= MAX_TOUCH_POINTERS_COUNT)
  						break;
  					touch_pointer &ip = input_pointer_cache[pointer_index];
  		    	ip.active = true;
  	        ip.xs = ip.x = AMotionEvent_getX(minput->i_event, pointer_index);
  	        ip.ys = ip.y = AMotionEvent_getY(minput->i_event, pointer_index);
  		    }
		    	break;
		    case AMOTION_EVENT_ACTION_MOVE:
		    	for (size_t i = 0, j = AMotionEvent_getPointerCount(minput->i_event); (i<j) && (i < MAX_TOUCH_POINTERS_COUNT); i++) {
						touch_pointer &ip = input_pointer_cache[i];
						if (!ip.active) continue;
		        ip.x = AMotionEvent_getX(minput->i_event, i);
		        ip.y = AMotionEvent_getY(minput->i_event, i);
		    	}
		    	break;
		    case AMOTION_EVENT_ACTION_POINTER_UP:
		    case AMOTION_EVENT_ACTION_UP:
		    case AMOTION_EVENT_ACTION_OUTSIDE:
		    {
					const int8_t pointer_index = (motion&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
					if (pointer_index >= MAX_TOUCH_POINTERS_COUNT)
						break;
					touch_pointer &ip = input_pointer_cache[pointer_index];
					if (!ip.active) break;
		    	ip.active = false;
	        ip.x = AMotionEvent_getX(minput->i_event, pointer_index);
	        ip.y = AMotionEvent_getY(minput->i_event, pointer_index);
		    }
		    	break;
		    case AMOTION_EVENT_ACTION_CANCEL:
		    	memset(input_pointer_cache, 0, MAX_TOUCH_POINTERS_COUNT*sizeof(touch_pointer));
		    	break;
		    case AMOTION_EVENT_ACTION_SCROLL:
		    case AMOTION_EVENT_ACTION_HOVER_ENTER:
		    case AMOTION_EVENT_ACTION_HOVER_MOVE:
		    case AMOTION_EVENT_ACTION_HOVER_EXIT:
		    	break;
			}
	    handled = 1;
		}
	    break;
	}
  AInputQueue_finishEvent(minput->inputQueue, minput->i_event, handled);
}
void android_input::attach_sensor() {
	if (minput->sensot_enabled) return;
  ASensorEventQueue_enableSensor(sensorEventQueue,accelerometerSensor);
  ASensorEventQueue_setEventRate(sensorEventQueue,accelerometerSensor,50000/3);
  ASensorEventQueue_enableSensor(sensorEventQueue,gyroscopeSensor);
  ASensorEventQueue_setEventRate(sensorEventQueue,gyroscopeSensor,50000/3);
	minput->sensot_enabled = true;
}
void android_input::process_sensor() {
	if (!minput->sensot_enabled) return;
	unsigned int i, j;
	float *sensor_temp;
	while ((i = ASensorEventQueue_getEvents(sensorEventQueue,s_event, 2)) > 0) {
		for (j = 0; j < i; j++) {
			ASensorEvent &e = s_event[j];
			switch (e.type) {
				case ASENSOR_TYPE_ACCELEROMETER:
					sensor_temp = minput->_sensor["accelerometer"];
					sensor_temp[0] = e.acceleration.x/2.f + 0.5f;
					sensor_temp[1] = e.acceleration.y/2.f + 0.5f;
					sensor_temp[2] = e.acceleration.z/2.f + 0.5f;
					break;
				case ASENSOR_TYPE_GYROSCOPE:
					sensor_temp = minput->_sensor["gyroscope"];
					sensor_temp[0] = e.acceleration.x/2.f + 0.5f;
					sensor_temp[1] = e.acceleration.y/2.f + 0.5f;
					sensor_temp[2] = e.acceleration.z/2.f + 0.5f;
					break;
				default:
					break;
			}
		}
	}
}
void android_input::detach_sensor() {
	if (!minput->sensot_enabled) return;
	for (auto i = minput->_sensor.begin(); i != minput->_sensor.end(); i++)
		memset(i->second, 0, 3*sizeof(float));
  ASensorEventQueue_disableSensor(sensorEventQueue, accelerometerSensor);
  ASensorEventQueue_disableSensor(sensorEventQueue, gyroscopeSensor);
	minput->sensot_enabled = false;
}
android_input::android_input(ALooper *looper) {
  minput = new ainput;
	minput->_sensor.emplace("accelerometer", new float[3]{});
	minput->_sensor.emplace("gyroscope", new float[3]{});
	
  s_event = new ASensorEvent[2];
  sensorManager = ASensorManager_getInstance();
  accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_ACCELEROMETER);
  gyroscopeSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_GYROSCOPE);
	sensorEventQueue = ASensorManager_createEventQueue(sensorManager, looper, 3 , NULL, nullptr);
	minput->inputQueue = nullptr;
  minput->sensot_enabled = false;
	//input
	input_pointer_cache = new touch_pointer[MAX_TOUCH_POINTERS_COUNT]{};
	engine::inpt = this;
}
android_input::~android_input() {
	detach_sensor();
	set_input_queue(NULL, NULL);
	for (auto i = minput->_sensor.begin(); i != minput->_sensor.end(); i++)
		delete[] i->second;
	minput->_sensor.clear();
	delete[] input_pointer_cache;
	delete[] s_event;
	key_pressed.clear();
	engine::inpt = nullptr;
	delete minput;
}
