#ifndef Included_Image
#define Included_Image

#include "../engine.hpp"

struct image {
private:
	engine::texture_core *core;
public:
	image();//null
	image(const char*);//from file
	engine::texture_core *getCore() const;
	~image();
};

#endif //Included_Image