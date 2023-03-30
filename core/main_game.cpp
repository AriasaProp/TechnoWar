#include "main_game.hpp"
#include "engine.hpp"

#include "math/matrix4.hpp"
#include "system/stb_image.hpp"
#include <chrono>
#include <cmath>
#include <cstdlib> /* srand, rand */
#include <cstring>
#include <ctime> /* time */

engine::mesh_core *mp;
engine::flat_vertex *fV;
engine::texture_core *tc;

Main::Main() {
  {
    /*stbi_io_callbacks clbk{
        [](void *d, char *buff, unsigned int len) -> int {
          return engine::asset->read_asset((engine::asset_core *)d, buff, len);
        },
        [](void *d, int l) -> void {
          engine::asset->seek_asset((engine::asset_core *)d, l);
        },
        [](void *d) -> bool {
          return engine::asset->eof_asset((engine::asset_core *)d);
        }
    };
    */
    int w = 2, h = 2;
    engine::asset_core *a_ = engine::asset->open_asset("test2.psd");
    try {
      // stbi_load_from_callbacks(&clbk, (void *)a_, &w, &h, nullptr, STBI_rgb_alpha);
      unsigned char *tempDf = (unsigned char *)malloc(16*sizeof(unsigned char));
    	memcpy(tempDf,(unsigned char[]){
    		0x0f,0x00,0xb0,0xff,
    		0xff,0x00,0x00,0xff,
    		0xf1,0xa7,0x40,0xff,
    		0xff,0x00,0x00,0xff
    	}, 16*sizeof(unsigned char));
      tc = engine::graph->gen_texture(w, h, tempDf);
      stbi_image_free(tempDf);
    } catch (const char *) {
      engine::graph->clearcolor(1, 1, 1, 1);
    }
    engine::asset->close_asset(a_);
  }
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
      0, 1, 3, 1, 2, 3,       // front
      4, 5, 7, 5, 6, 7,       // left
      8, 9, 11, 9, 10, 11,    // right
      12, 13, 15, 13, 14, 15, // bot
      16, 17, 19, 17, 18, 19, // top
      20, 21, 23, 21, 22, 23  // back
  };
  mp = engine::graph->gen_mesh(vert, 24, indices, 36);
  fV = new engine::flat_vertex[4]{
      {120.f, 120.f, {0xff, 0xf0, 0x01, 0xff}, 0, 0},
      {120.f, 520.f, {0xff, 0xf0, 0x01, 0xff}, 0, 1},
      {520.f, 120.f, {0xff, 0xf0, 0x01, 0xff}, 1, 0},
      {520.f, 520.f, {0xff, 0xf0, 0x01, 0xff}, 1, 1}};
}
void Main::resume() {
}
std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now(), end;
float delta = 0;
void Main::render() {
  end = std::chrono::high_resolution_clock::now();
  delta = float(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000000.f;
  start = end;
  engine::inpt->process_event();
  engine::graph->clear(7);

  matrix4::rotate(mp->trans,
                  M_PI / 12.f * (delta), // 15° /s
                  M_PI / 6.f * (delta),  // 30° /s
                  M_PI / 3.0f * (delta)  // 60° /s
  );
  engine::graph->mesh_render(&mp, 1);

  engine::graph->flat_render(tc, fV, 1);
}
void Main::pause() {
}
Main::~Main() {
  engine::graph->delete_mesh(mp);
  delete fV;
  {
    engine::graph->delete_texture(tc);
  }
}