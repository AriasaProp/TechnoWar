#ifndef Included_Input
#define Included_Input 1

struct input {
public:
  virtual float	getAccelerometerX() = 0;
  virtual float getAccelerometerY() = 0;
  virtual float getAccelerometerZ() = 0;
  virtual float getGyroscopeX() = 0;
  virtual float getGyroscopeY() = 0;
  virtual float getGyroscopeZ() = 0;
  virtual int getX(unsigned int) = 0;
  virtual int getDeltaX(unsigned int) = 0;
  virtual int getY(unsigned int) = 0;
  virtual int getDeltaY(unsigned int) = 0;
  virtual bool justTouched() = 0;
  virtual bool isTouched(unsigned int) = 0;
  virtual float getPressure(unsigned int) = 0;
  virtual bool isButtonPressed(int button) = 0;
  virtual bool isButtonJustPressed(int button) = 0;
  virtual bool isKeyPressed(int key) = 0;
  virtual bool isKeyJustPressed(int key) = 0;
  virtual float getAzimuth() = 0;
  virtual float getPitch() = 0;
  virtual float getRoll() = 0;
  virtual long getCurrentEventTime() = 0;
  virtual void setCatchKey(int keycode, bool catchKey) = 0;
  virtual bool isCatchKey(int keycode) = 0;
};


#endif // Included_Input