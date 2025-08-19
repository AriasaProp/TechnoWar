#ifndef STB_INCLUDE_STB_RECT_PACK_H
#define STB_INCLUDE_STB_RECT_PACK_H

#define STBRP__MAXVAL  0x7fffffff
typedef int stbrp_coord;
struct stbrp_rect {
   unsigned int id;
   stbrp_coord    w, h, x, y;

}; // 16 bytes, nominally
extern int stbrp_pack_rects (struct stbrp_rect *rects, int num_rects, int width, int height);
#endif