#ifndef Included_Actor
#define Included_Actor 1

#include <cstdint>
#include "../../graphics/graphics.h"

struct Actor {
	float x,y,width,height;
	uint32_t color;
	texture_core* tex = nullptr;
	
};

#endif //Included_Actor