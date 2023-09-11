#ifndef __BMFONT__
#define __BMFONT__

#include <unordered_map>
#include <vector>

struct KearningInfo;
struct CharDescriptor;

struct bmfont {
public:
  bmfont (const char *);
  ~bmfont ();

private:
  short LineHeight;
  short Base;
  short Width;
  short Height;
  short Pages;
  short Outline;
  std::unordered_map<int, CharDescriptor> Chars;
  std::vector<KearningInfo> Kearn;
  int fcolor;
  float fscale;

  bool ParseFont (const char *);
};

#endif
