#ifndef _Included_Android_Graphics
#define _Included_Android_Graphics

#include <EGL/egl.h> 
#include <android/native_activity.h>
#include "../engine.h"

struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

struct android_graphics {
	bool resize, resume, running, pause, destroyed;
	saved_state state;
	virtual void onResume() = 0;
	virtual void onWindowInit(ANativeWindow*) = 0;
	virtual void needResize() = 0;
	virtual void render() = 0;
	virtual void onWindowTerm() = 0;
	virtual void onPause() = 0;
	virtual void onDestroy() = 0;
};

#endif //_Included_Android_Graphics