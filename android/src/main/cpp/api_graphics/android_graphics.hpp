#ifndef _Included_Android_Graphics
#define _Included_Android_Graphics

#include "../engine.hpp"
#include <android/native_activity.h>

struct safe_insets {
  int left;
  int top;
  int right;
  int bottom;
};

struct android_graphics : public engine::graphics_core {
  bool resize, resume, pause;
  safe_insets cur_safe_insets;
  // android
  virtual void onResume () = 0;
  virtual void onWindowInit (ANativeWindow *) = 0;
  virtual void needResize () = 0;
  virtual void render () = 0;
  virtual void onWindowTerm () = 0;
  virtual void onPause () = 0;
  virtual ~android_graphics () = 0;
};

#endif //_Included_Android_Graphics