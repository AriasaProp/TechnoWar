#ifndef Included_BitmapFont_
#define Included_BitmapFont_

#include "../engine.hpp"

const char *keyChar = 
"!\" $%&'()*+,-./"//0-57
"0123456789"//58-57
":;<=>?@"//58-64
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"//65-90
"[\\]^_`"//91-96
"abcdefghijklmnopqrstuvwxyz"//97-122
"{|}~";//123-126

struct bitmapfont {
  engine::texture_core tex_core;  // ID dari texture yang digunakan untuk menggambar font
  float glyphWidth;  // lebar dari setiap karakter pada font
  float glyphHeight;  // tinggi dari setiap karakter pada font
  std::unordered_map<char, Glyph> glyphData;  // data untuk setiap karakter pada font
	struct glyph {
	  float left;  // jarak dari posisi karakter ke kiri
	  float top;  // jarak dari posisi karakter ke atas
	  float width;  // lebar karakter
	  float height;  // tinggi karakter
	  float xoffset;  // perpindahan horizontal dari posisi karakter saat menggambar
	  float yoffset;  // perpindahan vertikal dari posisi karakter saat menggambar
	  float xadvance;  // jarak horizontal yang ditempuh setelah menggambar karakter
	};
};



#endif //Included_BitmapFont_