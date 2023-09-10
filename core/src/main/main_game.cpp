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

#ifndef BUILD_DATE
#define BUILD_DATE "No Build Date"
#endif //BUILD_DATE
#define DATESTR #BUILD_DATE


struct mMainData {
  engine::mesh_core *mp;
  engine::texture_core *tc;
  bmfont *fnt;
};

Main::Main () {
  mdata = new mMainData{};
  mdata->fnt = new bmfont ("default.fnt");
  mdata->fnt->setFontSize(40.f);
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
      {{+350.0f, +350.0f, +350.0f}, 0xff00ff00}
  };
  unsigned short indices[36] = {0, 1, 3, 1, 2, 3, 4, 5, 7, 5, 6, 7, 8, 9, 11, 9, 10, 11, 12, 13, 15, 13, 14, 15, 16, 17, 19, 17, 18, 19, 20, 21, 23, 21, 22, 23};
  mdata->mp = engine::graph->gen_mesh (vert, 24, indices, 36);
  
  //add texture regions
  {
    int x, y;
    unsigned char *t;
    t = stbi_load_from_assets ("btn1.png", &x, &y, nullptr, STBI_rgb_alpha);
    engine::texture_core *tex;
    tex = engine::graph->gen_texture (x, y, t);
    uistage::addTextureRegion("btn1" ,tex, uistage::texture_region{{0,0}, {(unsigned int)x,(unsigned int)y}, {10,10,10,10}});
    stbi_image_free (t);
    t = stbi_load_from_assets ("btn1_.png", &x, &y, nullptr, STBI_rgb_alpha);
    tex = engine::graph->gen_texture (x, y, t);
    uistage::addTextureRegion("btn1_",tex, uistage::texture_region{{0,0}, {(unsigned int)x,(unsigned int)y}, {10,10,10,10}});
    stbi_image_free (t);
    t = stbi_load_from_assets ("test.jpeg", &x, &y, nullptr, STBI_rgb_alpha);
    tex = engine::graph->gen_texture (x, y, t);
    uistage::addTextureRegion("test",tex, uistage::texture_region{{0,0}, {(unsigned int)x,(unsigned int)y}, {}});
    stbi_image_free (t);
    t = stbi_load_from_assets ("test1.jpg", &x, &y, nullptr, STBI_rgb_alpha);
    mdata->tc = engine::graph->gen_texture (x, y, t);
    stbi_image_free (t);
  }
  uistage::makeButton({"btn1","btn1_"},Rect(400,200,ALIGN_CENTER, 200, 200));
  uistage::makeImage("test",Rect(550,200,ALIGN_CENTER, 200, 200));
  uistage::makeButton({"btn1","btn1_"},Rect(700,200,ALIGN_CENTER, 200, 200));
  uistage::makeButton({"btn1","btn1_"},Rect(850,200,ALIGN_CENTER, 200, 200));
  
  resume();
}
void Main::resume () {
  clock_count::start();
}
void Main::render () {
  float delta = clock_count::getDelta();
  engine::graph->clear (7);
  matrix4::rotate (mdata->mp->trans,
                   M_PI / 2.f * delta, // 90° /s
                   M_PI / 10.f * delta,  // 18° /s
                   M_PI / 18.0f * delta  // 10° /s
  );
  engine::graph->mesh_render (&mdata->mp, 1);
  
  uistage::draw(delta);
  
  mdata->fnt->draw_text (10, engine::graph->getHeight (), ALIGN_TOP_LEFT, "%03dFPS", clock_count::getFPS());
  mdata->fnt->draw_text (10, engine::graph->getHeight () - 40, ALIGN_TOP_LEFT, "%011u byte", memory_usage::mem_usage());
  mdata->fnt->draw_text (engine::graph->getWidth () * 0.5f, engine::graph->getHeight (), ALIGN_TOP, "08/09/2023");
  mdata->fnt->draw_text (engine::graph->getWidth () - 10, engine::graph->getHeight (), ALIGN_TOP_RIGHT, "Main");
  clock_count::render();
  uint32_t clr_ptr1 = engine::inpt->isTouched(0)? 0xff00ff00 : 0xff0000ff;
  uint32_t clr_ptr2 = engine::inpt->isTouched(1)? 0xff00ff00 : 0xff0000ff;
  uint32_t clr_ptr3 = engine::inpt->isTouched(2)? 0xff00ff00 : 0xff0000ff;
  uint32_t clr_ptr4 = engine::inpt->isTouched(3)? 0xff00ff00 : 0xff0000ff;
  uint32_t clr_ptr5 = engine::inpt->isTouched(4)? 0xff00ff00 : 0xff0000ff;
  uint32_t clr_ptr6 = engine::inpt->isTouched(5)? 0xff00ff00 : 0xff0000ff;
  engine::flat_vertex vers[] {
    //background
    { 39.5f, 849.5f, 0xff999999, 0, 1},
    { 39.5f, 900.5f, 0xff999999, 0, 0},
    {450.5f, 849.5f, 0xff999999, 1, 1},
    {450.5f, 900.5f, 0xff999999, 1, 0},
    //ptr 1
    { 40.0f, 850.0f, clr_ptr1, 0, 1},
    { 40.0f, 900.0f, clr_ptr1, 0, 0},
    { 90.0f, 850.0f, clr_ptr1, 1, 1},
    { 90.0f, 900.0f, clr_ptr1, 1, 0},
    //ptr 2
    {100.0f, 850.0f, clr_ptr2, 0, 1},
    {100.0f, 900.0f, clr_ptr2, 0, 0},
    {150.0f, 850.0f, clr_ptr2, 1, 1},
    {150.0f, 900.0f, clr_ptr2, 1, 0},
    //ptr 3
    {160.0f, 850.0f, clr_ptr3, 0, 1},
    {160.0f, 900.0f, clr_ptr3, 0, 0},
    {210.0f, 850.0f, clr_ptr3, 1, 1},
    {210.0f, 900.0f, clr_ptr3, 1, 0},
    //ptr 4
    {220.0f, 850.0f, clr_ptr4, 0, 1},
    {220.0f, 900.0f, clr_ptr4, 0, 0},
    {270.0f, 850.0f, clr_ptr4, 1, 1},
    {270.0f, 900.0f, clr_ptr4, 1, 0},
    //ptr 5
    {280.0f, 850.0f, clr_ptr5, 0, 1},
    {280.0f, 900.0f, clr_ptr5, 0, 0},
    {330.0f, 850.0f, clr_ptr5, 1, 1},
    {330.0f, 900.0f, clr_ptr5, 1, 0},
    //ptr 6
    {340.0f, 850.0f, clr_ptr6, 0, 1},
    {340.0f, 900.0f, clr_ptr6, 0, 0},
    {390.0f, 850.0f, clr_ptr6, 1, 1},
    {390.0f, 900.0f, clr_ptr6, 1, 0}
  };
  engine::graph->flat_render(nullptr, vers, 7);
}
void Main::pause () {
}
Main::~Main () {
  clock_count::end();
  uistage::clear();
  
  engine::graph->delete_mesh (mdata->mp);
  delete mdata->fnt;
  delete mdata->tc;
  delete mdata;
}