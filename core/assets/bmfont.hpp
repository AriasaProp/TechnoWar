#ifndef __BMFONT__
#define __BMFONT__

#include <vector>
#include <unordered_map>

#ifndef MAKE_RGBA
#define MAKE_RGBA(r,g,b,a)  (r | (g << 8) | (b << 16) | (a << 24))
#endif

#include "../engine.hpp"
enum Align {
  ALIGN_TOP_LEFT(0, 0),
  ALIGN_LEFT(0, 1),
  ALIGN_BOTTOM_LEFT(0, 2),
  ALIGN_TOP_CENTER(1, 0),
  ALIGN_CENTER(1,1),
  ALIGN_BOTTOM_CENTER(1, 2),
  ALIGN_TOP_RIGHT(2, 0),
  ALIGN_RIGHT(2,1),
  ALIGN_BOTTOM_RIGHT(2,2);
  
  char v, h;
  Align(char V, char H): v(V), h(H) {}
};

struct KearningInfo;
struct CharDescriptor;

struct bmfont {
public:
	bool LoadFont(char *);
	void SetColor(int r, int g, int b, int a) {fcolor = MAKE_RGBA(r,g,b,a);}
	void SetScale(float scale){fscale = scale;}
	float GetHeight(){return LineHeight * fscale;}
	void draw_text(float, float, Align, const char *,...);
	void draw_text_center (float, const char *);
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
	std::vector<KearningInfo> Kearn;
	int fcolor;
	engine::texture_core *ftexid;
	float fscale;

	bool ParseFont(const char *);
	int GetKerningPair(int, int);
	float GetStringWidth(const char *);

};

#endif
