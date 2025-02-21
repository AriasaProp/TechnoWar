#include "engine.h"
#include "util.h"
#include "core.h"

void Main_update (struct core *ic) {
  if ((ic->x += 0.08f) > 1.0f) ic->x = 0.f;
  if ((ic->y += 0.15f) > 1.0f) ic->y = 0.f;

  // Just fill the screen with a color.
  get_engine ()->g.clearColor ((struct fcolor){
  	((float)get_engine ()->i.getTouch(0).x / engine->width * 0.5f) + (ic->x * 0.5f),
  	((float)get_engine ()->i.getTouch(0).y * 0.5f) + (ic->y * 0.5f),
  	((float)get_engine ()->i.getTouch(0).y / engine->height * 0.3f) + (ic->x * 0.3f) + (ic->y * 0.4f),
  	1.0f
  });
  get_engine ()->g.clear (GL_COLOR_BUFFER_BIT);
}

void Main_term (struct core *ic) {
	free_mem (ic);
}