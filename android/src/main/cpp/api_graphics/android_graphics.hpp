#ifndef _Included_Android_Graphics
#define _Included_Android_Graphics

#include "../engine.hpp"
#include <android/native_activity.h>
struct android_graphics : public engine::graphics_core {
  float cur_safe_insets[4];
  // android
  virtual void onResume () = 0;
  virtual void onWindowInit (ANativeWindow *) = 0;
  virtual void needResize () = 0;
  virtual void needLayout () = 0;
  virtual void render () = 0;
  virtual void onWindowTerm () = 0;
  virtual void onPause () = 0;
  virtual ~android_graphics () = 0;
};

#endif //_Included_Android_Graphics