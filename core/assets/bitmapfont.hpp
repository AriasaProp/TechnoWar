#ifndef Included_BitmapFont_
#define Included_BitmapFont_

#include "../engine.hpp"

struct bitmapfont {
private:
  engine::flat_vertex *quad;
  engine::texture_core *tex_core;  // ID dari texture yang digunakan untuk menggambar font
public:
  bitmapfont(const char *, const char *);
  ~bitmapfont();
  void draw();
};

#endif //Included_BitmapFont_