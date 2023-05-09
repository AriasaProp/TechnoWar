#include "main_game.hpp"
#include "engine.hpp"

#include "assets/bmfont.hpp"
#include "assets/stb_image.hpp"
#include "math/matrix4.hpp"
#include <chrono>
#include <cmath>
#include <cstdlib> /* srand, rand */
#include <cstring>
#include <ctime> /* time */

engine::mesh_core *mp;
engine::flat_vertex *fV, *fV1;
engine::texture_core *myTex;
bmfont *fnt;

Main::Main () {
  fnt = new bmfont("default.fnt");
  int x, y;
  unsigned int datRI;
  void *datR = engine::asset->asset_buffer("test.jpeg", &datRI);
  unsigned char *tD = stbi_load_from_memory ((unsigned char const*)datR, (int)datRI, &x, &y, nullptr, STBI_rgb_alpha);
  free(datR);
  myTex = engine::graph->gen_texture (x, y, tD);
  stbi_image_free (tD);
  engine::mesh_core::data vert[24] = {
      // front red
      {+350.0f, +350.0f, -350.0f, 0xff, 0x00, 0x00, 0xff},
      {+350.0f, -350.0f, -350.0f, 0xff, 0x00, 0x00, 0xff},
      {-350.0f, -350.0f, -350.0f, 0xff, 0x00, 0x00, 0xff},
      {-350.0f, +350.0f, -350.0f, 0xff, 0x00, 0x00, 0xff},
      // left green
      {-350.0f, +350.0f, -350.0f, 0x00, 0xff, 0x00, 0xff},
      {-350.0f, -350.0f, -350.0f, 0x00, 0xff, 0x00, 0xff},
      {-350.0f, -350.0f, +350.0f, 0x00, 0xff, 0x00, 0xff},
      {-350.0f, +350.0f, +350.0f, 0x00, 0xff, 0x00, 0xff},
      // right blue
      {+350.0f, +350.0f, +350.0f, 0x00, 0x00, 0xff, 0xff},
      {+350.0f, -350.0f, +350.0f, 0x00, 0x00, 0xff, 0xff},
      {+350.0f, -350.0f, -350.0f, 0x00, 0x00, 0xff, 0xff},
      {+350.0f, +350.0f, -350.0f, 0x00, 0x00, 0xff, 0xff},
      // bot gray
      {+350.0f, -350.0f, -350.0f, 0x33, 0x33, 0x33, 0xff},
      {+350.0f, -350.0f, +350.0f, 0x33, 0x33, 0x33, 0xff},
      {-350.0f, -350.0f, +350.0f, 0x33, 0x33, 0x33, 0xff},
      {-350.0f, -350.0f, -350.0f, 0x33, 0x33, 0x33, 0xff},
      // top purple
      {+350.0f, +350.0f, +350.0f, 0xff, 0x00, 0xff, 0xff},
      {+350.0f, +350.0f, -350.0f, 0xff, 0x00, 0xff, 0xff},
      {-350.0f, +350.0f, -350.0f, 0xff, 0x00, 0xff, 0xff},
      {-350.0f, +350.0f, +350.0f, 0xff, 0x00, 0xff, 0xff},
      // back fulle
      {-350.0f, +350.0f, +350.0f, 0x00, 0xff, 0xff, 0xff},
      {-350.0f, -350.0f, +350.0f, 0xff, 0xff, 0xff, 0xff},
      {+350.0f, -350.0f, +350.0f, 0x00, 0x00, 0xff, 0xff},
      {+350.0f, +350.0f, +350.0f, 0x00, 0xff, 0x00, 0xff}};
  unsigned short indices[36] = {
      0, 1, 3, 1, 2, 3, 4, 5, 7, 5, 6, 7, 8, 9, 11, 9, 10, 11, 12, 13, 15, 13, 14, 15, 16, 17, 19, 17, 18, 19, 20, 21, 23, 21, 22, 23};
  mp = engine::graph->gen_mesh (vert, 24, indices, 36);
  fV = new engine::flat_vertex[4]{
      {120.f, 120.f, {0xff, 0xf0, 0x01, 0xff}, 0, 1},
      {120.f, 520.f, {0xff, 0xf0, 0x01, 0xff}, 0, 0},
      {520.f, 120.f, {0xff, 0xf0, 0x01, 0xff}, 1, 1},
      {520.f, 520.f, {0xff, 0xf0, 0x01, 0xff}, 1, 0}};
  fV1 = new engine::flat_vertex[4]{
      {320.f, 320.f, {0xff, 0xf0, 0x01, 0xff}, 0, 1},
      {320.f, 720.f, {0xff, 0xf0, 0x01, 0xff}, 0, 0},
      {720.f, 320.f, {0xff, 0xf0, 0x01, 0xff}, 1, 1},
      {720.f, 720.f, {0xff, 0xf0, 0x01, 0xff}, 1, 0}};
}
void Main::resume () {
}
std::chrono::time_point<std::chrono::high_resolution_clock> startP = std::chrono::high_resolution_clock::now(), endP;
float delta = 0;
void Main::render () {
  endP = std::chrono::high_resolution_clock::now();
  delta = float (std::chrono::duration_cast<std::chrono::microseconds> (endP - startP).count ()) / 1000000.f;
  startP = endP;
  engine::inpt->process_event ();
  engine::graph->clear (7);

  matrix4::rotate (mp->trans,
                   M_PI / 12.f * (delta), // 15° /s
                   M_PI / 6.f * (delta),  // 30° /s
                   M_PI / 3.0f * (delta)  // 60° /s
  );
  engine::graph->mesh_render (&mp, 1);
  engine::graph->flat_render (myTex, fV, 1);
  engine::graph->flat_render (myTex, fV1, 1);
  fnt->draw_text(engine::graph->getWidth(), engine::graph->getHeight(), ALIGN_TOP_RIGHT, "Hello World!");
}
void Main::pause () {
}
Main::~Main () {
  engine::graph->delete_mesh (mp);
  delete myTex;
  delete[] fV;
  delete[] fV1;
  delete fnt;
}