#include "stb_image.hpp"
#include "bitmapfont.hpp"
#include <cstring>
#include <cstdio>
/*
static const char *keyChar = 
"!\" $%&'()*+,-./"//0-57
"0123456789"//58-57
":;<=>?@"//58-64
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"//65-90
"[\\]^_`"//91-96
"abcdefghijklmnopqrstuvwxyz"//97-122
"{|}~";//123-126
*/
bitmapfont::bitmapfont(const char *dat, const char *tex) {
  int x, y, chnl;
  unsigned char *tD = stbi_load_from_assets(tex, &x, &y, &chnl, STBI_rgb_alpha);
  tex_core = engine::graph->gen_texture (x, y, tD);
  stbi_image_free (tD);
  float xh = 100.f, yh = 100.f;
  float xo = 720.f, yo = 360.f;
  bool diff = STBI_rgb_alpha != chnl;
  quad = new engine::flat_vertex[4]{
    {xo-xh, yo-yh, {0xff, diff?0xff:0x00, diff?0xff:0x00, 0xff}, 0, 1},
    {xo-xh, yo+yh, {0xff, diff?0xff:0x00, diff?0xff:0x00, 0xff}, 0, 0},
    {xo+xh, yo-yh, {0xff, diff?0xff:0x00, diff?0xff:0x00, 0xff}, 1, 1},
    {xo+xh, yo+yh, {0xff, diff?0xff:0x00, diff?0xff:0x00, 0xff}, 1, 0}
  };
  // Membuka file fnt dan menyimpan objek asset_core ke dalam variabel rc
  engine::asset_core *rc = engine::assets->asset_open(dat);
  //temporary buffer to read file
  char reader[2048];
  // Membaca header file
  for (unsigned int needRead = 2048, readed = 0, curRead = 0; needRead; ) {
    unsigned int rend = rc->read(reader+readed, needRead);
    needRead -= rend;
    readed += rend;
  }
  char *find_value = strstr(reader, "size=");
  if (find_value) sscanf(find_value, "size=%f", &fontSize);
  int temp_int;
  find_value = strstr(reader, "bold=");
  if (find_value) sscanf(find_value, "bold=%d", &temp_int);
  bold = (temp_int);
  find_value = strstr(reader, "italic=");
  if (find_value) sscanf(find_value, "italic=%d", &temp_int);
  italic = (temp_int);
  find_value = strstr(reader, "stretchH=");
  if (find_value) sscanf(find_value, "stretchH=%f", &stretchY);
  find_value = strstr(reader, "stretchW=");
  if (find_value) sscanf(find_value, "stretchW=%f", &stretchX);
  find_value = strstr(reader, "smooth=");
  if (find_value) sscanf(find_value, "smooth=%f", &stretchX);
  find_value = strstr(reader, "padding=");
  if (find_value) sscanf(find_value, "padding=%f,%f,%f,%f", &padding_left, &padding_top, &padding_right, &padding_bottom);
  find_value = strstr(reader, "spacing=");
  if (find_value) sscanf(find_value, "spacing=%f,%f", &padding_left, &padding_top, &padding_right, &padding_bottom);
  /*
  int char_count = 0;
  // Mencari nilai count= pada header
  for
  char *pch = strstr(header, "chars count=");
  if (pch) {
    sscanf(pch, "chars count=%d", &char_count);
  }

  // Mengalokasikan memori untuk array bitmapset_pic
  BitmapSet *bitmapset_pic = new BitmapSet[char_count];

  // Loop untuk membaca setiap baris karakter
  for (int i = 0; i < char_count; i++) {
    char line[256];
    rc->read(line, 256);

    // Memproses nilai-nilai pada baris menggunakan sscanf
    int id = 0, x = 0, y = 0, width = 0, height = 0, xoffset = 0, yoffset = 0, xadvance = 0, page = 0, channel = 0;
    sscanf(line, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d", &id, &x, &y, &width, &height, &xoffset, &yoffset, &xadvance, &page, &channel);

    // Menyimpan nilai-nilai ke dalam struktur BitmapSet yang sesuai
    bitmapset_pic[id].x = x;
    bitmapset_pic[id].y = y;
    bitmapset_pic[id].w = width;
    bitmapset_pic[id].h = height;
    bitmapset_pic[id].xoff = xoffset;
    bitmapset_pic[id].yoff = yoffset;
    bitmapset_pic[id].xadvance = xadvance;
    bitmapset_pic[id].page = page;
    bitmapset_pic[id].channel = channel;
  }
  */
  delete rc;
}
void bitmapfont::draw() {
  engine::graph->flat_render(tex_core, quad, 1);
}
bitmapfont::~bitmapfont() {
  engine::graph->delete_texture(tex_core);
  delete quad;
}
