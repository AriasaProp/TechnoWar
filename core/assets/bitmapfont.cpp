#include "stb_image.hpp"
#include "bitmapfont.hpp"
#include <cstring>
#include <cstdio>
bitmapfont::bitmapfont(const char *dat, const char *tex) {
  int x, y, chnl;
  unsigned char *tD = stbi_load_from_assets(tex, &x, &y, &chnl, STBI_rgb_alpha);
  tex_core = engine::graph->gen_texture (x, y, tD);
  stbi_image_free (tD);
  float xh = (float)x, yh = (float)y;
  float xo = engine::graph->getWidth()/2.f, yo = engine::graph->getHeight()/2.f;
  quad = new engine::flat_vertex[4]{
    {xo-xh, yo-yh, {0xff, 0xff, 0xff, 0xff}, 0, 1},
    {xo-xh, yo+yh, {0xff, 0xff, 0xff, 0xff}, 0, 0},
    {xo+xh, yo-yh, {0xff, 0xff, 0xff, 0xff}, 1, 1},
    {xo+xh, yo+yh, {0xff, 0xff, 0xff, 0xff}, 1, 0}
  };
  (void)dat;
}
void bitmapfont::draw() {
  engine::graph->flat_render(tex_core, quad, 1);
}
bitmapfont::~bitmapfont() {
  engine::graph->delete_texture(tex_core);
  delete quad;
}
