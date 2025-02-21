#include "core.h"
#include "engine.h"
#include "util.h"

struct flat_vertex triangle[] = {
    {(struct vec2){200.0f, 200.0f}, (struct vec2){0.0f, 0.0f}},
    {(struct vec2){700.0f, 200.0f}, (struct vec2){0.0f, 0.0f}},
    {(struct vec2){300.0f, 700.0f}, (struct vec2){0.0f, 0.0f}}};

struct core core_cache = {0};

void Main_update (struct core *ic) {
  if ((ic->x += 0.08f) > 1.0f) ic->x = 0.f;
  if ((ic->y += 0.15f) > 1.0f) ic->y = 0.f;

  get_engine ()->g.flatRender (0, triangle, 3);
}

void Main_term (struct core *ic) {
  free_mem (ic);
}