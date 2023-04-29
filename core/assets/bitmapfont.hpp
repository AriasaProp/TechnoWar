#ifndef Included_BitmapFont_
#define Included_BitmapFont_

#include "../engine.hpp"

struct bitmapfont {
private:
  engine::flat_vertex *quad;
  engine::texture_core *tex_core;  // ID dari texture yang digunakan untuk menggambar font
  float glyphWidth;  // lebar dari setiap karakter pada font
  float glyphHeight;  // tinggi dari setiap karakter pada font
	struct glyph {
	  float x, y;  // jarak dari posisi karakter
	  float width, height;  // ukuran karakter
	  float xoffset, yoffset;  // perpindahan posisi karakter saat menggambar
	  float xadvance;  // jarak horizontal yang ditempuh setelah menggambar karakter
	};
  glyph glyphData[128];  // data untuk setiap karakter pada font
public:
  bitmapfont(const char *, const char *);
  ~bitmapfont();
  void draw();
};

#endif //Included_BitmapFont_