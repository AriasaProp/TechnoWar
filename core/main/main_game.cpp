#include "main_game.hpp"
#include "engine.hpp"

#include "stbi/stbi_load.hpp"
#include "utils/math.hpp"
#include "utils/uistage.hpp"
#include <cmath>
#include <cstddef>
#include <cstdlib> /* srand, rand */
#include <cstring>
#include <ctime> /* time */

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

#ifdef BUILD_DATE
#define DATESTR STRINGIZE_VALUE_OF (BUILD_DATE)
#else
#define DATESTR "No Build Date"
#endif // BUILD_DATE

#ifdef BUILD_BRANCH
#define NAMED_BUILD STRINGIZE_VALUE_OF (BUILD_BRANCH)
#else
#define NAMED_BUILD "Unspesified"
#endif // _IDENTITY_

namespace Main {
engine::mesh_core *mp;
uistage::text_actor *t_fps, *t_dlt, *t_mem, *t_dev;

void start () {
  uistage::clear ();
  uistage::loadBMFont ("fonts/default/default.fnt");
  engine::mesh_core::data vert[24] = {
      //{{x,y,z}, 0xabgr
      // front red
      {{+250.0f, +250.0f, -250.0f}, 0xff0000ff},
      {{+250.0f, -250.0f, -250.0f}, 0xff0000ff},
      {{-250.0f, -250.0f, -250.0f}, 0xff0000ff},
      {{-250.0f, +250.0f, -250.0f}, 0xff0000ff},
      // left green
      {{-250.0f, +250.0f, -250.0f}, 0xff00ff00},
      {{-250.0f, -250.0f, -250.0f}, 0xff00ff00},
      {{-250.0f, -250.0f, +250.0f}, 0xff00ff00},
      {{-250.0f, +250.0f, +250.0f}, 0xff00ff00},
      // right blue
      {{+250.0f, +250.0f, +250.0f}, 0xffff0000},
      {{+250.0f, -250.0f, +250.0f}, 0xffff0000},
      {{+250.0f, -250.0f, -250.0f}, 0xffff0000},
      {{+250.0f, +250.0f, -250.0f}, 0xffff0000},
      // bot gray
      {{+250.0f, -250.0f, -250.0f}, 0xff333333},
      {{+250.0f, -250.0f, +250.0f}, 0xff333333},
      {{-250.0f, -250.0f, +250.0f}, 0xff333333},
      {{-250.0f, -250.0f, -250.0f}, 0xff333333},
      // top purple
      {{+250.0f, +250.0f, +250.0f}, 0xff00ffff},
      {{+250.0f, +250.0f, -250.0f}, 0xff00ffff},
      {{-250.0f, +250.0f, -250.0f}, 0xff00ffff},
      {{-250.0f, +250.0f, +250.0f}, 0xff00ffff},
      // back fulle
      {{-250.0f, +250.0f, +250.0f}, 0xffffff00},
      {{-250.0f, -250.0f, +250.0f}, 0xffffffff},
      {{+250.0f, -250.0f, +250.0f}, 0xffff0000},
      {{+250.0f, +250.0f, +250.0f}, 0xff00ff00}};
  unsigned short indices[36] = {0, 1, 3, 1, 2, 3, 4, 5, 7, 5, 6, 7, 8, 9, 11, 9, 10, 11, 12, 13, 15, 13, 14, 15, 16, 17, 19, 17, 18, 19, 20, 21, 23, 21, 22, 23};
  mp = engine::graphics::gen_mesh (vert, 24, indices, 36);

  // add texture regions
  {
    int x, y;
    unsigned char *t;
    t = stbi::load::load_from_assets ("images/btn1.png", &x, &y, nullptr, stbi::load::channel::rgb_alpha);
    engine::texture_core *tex;
    tex = engine::graphics::gen_texture (x, y, t);
    uistage::addTextureRegion ("btn1", tex, uistage::texture_region{{0, 0}, {(unsigned int)x, (unsigned int)y}, {10, 10, 10, 10}});
    stbi::load::image_free (t);
    t = stbi::load::load_from_assets ("images/btn1_.png", &x, &y, nullptr, stbi::load::channel::rgb_alpha);
    tex = engine::graphics::gen_texture (x, y, t);
    uistage::addTextureRegion ("btn1_", tex, uistage::texture_region{{0, 0}, {(unsigned int)x, (unsigned int)y}, {10, 10, 10, 10}});
    stbi::load::image_free (t);
    t = stbi::load::load_from_assets ("images/test.jpeg", &x, &y, nullptr, stbi::load::channel::rgb_alpha);
    tex = engine::graphics::gen_texture (x, y, t);
    uistage::addTextureRegion ("test", tex, uistage::texture_region{{0, 0}, {(unsigned int)x, (unsigned int)y}, {}});
    stbi::load::image_free (t);
  }

  uistage::makeText (Vector2 (0, 0, ALIGN_TOP), ALIGN_TOP, DATESTR);
  uistage::makeText (Vector2 (10, 0, ALIGN_TOP_RIGHT), ALIGN_TOP_RIGHT, NAMED_BUILD);

  t_fps = uistage::makeText (Vector2 (10, 0, ALIGN_TOP_LEFT), ALIGN_TOP_LEFT, "#### FPS");
  t_dlt = uistage::makeText (Vector2 (10, 40, ALIGN_TOP_LEFT), ALIGN_TOP_LEFT, "#### sec");
  t_mem = uistage::makeText (Vector2 (10, 80, ALIGN_TOP_LEFT), ALIGN_TOP_LEFT, "##### MB");
  t_dev = uistage::makeText (Vector2 (10, 120, ALIGN_TOP_LEFT), ALIGN_TOP_LEFT, "Device");

  uistage::makeButton ({"btn1", "btn1_"}, Rect (100, 200, 200, 200, ALIGN_BOTTOM_LEFT, ALIGN_CENTER), [] () -> void {
    uistage::temporaryTooltip ("Tooltip test 1 for button 1. Hello!");
  });
  uistage::makeButton ({"btn1", "btn1_"}, Rect (350, 200, 200, 200, ALIGN_BOTTOM_LEFT, ALIGN_CENTER), [] () -> void {
    uistage::temporaryTooltip ("Tooltip test 2 for button 2. Nothing happen?!");
  });
  uistage::makeImage ("test", Rect (600, 200, 200, 200, ALIGN_BOTTOM_LEFT, ALIGN_CENTER));
  uistage::makeButton ({"btn1", "btn1_"}, Rect (850, 200, 200, 200, ALIGN_BOTTOM_LEFT, ALIGN_CENTER), [] () -> void {
    uistage::temporaryTooltip ("Tooltip test 3 for button 3. Yeah ;-)");
  });
  uistage::makeButton ({"btn1", "btn1_"}, Rect (1100, 200, 200, 200, ALIGN_BOTTOM_LEFT, ALIGN_CENTER), [] () -> void {
    uistage::temporaryTooltip ("Tooltip test 4 for button 4. Okay");
  });

  resume ();
}
void resume () {
  clock_count::start ();
}
void render () {
  t_fps->setText ("%03d FPS", clock_count::getFPS ());
  float delta = clock_count::getDelta ();

  t_dlt->setText ("%03.3f sec", delta);
  t_mem->setText ("%d kByte", engine::info::memory ());
  t_dev->setText ("Platform: %s", engine::info::get_platform_info ());

  engine::graphics::clear (7);
  matrix4::rotate (mp->trans,
                   M_PI / 2.f * delta,  // 90° /s
                   M_PI / 10.f * delta, // 18° /s
                   M_PI / 18.0f * delta // 10° /s
  );

  engine::graphics::mesh_render (&mp, 1);

  uistage::draw (delta);
  clock_count::render ();
}
void pause () {
  clock_count::end ();
}
void end () {
  pause ();
  uistage::clear ();
  engine::graphics::delete_mesh (mp);
}

} // namespace Main
// done
#undef NAMED_BUILD
#undef DATESTR

#undef STRINGIZE_VALUE_OF
#undef STRINGIZE
