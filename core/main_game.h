#ifndef Included_MainListener
#define Included_MainListener 1

#include "graphics/graphics.h"
#include "input/input.h"

namespace Main {
	void create(graphics*, input*);
	void resume();
	void render(float);
	void pause();
	void destroy();
};

#endif // Included_MainListener

