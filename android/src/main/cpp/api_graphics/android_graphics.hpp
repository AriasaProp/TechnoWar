#ifndef _Included_Android_Graphics
#define _Included_Android_Graphics

#include "../engine.hpp"
#include <android/native_activity.h>

// AGSR Android Graphics State Request
#define AGSR_RESUME 1
#define AGSR_PAUSE 2
#define AGSR_DESTROY 4

struct android_graphics : public engine::graphics_core {
  float cur_safe_insets[4];
  // android
  virtual void onResume () = 0;
  virtual void onWindowInit (ANativeWindow *) = 0;
  virtual bool preRender (unsigned int&) = 0;
  virtual void render (unsigned int&) = 0;
  virtual void postRender (bool) = 0;
  virtual void onWindowTerm () = 0;
  virtual void onPause () = 0;
  virtual ~android_graphics () = 0;
};

#endif //_Included_Android_Graphics