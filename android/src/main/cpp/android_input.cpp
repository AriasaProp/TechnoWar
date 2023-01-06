#include "android_input.h"

#include <cstdlib>
#include <cstring>

struct touch_pointer {
	bool active;
	float xs, ys;
	float x, y;
};
float *m_accelerometer;
float *m_gyroscope;
touch_pointer *input_pointer_cache;
ASensorEvent *s_event;
ASensorManager* sensorManager;
const ASensor* accelerometerSensor;
const ASensor* gyroscopeSensor;
ASensorEventQueue* sensorEventQueue;
ALooper *m_looper;

android_input::android_input(ALooper *_looper) {
	m_looper = _looper;
	m_accelerometer = new float[3]{};
	m_gyroscope = new float[3]{};
	input_pointer_cache = new touch_pointer[20]{};
	s_event = new ASensorEvent[2];
  sensorManager = ASensorManager_getInstance();
  accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_ACCELEROMETER);
  gyroscopeSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_GYROSCOPE);
	sensorEventQueue = ASensorManager_createEventQueue(sensorManager, _looper, 3 , NULL, nullptr);
}
input::~input(){}
android_input::~android_input() {
	detach_sensor();
	set_input_queue(NULL);
	delete[] m_accelerometer;
	delete[] m_gyroscope;
	delete[] input_pointer_cache;
	delete[] s_event;
}
float	android_input::getAccelerometerX() { return m_accelerometer[0]; }
float android_input::getAccelerometerY() { return m_accelerometer[1]; }
float android_input::getAccelerometerZ() { return m_accelerometer[2];}
float android_input::getGyroscopeX() {	return m_gyroscope[0]; }
float android_input::getGyroscopeY() {	return m_gyroscope[1]; }
float android_input::getGyroscopeZ() {	return m_gyroscope[2]; }
int android_input::getX(unsigned int p = 0) {	return input_pointer_cache[p].x; }
int android_input::getDeltaX(unsigned int p = 0) {	return input_pointer_cache[p].x - input_pointer_cache[p].xs; }
int android_input::getY(unsigned int p = 0) {	return input_pointer_cache[p].y; }
int android_input::getDeltaY(unsigned int p = 0) {	return input_pointer_cache[p].y - input_pointer_cache[p].ys; }
bool android_input::justTouched() {
	return false;
}
bool android_input::isTouched(unsigned int p = 0) { return input_pointer_cache[p].active; }
float android_input::getPressure(unsigned int p = 0) {
	return false;
}
bool android_input::isButtonPressed(int button) {
	return false;
}
bool android_input::isButtonJustPressed(int button) {
	return false;
}
bool android_input::isKeyPressed(int key) {
	return false;
}
bool android_input::isKeyJustPressed(int key) {
	return false;
}
float android_input::getAzimuth() {
	return 0;
}
float android_input::getPitch() {
	return 0;
}
float android_input::getRoll() {
	return 0;
}
long android_input::getCurrentEventTime() {
	return 0;
}
void android_input::setCatchKey(int keycode, bool catchKey) {
	//todo
}
bool android_input::isCatchKey(int keycode) {
	return false;
}
bool input_enabled = false;
AInputQueue *inputQueue = NULL;
void android_input::set_input_queue(AInputQueue *i) {
	if (inputQueue)
		AInputQueue_detachLooper(inputQueue);
	inputQueue = i;
	if (inputQueue)
		AInputQueue_attachLooper(inputQueue, m_looper, 2, NULL, nullptr);
}
AInputEvent* i_event;
void android_input::process_input() {
	if (inputQueue == NULL) return;
	if (AInputQueue_getEvent(inputQueue, &i_event) < 0) return;
  if (!AInputQueue_preDispatchEvent(inputQueue, i_event)) {
    int32_t handled = 0;
		switch (AInputEvent_getType(i_event)) {
			case AINPUT_EVENT_TYPE_MOTION:
				int32_t motion = AMotionEvent_getAction(i_event);
				int8_t motion_ptr = (motion&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
				int8_t motion_act = motion&AMOTION_EVENT_ACTION_MASK;
				touch_pointer &ip = input_pointer_cache[motion_ptr];
				switch(motion_act) {
			    case AMOTION_EVENT_ACTION_DOWN:
			    	ip.active = true;
		        ip.xs = ip.x = AMotionEvent_getX(i_event, motion_ptr);
		        ip.ys = ip.y = AMotionEvent_getY(i_event, motion_ptr);
			    	break;
			    case AMOTION_EVENT_ACTION_MOVE:
		        ip.x = AMotionEvent_getX(i_event, motion_ptr);
		        ip.y = AMotionEvent_getY(i_event, motion_ptr);
			    	break;
			    case AMOTION_EVENT_ACTION_UP:
			    	ip.active = false;
			    	break;
			    case AMOTION_EVENT_ACTION_CANCEL:
			    	ip.active = false;
			    	break;
			    case AMOTION_EVENT_ACTION_OUTSIDE:
			    	ip.active = false;
			    	break;
			    case AMOTION_EVENT_ACTION_POINTER_DOWN:
			    	break;
			    case AMOTION_EVENT_ACTION_POINTER_UP:
			    	break;
			    case AMOTION_EVENT_ACTION_SCROLL:
			    	break;
			    case AMOTION_EVENT_ACTION_HOVER_ENTER:
			    	break;
			    case AMOTION_EVENT_ACTION_HOVER_MOVE:
			    	break;
			    case AMOTION_EVENT_ACTION_HOVER_EXIT:
			    	break;
				}
		    handled = 1;
		    break;
		}
    AInputQueue_finishEvent(inputQueue, i_event, handled);
  }
}

bool sensor_enabled = false;
void android_input::attach_sensor() {
	if (sensor_enabled) return;
  ASensorEventQueue_enableSensor(sensorEventQueue,accelerometerSensor);
  ASensorEventQueue_setEventRate(sensorEventQueue,accelerometerSensor,50000/3);
  ASensorEventQueue_enableSensor(sensorEventQueue,gyroscopeSensor);
  ASensorEventQueue_setEventRate(sensorEventQueue,gyroscopeSensor,50000/3);
	sensor_enabled = true;
}
void android_input::process_sensor() {
	if (!sensor_enabled) return;
	unsigned int i, j;
	while ((i = ASensorEventQueue_getEvents(sensorEventQueue,s_event, 2)) > 0) {
		for (j = 0; j < i; j++) {
			ASensorEvent &e = s_event[j];
			switch (e.type) {
				case ASENSOR_TYPE_ACCELEROMETER:
					m_accelerometer[0] = e.acceleration.x/2.f + 0.5f;
					m_accelerometer[1] = e.acceleration.y/2.f + 0.5f;
					m_accelerometer[2] = e.acceleration.z/2.f + 0.5f;
					break;
				case ASENSOR_TYPE_GYROSCOPE:
					m_gyroscope[0] = e.acceleration.x/2.f + 0.5f;
					m_gyroscope[1] = e.acceleration.y/2.f + 0.5f;
					m_gyroscope[2] = e.acceleration.z/2.f + 0.5f;
					break;
				default:
					break;
			}
		}
	}
}
void android_input::detach_sensor() {
	if (!sensor_enabled) return;
	memset(m_accelerometer, 0, 3*sizeof(float));
	memset(m_gyroscope, 0, 3*sizeof(float));
  ASensorEventQueue_disableSensor(sensorEventQueue, accelerometerSensor);
  ASensorEventQueue_disableSensor(sensorEventQueue, gyroscopeSensor);
	sensor_enabled = false;
}

