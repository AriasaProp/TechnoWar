#ifndef __BMFONT__
#define __BMFONT__

#include <vector>
#include <map>

#ifndef MAKE_RGBA
#define MAKE_RGBA(r,g,b,a)  (r | (g << 8) | (b << 16) | (a << 24))
#endif

using namespace std;

#include "../engine.hpp"

struct KearningInfo {
	short First;
	short Second;
	short Amount;
};


class CharDescriptor
{

public:
	short x, y;
	short Width;
	short Height;
	short XOffset;
	short YOffset;
	short XAdvance;
	short Page;

	CharDescriptor() : x( 0 ), y( 0 ), Width( 0 ), Height( 0 ), XOffset( 0 ), YOffset( 0 ),
		XAdvance( 0 ), Page( 0 )
	{ }
};

class BMFont {
public:
	bool LoadFont(char *);
	void SetColor(int r, int g, int b, int a) {fcolor = MAKE_RGBA(r,g,b,a);}
	void SetBlend(int b) {fblend = b;}
	void SetScale(float scale){fscale = scale;}
	float GetHeight(){return LineHeight * fscale;}
	void Print(float, float, const char *,...);
	void PrintCenter( float, const char *);
	BMFont(const char*);
	~BMFont();

private:
  short LineHeight;
	short Base;
	short Width;
	short Height;
	short Pages;
	short Outline;
	std::map<int,CharDescriptor> Chars;
	std::vector<KearningInfo> Kearn;
	int fcolor;
	engine::texture_core *ftexid;
	float fscale;
	int fblend;

	bool ParseFont(const char *);
	int GetKerningPair(int, int);
	float GetStringWidth(const char *);

};

#endif
