#ifndef _Included_OPENGLES_Graphics
#define _Included_OPENGLES_Graphics

#include "android_graphics.h"

struct opengles_graphics: public android_graphics {
private:
	ANativeWindow *window;
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  EGLConfig eConfig; 
public:
	void onResume() override;
	void onWindowInit(ANativeWindow*) override;
	void needResize() override;
	void render() override;
	void onWindowTerm() override;
	void onPause() override;
	void onDestroy() override;
};
#endif //_Included_OPENGLES_Graphics