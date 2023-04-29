#include "bitmapfont.hpp"
#include "stb_image.hpp"

static const char *keyChar = 
"!\" $%&'()*+,-./"//0-57
"0123456789"//58-57
":;<=>?@"//58-64
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"//65-90
"[\\]^_`"//91-96
"abcdefghijklmnopqrstuvwxyz"//97-122
"{|}~";//123-126

bitmapfont::bitmapfont(const char *dat, const char *tex) {
  int x, y;
  unsigned char *tD = stbi_load_from_assets(tex, &x, &y, nullptr, STBI_rgb_alpha);
  tex_core = engine::graph->gen_texture (x, y, tD);
  stbi_image_free (tD);
  (void)dat;
  float xh = (float)x/2.f, yh = (float)y/2.f;
  float xo = engine::graph->getWidth()/2.f, yo = engine::graph->getHeight()/2.f;
  quad = new engine::flat_vertex[4]{
    {xo-xh, yo-yh, {0xff, 0xff, 0xff, 0xff}, 0, 1},
    {xo-xh, yo+yh, {0xff, 0xff, 0xff, 0xff}, 0, 0},
    {xo+xh, yo-yh, {0xff, 0xff, 0xff, 0xff}, 1, 1},
    {xo+xh, yo+yh, {0xff, 0xff, 0xff, 0xff}, 1, 0}
  };
}
void bitmapfont::draw() {
  engine::graph->flat_render(tex_core, quad, 1);
}
bitmapfont::~bitmapfont() {
  engine::graph->delete_texture(tex_core);
  delete quad;
}
