#ifndef _Included_Android_Input
#define _Included_Android_Input

#include "../engine.hpp"
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>
#include <cstdint>

// define input engine extern
#define MAX_TOUCH_POINTERS_COUNT 30
struct android_input : public engine::input_core {
private:
  struct ainput *minput;

public:
  engine::sensor_value getSensorValue (const char *) const override;
  void getPointerPos (float *, unsigned int) override;
  void getPointerDelta (float *, unsigned int) override;
  bool justTouched () override;
  bool onTouched () override;
  bool isTouched (unsigned int) override;
  float getPressure (unsigned int) override;
  bool isButtonPressed (int) override;
  bool isButtonJustPressed (int) override;
  bool isKeyPressed (int) override;
  bool isKeyJustPressed (int) override;
  void process_event () override;

  void set_input_queue (ALooper *, AInputQueue *);
  void attach_sensor ();
  void detach_sensor ();

  android_input (ALooper *);
  ~android_input ();
};

#endif //_Included_Android_Input