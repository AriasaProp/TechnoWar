#ifndef UISKIN_INCLUDE_
#define UISKIN_INCLUDE_

// static size for how maximum size of image that can be process on any platform (mobile)@
#define PACK_SIZE 1024

#include <string>
#include <unordered_set>
#include <cstdint>
#include "../engine.hpp"


struct uiskin {
public:
	uiskin();
	~uiskin();
	static uiskin read_from_filename(const char *);
	
	struct region {
		std::string id;
		uint32_t x,y,w,h;
		
		struct hash {
	    size_t operator()(const region&) const;
	    size_t operator()(const char *) const;
	  };
	};
private:
	unsigned int atlas_size;
	std::unordered_set<region, region::hash> regions;
	engine::texture_core tex;
};




#endif //UISKIN_INCLUDE_