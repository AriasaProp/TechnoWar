#include "main_game.hpp"
#include "engine.hpp"

#include "assets/stb_image.hpp"
#include "utils/math.hpp"
#include "utils/uistage.hpp"
#include <cmath>
#include <cstdlib> /* srand, rand */
#include <cstring>
#include <ctime> /* time */

#ifndef BUILD_DATE
#define BUILD_DATE "No Build Date"
#endif // BUILD_DATE
#define DATESTR #BUILD_DATE

struct mMainData {
  engine::mesh_core *mp;
  engine::texture_core *tc;
  engine::flat_vertex *touch_ptr;
  uistage::text_actor *t_fps, *t_dlt, *t_mem;
};

Main::Main () {
  mdata = new mMainData{};
  uistage::loadBMFont ("default.fnt");
  mdata->touch_ptr = new engine::flat_vertex[]{
      // background
      {35.5f, 845.0f, 0xff999999, 0, 1},
      {35.5f, 905.0f, 0xff999999, 0, 0},
      {405.5f, 845.0f, 0xff999999, 1, 1},
      {405.5f, 905.0f, 0xff999999, 1, 0},
      // ptr 1
      {40.0f, 850.0f, 0xff00ff00, 0, 1},
      {40.0f, 900.0f, 0xff00ff00, 0, 0},
      {90.0f, 850.0f, 0xff00ff00, 1, 1},
      {90.0f, 900.0f, 0xff00ff00, 1, 0},
      // ptr 2
      {100.0f, 850.0f, 0xff00ff00, 0, 1},
      {100.0f, 900.0f, 0xff00ff00, 0, 0},
      {150.0f, 850.0f, 0xff00ff00, 1, 1},
      {150.0f, 900.0f, 0xff00ff00, 1, 0},
      // ptr 3
      {160.0f, 850.0f, 0xff00ff00, 0, 1},
      {160.0f, 900.0f, 0xff00ff00, 0, 0},
      {210.0f, 850.0f, 0xff00ff00, 1, 1},
      {210.0f, 900.0f, 0xff00ff00, 1, 0},
      // ptr 4
      {220.0f, 850.0f, 0xff00ff00, 0, 1},
      {220.0f, 900.0f, 0xff00ff00, 0, 0},
      {270.0f, 850.0f, 0xff00ff00, 1, 1},
      {270.0f, 900.0f, 0xff00ff00, 1, 0},
      // ptr 5
      {280.0f, 850.0f, 0xff00ff00, 0, 1},
      {280.0f, 900.0f, 0xff00ff00, 0, 0},
      {330.0f, 850.0f, 0xff00ff00, 1, 1},
      {330.0f, 900.0f, 0xff00ff00, 1, 0},
      // ptr 6
      {340.0f, 850.0f, 0xff00ff00, 0, 1},
      {340.0f, 900.0f, 0xff00ff00, 0, 0},
      {390.0f, 850.0f, 0xff00ff00, 1, 1},
      {390.0f, 900.0f, 0xff00ff00, 1, 0}
  };
  engine::mesh_core::data vert[24] = {
      //{{x,y,z}, 0xabgr
      // front red
      {{+350.0f, +350.0f, -350.0f}, 0xff0000ff},
      {{+350.0f, -350.0f, -350.0f}, 0xff0000ff},
      {{-350.0f, -350.0f, -350.0f}, 0xff0000ff},
      {{-350.0f, +350.0f, -350.0f}, 0xff0000ff},
      // left green
      {{-350.0f, +350.0f, -350.0f}, 0xff00ff00},
      {{-350.0f, -350.0f, -350.0f}, 0xff00ff00},
      {{-350.0f, -350.0f, +350.0f}, 0xff00ff00},
      {{-350.0f, +350.0f, +350.0f}, 0xff00ff00},
      // right blue
      {{+350.0f, +350.0f, +350.0f}, 0xffff0000},
      {{+350.0f, -350.0f, +350.0f}, 0xffff0000},
      {{+350.0f, -350.0f, -350.0f}, 0xffff0000},
      {{+350.0f, +350.0f, -350.0f}, 0xffff0000},
      // bot gray
      {{+350.0f, -350.0f, -350.0f}, 0xff333333},
      {{+350.0f, -350.0f, +350.0f}, 0xff333333},
      {{-350.0f, -350.0f, +350.0f}, 0xff333333},
      {{-350.0f, -350.0f, -350.0f}, 0xff333333},
      // top purple
      {{+350.0f, +350.0f, +350.0f}, 0xff00ffff},
      {{+350.0f, +350.0f, -350.0f}, 0xff00ffff},
      {{-350.0f, +350.0f, -350.0f}, 0xff00ffff},
      {{-350.0f, +350.0f, +350.0f}, 0xff00ffff},
      // back fulle
      {{-350.0f, +350.0f, +350.0f}, 0xffffff00},
      {{-350.0f, -350.0f, +350.0f}, 0xffffffff},
      {{+350.0f, -350.0f, +350.0f}, 0xffff0000},
      {{+350.0f, +350.0f, +350.0f}, 0xff00ff00}};
  unsigned short indices[36] = {0, 1, 3, 1, 2, 3, 4, 5, 7, 5, 6, 7, 8, 9, 11, 9, 10, 11, 12, 13, 15, 13, 14, 15, 16, 17, 19, 17, 18, 19, 20, 21, 23, 21, 22, 23};
  mdata->mp = engine::graph->gen_mesh (vert, 24, indices, 36);

  // add texture regions
  {
    int x, y;
    unsigned char *t;
    t = stbi_load_from_assets ("btn1.png", &x, &y, nullptr, STBI_rgb_alpha);
    engine::texture_core *tex;
    tex = engine::graph->gen_texture (x, y, t);
    uistage::addTextureRegion ("btn1", tex, uistage::texture_region{{0, 0}, {(unsigned int)x, (unsigned int)y}, {10, 10, 10, 10}});
    stbi_image_free (t);
    t = stbi_load_from_assets ("btn1_.png", &x, &y, nullptr, STBI_rgb_alpha);
    tex = engine::graph->gen_texture (x, y, t);
    uistage::addTextureRegion ("btn1_", tex, uistage::texture_region{{0, 0}, {(unsigned int)x, (unsigned int)y}, {10, 10, 10, 10}});
    stbi_image_free (t);
    t = stbi_load_from_assets ("test.jpeg", &x, &y, nullptr, STBI_rgb_alpha);
    tex = engine::graph->gen_texture (x, y, t);
    uistage::addTextureRegion ("test", tex, uistage::texture_region{{0, 0}, {(unsigned int)x, (unsigned int)y}, {}});
    stbi_image_free (t);
    t = stbi_load_from_assets ("test1.jpg", &x, &y, nullptr, STBI_rgb_alpha);
    stbi_image_free (t);
  }

  uistage::makeText (engine::graph->getWidth () * 0.5f, engine::graph->getHeight (), ALIGN_TOP, "08/09/2023");
  uistage::makeText (engine::graph->getWidth () - 10, engine::graph->getHeight (), ALIGN_TOP_RIGHT, "Main");
  
  
  t_fps = uistage::makeText (10, engine::graph->getHeight (), ALIGN_TOP_LEFT, "%03dFPS", clock_count::getFPS());
  t_dlt = uistage::makeText (10, engine::graph->getHeight () - 40, ALIGN_TOP_LEFT, "%03d sec", memory_usage::mem_usage());
  t_mem = uistage::makeText (10, engine::graph->getHeight () - 80, ALIGN_TOP_LEFT, "%011u byte", memory_usage::mem_usage());
  

  uistage::makeButton ({"btn1", "btn1_"}, Rect (150, 200, ALIGN_CENTER, 200, 200), NULL);
  uistage::makeButton ({"btn1", "btn1_"}, Rect (400, 200, ALIGN_CENTER, 200, 200), NULL);
  uistage::makeImage ("test", Rect (650, 200, ALIGN_CENTER, 200, 200));
  uistage::makeButton ({"btn1", "btn1_"}, Rect (900, 200, ALIGN_CENTER, 200, 200), NULL);
  uistage::makeButton ({"btn1", "btn1_"}, Rect (1150, 200, ALIGN_CENTER, 200, 200), NULL);

  resume ();
}
void Main::resume () {
  clock_count::start ();
}
void Main::render () {
  mdata->t_fps->setText("%03d FPS", clock_count::getFPS());
  float delta = clock_count::getDelta ();
  mdata->t_dlt->setText("%03d sec", delta);
  mdata->t_mem->setText("%011u byte", memory_usage::mem_usage());
  engine::graph->clear (7);
  matrix4::rotate (mdata->mp->trans,
                   M_PI / 2.f * delta,  // 90° /s
                   M_PI / 10.f * delta, // 18° /s
                   M_PI / 18.0f * delta // 10° /s
  );
  engine::graph->mesh_render (&mdata->mp, 1);

  uistage::draw (delta);
  clock_count::render ();
  engine::flat_vertex *tch = mdata->touch_ptr;
  tch[4].color = engine::inpt->isTouched (0) ? 0xff00ff00 : 0xff0000ff;
  tch[5].color = tch[6].color = tch[7].color = tch[4].color;
  tch[8].color = engine::inpt->isTouched (1) ? 0xff00ff00 : 0xff0000ff;
  tch[9].color = tch[10].color = tch[11].color = tch[8].color;
  tch[12].color = engine::inpt->isTouched (2) ? 0xff00ff00 : 0xff0000ff;
  tch[13].color = tch[14].color = tch[15].color = tch[12].color;
  tch[16].color = engine::inpt->isTouched (3) ? 0xff00ff00 : 0xff0000ff;
  tch[17].color = tch[18].color = tch[19].color = tch[16].color;
  tch[20].color = engine::inpt->isTouched (4) ? 0xff00ff00 : 0xff0000ff;
  tch[21].color = tch[22].color = tch[23].color = tch[20].color;
  tch[24].color = engine::inpt->isTouched (5) ? 0xff00ff00 : 0xff0000ff;
  tch[25].color = tch[26].color = tch[27].color = tch[24].color;
  engine::graph->flat_render (nullptr, tch, 7);
}
void Main::pause () {
}
Main::~Main () {
  clock_count::end ();
  uistage::clear ();

  engine::graph->delete_mesh (mdata->mp);
  delete[] mdata->touch_ptr;
  delete mdata;
}