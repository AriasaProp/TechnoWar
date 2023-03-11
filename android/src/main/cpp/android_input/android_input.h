#ifndef _Included_Android_Input
#define _Included_Android_Input

#include "../engine.h"
#include <unordered_set>
#include <android/sensor.h>
#include <android/looper.h>
#include <android/native_activity.h>

//define input engine extern
#define MAX_TOUCH_POINTERS_COUNT 30
struct android_input: public input_core {
	float	*getSensorValue(const char *) override;
	int getX(unsigned int) override;
	int getDeltaX(unsigned int) override;
	int getY(unsigned int) override;
	int getDeltaY(unsigned int) override;
	bool justTouched() override;
	bool isTouched(unsigned int) override;
	float getPressure(unsigned int) override;
	bool isButtonPressed(int) override;
	bool isButtonJustPressed(int) override;
	bool isKeyPressed(int) override;
	bool isKeyJustPressed(int) override;
	void process_event() override
	
	void set_input_queue(ALooper*, AInputQueue*);
	void process_input();
	void attach_sensor();
	void process_sensor();
	void detach_sensor();
	
	void android_input(ALooper*);
	void ~android_input();
};


#endif //_Included_Android_Input