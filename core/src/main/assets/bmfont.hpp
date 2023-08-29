#ifndef __BMFONT__
#define __BMFONT__

#include <unordered_map>
#include <utility>

#include "../engine.hpp"
#include "../utils/math.hpp"

struct CharDescriptor;

struct bmfont {
public:
  void SetColor (unsigned char, unsigned char, unsigned char, unsigned char);
  void setFontSize (float);//px
  float GetHeight ();
  void draw_text (float, float, Align, const char *, ...);
  bmfont (const char *);
  ~bmfont ();

private:
  short LineHeight;
  short Base;
  short Width;
  short Height;
  short Outline;
  std::unordered_map<int, CharDescriptor> Chars;
  std::unordered_map<unsigned int, float> Kearn;
  uint32_t fcolor;
  engine::texture_core *ftexid;
  float fontSizeBase, fontSizeUsed;
  
  float fscale ();
  bool ParseFont (const char *);
};

#endif
