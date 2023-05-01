#ifndef Included_BitmapFont_
#define Included_BitmapFont_

#include "../engine.hpp"

struct bitmapfont {
public:
  bitmapfont(const char *, const char *);
  ~bitmapfont();
  void draw();
  
private:
  engine::flat_vertex *quad;
  engine::texture_core *tex_core;  // ID dari texture yang digunakan untuk menggambar font
  float fontSize;
  bool bold, italic;
  const char *charset;
  int unicode;
  float stretchY, stretchX;
  float smooth, aa;
  float padding_left, padding_top, padding_right, padding_bottom;
  float spacingX, spacingY;
  /*
  float common_lineHeight, base;
  float scaleX, scaleY;
  */
  
};

#endif //Included_BitmapFont_