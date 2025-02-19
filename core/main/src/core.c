#include "engine.h"
#include "util.h"
float c = 0.0f, d = 0.27f;
void Main_update () {
  if ((c += 0.08f) > 1.0f) c = 0.f;
  if ((d += 0.15f) > 1.0f) d = 0.f;
  get_engine ()->g.clearColor ((struct fcolor){c, d, 1.0f - c, 1.0f});
  get_engine ()->g.clear (GRAPHICS_CLEAR_COLOR);
}
