#include "engine.h"
#include "util.h"

void Main_update () {
  get_engine ()->g.clearColor ((struct fcolor){1.0f, 0.0f, 0.0f, 1.0f});
  get_engine ()->g.clear (GRAPHICS_CLEAR_COLOR);
}
