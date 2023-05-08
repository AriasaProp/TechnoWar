#ifndef __BMFONT__
#define __BMFONT__

#include <vector>
#include <map>

#ifndef MAKE_RGBA
#define MAKE_RGBA(r,g,b,a)  (r | (g << 8) | (b << 16) | (a << 24))
#endif

#ifndef GET_BLUE
#define GET_BLUE(rgba) (( (rgba)>>16 ) & 0xff )
#endif
 
#ifndef GET_GREEN
#define GET_GREEN(rgba) (( (rgba)>>8 ) & 0xff )
#endif
 
#ifndef GET_RED
#define GET_RED(rgba) ( rgba & 0xff )
#endif

#ifndef GET_ALPHA
#define GET_ALPHA(rgba) (( (rgba)>>24 ) & 0xff)
#endif

using namespace std;

class KearningInfo {
public:
	short First;
	short Second;
	short Amount;

	KearningInfo() :  First( 0 ), Second( 0 ), Amount( 0 )	{ }
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
	BMFont();
	~BMFont();

private:
  short LineHeight;
	short Base;
	short Width;
	short Height;
	short Pages;
	short Outline;
	short KernCount;
	std::map<int,CharDescriptor> Chars;
	std::vector<KearningInfo> Kearn;
	int fcolor;
	engine::texture_core *ftexid;
	float fscale;
	int fblend;

	bool ParseFont(char *);
	int GetKerningPair(int, int);
	float GetStringWidth(const char *);

};

#endif
