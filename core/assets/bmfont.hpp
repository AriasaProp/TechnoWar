#ifndef __BMFONT__
#define __BMFONT__

#include <set>
#include <unordered_map>

#ifndef MAKE_RGBA
#define MAKE_RGBA(r,g,b,a)  (r | (g << 8) | (b << 16) | (a << 24))
#endif

#include "../engine.hpp"
// base 3
// horizontal = 0 2 1
// vertical =   3 4 5
// in binary 0xvvhh b
enum Align {
  ALIGN_TOP_LEFT = 0,
  ALIGN_LEFT,
  ALIGN_BOTTOM_LEFT,
  ALIGN_TOP_CENTER,
  ALIGN_CENTER,
  ALIGN_BOTTOM_CENTER,
  ALIGN_TOP_RIGHT,
  ALIGN_RIGHT,
  ALIGN_BOTTOM_RIGHT
};

struct KearningInfo {
	short First, Second, Amount;
};

struct CharDescriptor {
	short x = 0, y = 0;
	short Width = 0;
	short Height = 0;
	short XOffset = 0, YOffset = 0;
	short XAdvance = 0;
	short Page = 0;
	CharDescriptor();
};

struct bmfont {
public:
	bool LoadFont(char *);
	void SetColor(int r, int g, int b, int a) {fcolor = MAKE_RGBA(r,g,b,a);}
	void SetScale(float scale){fscale = scale;}
	float GetHeight(){return LineHeight * fscale;}
	void draw_text(float, float, const char *,...);
	template<typename... Args>
	void draw_text_center (float, const char *, Args...);
	bmfont(const char*);
	~bmfont();

private:
  short LineHeight;
	short Base;
	short Width;
	short Height;
	short Pages;
	short Outline;
	std::unordered_map<int,CharDescriptor> Chars;
	std::set<KearningInfo> Kearn;
	int fcolor;
	engine::texture_core *ftexid;
	float fscale;

	bool ParseFont(const char *);
	int GetKerningPair(int, int);
	float GetStringWidth(const char *);

};

#endif
