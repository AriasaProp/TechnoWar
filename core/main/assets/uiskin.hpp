#ifndef UISKIN_INCLUDE_
#define UISKIN_INCLUDE_

// static size for how maximum size of image that can be process on any platform (mobile)@
#define PACK_SIZE 1024

#include "../engine.hpp"
#include <string>
#include <vector>

struct region {
  std::string id;
  unsigned int x, y, w, h;
};

struct uiskin {
private:
  unsigned int atlas_size;
  std::vector<region> regions;
  texture_core tex;

public:
  uiskin (unsigned int atlas_size_);
  ~uiskin ();
  static uiskin read_from_filename (const char *);
};

#endif UISKIN_INCLUDE_