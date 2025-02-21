#include "engine.h"
#include "util.h"
struct core init_core;
float c = 0.0f, d = 0.27f;
void Main_update () {
  if ((c += 0.08f) > 1.0f) c = 0.f;
  if ((d += 0.15f) > 1.0f) d = 0.f;

  // Just fill the screen with a color.
  get_engine ()->g.clearColor ((struct fcolor){
  	((float)get_engine ()->i.getTouch(0).x / engine->width * 0.5f) + (c * 0.5f),
  	((float)get_engine ()->i.getTouch(0).y * 0.5f) + (d * 0.5f),
  	((float)get_engine ()->i.getTouch(0).y / engine->height * 0.3f) + (c * 0.3f) + (d * 0.4f),
  	1.0f
  });
  get_engine ()->g.clear (GL_COLOR_BUFFER_BIT);
}
