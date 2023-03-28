#ifndef Included_Actor
#define Included_Actor 1

#include "../../assets/image.hpp"

struct Actor {
	float x,y,width,height;
	unsigned char color[4];
	image img;
};

#endif //Included_Actor