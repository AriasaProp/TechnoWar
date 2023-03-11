#ifndef _Included_VULKAN_Graphics
#define _Included_VULKAN_Graphics

#include "android_graphics.h"

struct vulkan_graphics: public android_graphics {
private:
	ANativeWindow *window;
public:
	void onResume() override;
	void onWindowInit(ANativeWindow*) override;
	void needResize() override;
	void render() override;
	void onWindowTerm() override;
	void onPause() override;
	void onDestroy() override;
};

#endif //_Included_VULKAN_Graphics