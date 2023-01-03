#ifndef Included_Android_Input
#define Included_Android_Input 1

#include "android/input.h"
#include "android/sensor.h"
#include "input/input.h"

struct android_input : public input {
public:
	ASensorManager* sensorManager;
  const ASensor* accelerometerSensor;
  const ASensor* gyroscopeSensor;
  ASensorEventQueue* sensorEventQueue;
	android_input();
	~android_input() override;
	float	getAccelerometerX() override;
  float getAccelerometerY() override;
  float getAccelerometerZ() override;
  float getGyroscopeX() override;
  float getGyroscopeY() override;
  float getGyroscopeZ() override;
  int getX(unsigned int) override;
  int getDeltaX(unsigned int) override;
  int getY(unsigned int) override;
  int getDeltaY(unsigned int) override;
  bool justTouched() override;
  bool isTouched(unsigned int) override;
  float getPressure(unsigned int) override;
  bool isButtonPressed(int button) override;
  bool isButtonJustPressed(int button) override;
  bool isKeyPressed(int key) override;
  bool isKeyJustPressed(int key) override;
  float getAzimuth() override;
  float getPitch() override;
  float getRoll() override;
  long getCurrentEventTime() override;
  void setCatchKey(int keycode, bool catchKey) override;
  bool isCatchKey(int keycode) override;
  
  void set_input_queue(AInputQueue *);
  void process_input();
  void attach_sensor();
  void process_sensor();
  void detach_sensor();
};

#endif //Included_Android_Input