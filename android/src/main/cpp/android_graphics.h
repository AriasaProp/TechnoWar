#ifndef Included_Android_Graphics
#define Included_Android_Graphics 1

#include "graphics/graphics.h"

struct android_graphics : public graphics {
	virtual ~android_graphics() = 0;
	virtual void resize_viewport(const int, const int) = 0;
	// Android may lost resources
	virtual void validate() = 0;
	virtual void invalidate() = 0;
};

#endif // Included_Android_Graphics
