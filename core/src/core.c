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
static actor label_hello;
// only called in Main_update when stateSystem not init
static void Main_init() {
  stateSystem |= STATE_SYSTEM_INIT;
  uistage_init();
  game_init();
  label_hello = create_label(128);
  set_actor_origin(label_hello, (vec2){0, 0});
  set_actor_pivot_origin(label_hello, PIVOT_CENTER);
  set_actor_pivot_world(label_hello, PIVOT_CENTER);
  set_label_text(label_hello, "Hello World!");
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

  unsigned int lb;
  mesh *v = game_update(&lb, global_engine.deltaTime());
  global_engine.meshRender(v, lb);
  uistage_draw();
}
void Main_pause() {
  stateSystem &= ~STATE_SYSTEM_RUNNING;
}

void Main_term() {
  destroy_actor(label_hello);
  uistage_term();
  stateSystem = 0;
  memset(&state, 0, sizeof(state));
}