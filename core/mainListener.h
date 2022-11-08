#ifndef Included_MainListener
#define Included_MainListener


#include "translatedGraphicsFunction.h"

TranslatedGraphicsFunction *tgf = 0;

namespace Main {
	void create(unsigned int, unsigned int);
	void resume();
	void resize(unsigned int, unsigned int);
	void render(float);
	void pause();
	void destroy();
};

#endif // Included_MainListener

