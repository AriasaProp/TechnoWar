#ifndef _Included_Android_Graphics
#define _Included_Android_Graphics 1

#include "../engine.hpp"
#include <android/native_activity.h>

struct android_graphics : public engine::graphics_core {
  float cur_safe_insets[4];
  // android
  virtual void preRender (ANativeWindow *, unsigned int&) = 0;
  virtual void postRender (bool) = 0;
  virtual void onWindowTerm () = 0;
  virtual ~android_graphics () = 0;
};

#endif //_Included_Android_Graphics