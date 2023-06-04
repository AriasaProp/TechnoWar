#include "main_game.hpp"
#include "engine.hpp"

#include "assets/bmfont.hpp"
#include "assets/stb_image.hpp"
#include "utils/math.hpp"
#include "utils/uistage.hpp"
#include <cmath>
#include <cstdlib> /* srand, rand */
#include <cstring>
#include <ctime> /* time */

struct mMainData {
  engine::mesh_core *mp;
  engine::flat_vertex fV[4] = {
      {120.f, 120.f, {0xff, 0xf0, 0x01, 0xff}, 0, 1},
      {120.f, 520.f, {0xff, 0xf0, 0x01, 0xff}, 0, 0},
      {520.f, 120.f, {0xff, 0xf0, 0x01, 0xff}, 1, 1},
      {520.f, 520.f, {0xff, 0xf0, 0x01, 0xff}, 1, 0}};
  engine::texture_core *myTex;
  bmfont *fnt;
  uistage::actor *button1;
};

Main::Main () {
  mdata = new mMainData{};
  mdata->fnt = new bmfont ("default.fnt");
  {
    int x, y;
    unsigned char *tD = stbi_load_from_assets ("test.jpeg", &x, &y, nullptr, STBI_rgb_alpha);
    mdata->myTex = engine::graph->gen_texture (x, y, tD);
    stbi_image_free (tD);
  }
  engine::mesh_core::data vert[24] = {
      // front red
      {{+350.0f, +350.0f, -350.0f}, {0xff, 0x00, 0x00, 0xff}},
      {{+350.0f, -350.0f, -350.0f}, {0xff, 0x00, 0x00, 0xff}},
      {{-350.0f, -350.0f, -350.0f}, {0xff, 0x00, 0x00, 0xff}},
      {{-350.0f, +350.0f, -350.0f}, {0xff, 0x00, 0x00, 0xff}},
      // left green
      {{-350.0f, +350.0f, -350.0f}, {0x00, 0xff, 0x00, 0xff}},
      {{-350.0f, -350.0f, -350.0f}, {0x00, 0xff, 0x00, 0xff}},
      {{-350.0f, -350.0f, +350.0f}, {0x00, 0xff, 0x00, 0xff}},
      {{-350.0f, +350.0f, +350.0f}, {0x00, 0xff, 0x00, 0xff}},
      // right blue
      {{+350.0f, +350.0f, +350.0f}, {0x00, 0x00, 0xff, 0xff}},
      {{+350.0f, -350.0f, +350.0f}, {0x00, 0x00, 0xff, 0xff}},
      {{+350.0f, -350.0f, -350.0f}, {0x00, 0x00, 0xff, 0xff}},
      {{+350.0f, +350.0f, -350.0f}, {0x00, 0x00, 0xff, 0xff}},
      // bot gray
      {{+350.0f, -350.0f, -350.0f}, {0x33, 0x33, 0x33, 0xff}},
      {{+350.0f, -350.0f, +350.0f}, {0x33, 0x33, 0x33, 0xff}},
      {{-350.0f, -350.0f, +350.0f}, {0x33, 0x33, 0x33, 0xff}},
      {{-350.0f, -350.0f, -350.0f}, {0x33, 0x33, 0x33, 0xff}},
      // top purple
      {{+350.0f, +350.0f, +350.0f}, {0xff, 0x00, 0xff, 0xff}},
      {{+350.0f, +350.0f, -350.0f}, {0xff, 0x00, 0xff, 0xff}},
      {{-350.0f, +350.0f, -350.0f}, {0xff, 0x00, 0xff, 0xff}},
      {{-350.0f, +350.0f, +350.0f}, {0xff, 0x00, 0xff, 0xff}},
      // back fulle
      {{-350.0f, +350.0f, +350.0f}, {0x00, 0xff, 0xff, 0xff}},
      {{-350.0f, -350.0f, +350.0f}, {0xff, 0xff, 0xff, 0xff}},
      {{+350.0f, -350.0f, +350.0f}, {0x00, 0x00, 0xff, 0xff}},
      {{+350.0f, +350.0f, +350.0f}, {0x00, 0xff, 0x00, 0xff}}
  };
  unsigned short indices[36] = {0, 1, 3, 1, 2, 3, 4, 5, 7, 5, 6, 7, 8, 9, 11, 9, 10, 11, 12, 13, 15, 13, 14, 15, 16, 17, 19, 17, 18, 19, 20, 21, 23, 21, 22, 23};
  mdata->mp = engine::graph->gen_mesh (vert, 24, indices, 36);
  
  //add texture regions
  {
    int x, y;
    unsigned char *t = stbi_load_from_assets ("btn1.png", &x, &y, nullptr, STBI_rgb_alpha);
    engine::texture_core *tex = engine::graph->gen_texture (x, y, t);
    uistage::addTextureRegion("btn1" ,tex, uistage::texture_region{{0,0}, {tex->width(),tex->height()}, {10,10,10,10}});
    stbi_image_free (t);
    t = stbi_load_from_assets ("btn1_.png", &x, &y, nullptr, STBI_rgb_alpha);
    tex = engine::graph->gen_texture (x, y, t);
    uistage::addTextureRegion("btn1_",tex, uistage::texture_region{{0,0}, {tex->width(),tex->height()}, {10,10,10,10}});
    stbi_image_free (t);
  }
  uistage::actor *btn = mdata->button1 = new uistage::actor;
  btn->pos[0] = 300.f;
  btn->pos[1] = 150.f;
  btn->size[0] = 200.f;
  btn->size[1] = 200.f;
  btn->texKey =  "btn1";
  resume();
}
void Main::resume () {
  clock_count::start();
}
//AtomicCounter averagefps(10);
void Main::render () {
  float delta = clock_count::getDelta();
  engine::graph->clear (7);
  matrix4::rotate (mdata->mp->trans,
                   M_PI / 12.f * delta, // 15° /s
                   M_PI / 6.f * delta,  // 30° /s
                   M_PI / 3.0f * delta  // 60° /s
  );
  engine::graph->mesh_render (&mdata->mp, 1);
  engine::graph->flat_render (mdata->myTex, mdata->fV, 1);
  
  uistage::draw();
  
  mdata->fnt->draw_text (10, engine::graph->getHeight (), ALIGN_TOP_LEFT, "%03dFPS", clock_count::getFPS());
  mdata->fnt->draw_text (10, engine::graph->getHeight () - 40, ALIGN_TOP_LEFT, "%011u byte", memory_usage::mem_usage());
  mdata->fnt->draw_text (engine::graph->getWidth () * 0.5f, engine::graph->getHeight (), ALIGN_TOP, "4/06/2023");
  mdata->fnt->draw_text (engine::graph->getWidth () - 10, engine::graph->getHeight (), ALIGN_TOP_RIGHT, "feature/ui");
  clock_count::render();
}
void Main::pause () {
}
Main::~Main () {
  clock_count::end();
  delete mdata->button1;
  uistage::clear();
  
  engine::graph->delete_mesh (mdata->mp);
  delete mdata->myTex;
  delete mdata->fnt;
  delete mdata;
}