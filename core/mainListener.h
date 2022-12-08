#ifndef Included_MainListener
#define Included_MainListener 1

#include "translatedGraphicsFunction.h"

namespace Main {
	void create(unsigned int, unsigned int);
	void resume();
	void resize(unsigned int, unsigned int);
	void render(float);
	void pause();
	void destroy();
};

#endif // Included_MainListener

