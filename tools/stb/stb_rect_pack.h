#ifndef __STB_RECT_PACK_H__
#define __STB_RECT_PACK_H__

#define STBRP__MAXVAL 0x7fffffff
typedef int stbrp_coord;
typedef struct {
  int id;
  stbrp_coord w, h, x, y;
} stbrp_rect;
#ifndef __STB_RECT_PACK_IMPLEMENTATION__
extern int stbrp_pack_rects(stbrp_rect *rects, int num_rects, int width, int height);
#endif // __STB_RECT_PACK_IMPLEMENTATION__
#endif // __STB_RECT_PACK_H__