#ifndef _STB_RECTPACK_INCLUDED_
#define _STB_RECTPACK_INCLUDED_

#define STBRP__MAXVAL 0x7fffffff
typedef int stbrp_coord;
typedef struct {
  int id;
  stbrp_coord w, h, x, y;
} stbrp_rect;

#ifndef _STB_RECTPACK_IMPLEMENTATION_
extern int stbrp_pack_rects(stbrp_rect *rects, int num_rects, int width, int height);
#endif // _STB_RECTPACK_IMPLEMENTATION_

#endif // _STB_RECTPACK_INCLUDED_