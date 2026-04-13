#include <string.h>

#include "common.h"
#include "core.h"
#include "engine.h"
#include "uistage.h"

// for sample
extern void game_init();
extern mesh *game_update(unsigned int *, float);
extern void game_clean();

struct engine global_engine = {0};
static struct {
  int x, y, z, w;
} state = {0};

void *core_stateSaved(void) {
  void *ret = malloc(sizeof(state));
  memcpy(ret, &state, sizeof(state));
  return ret;
}
unsigned int core_stateLength(void) {
  return sizeof(state);
}
void core_stateLoad(void *data) {
  memcpy(&state, data, sizeof(state));
}

enum {
  STATE_SYSTEM_INIT = 1,
  STATE_SYSTEM_RUNNING = 2,
};
static int stateSystem = 0;
static actor text_hello, info_hello;
// only called in Main_update when stateSystem not init
static void Main_init() {
  stateSystem |= STATE_SYSTEM_INIT;
  uistage_init();
  game_init();
  text_hello = create_text(128);
  set_actor_origin(text_hello, (vec2){0, 0});
  set_actor_pivot(text_hello, PIVOT_LEFT | PIVOT_TOP, PIVOT_LEFT | PIVOT_TOP);
  info_hello = create_text(8049);
  set_actor_origin(info_hello, (vec2){0, 100.f});
  set_actor_pivot(info_hello, PIVOT_LEFT | PIVOT_TOP, PIVOT_LEFT | PIVOT_TOP);
  set_text_str(info_hello, global_engine.engine_graphics_info());
}
// only called in Main_update when stateSystem not running
static void Main_resume() {
  stateSystem |= STATE_SYSTEM_RUNNING;
}

void Main_update() {
  if (!(stateSystem & STATE_SYSTEM_INIT))
    Main_init();
  if (!(stateSystem & STATE_SYSTEM_RUNNING))
    Main_resume();

  set_text_str(text_hello, "%05.2f ms %03lu FPS", global_engine.getDeltaTimeMs(), global_engine.getFPS());
  unsigned int lb;
  mesh *v = game_update(&lb, global_engine.getDeltaTimeMs() / 100.f);
  global_engine.meshRender(v, lb);
  uistage_draw();
}
void Main_pause() {
  stateSystem &= ~STATE_SYSTEM_RUNNING;
}

void Main_term() {
  destroy_actor(text_hello);
  destroy_actor(info_hello);
  uistage_term();
  stateSystem = 0;
  memset(&state, 0, sizeof(state));
}