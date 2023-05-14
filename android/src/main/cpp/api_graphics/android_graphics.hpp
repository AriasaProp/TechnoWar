#ifndef _Included_Android_Graphics
#define _Included_Android_Graphics

#include <EGL/egl.h> 
#include <android/native_activity.h>
#include "../engine.hpp"

#define TERM_EGL_SURFACE 1
#define TERM_EGL_CONTEXT 2
#define TERM_EGL_DISPLAY 4

struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};
struct safe_insets {
  int left;
  int top;
  int right;
  int bottom;
};

struct android_graphics: public engine::graphics_core {
	bool resize, resume, running, pause, destroyed;
	saved_state state;
	safe_insets cur_safe_insets;
	//android
	virtual void onResume() = 0;
	virtual void onWindowInit(ANativeWindow*) = 0;
	virtual void needResize() = 0;
	virtual void render() = 0;
	virtual void onWindowTerm() = 0;
	virtual void onPause() = 0;
	virtual ~android_graphics() = 0;
};

#endif //_Included_Android_Graphics