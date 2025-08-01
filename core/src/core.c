#include <string.h>

#include "core.h"
#include "engine.h"
#include "uistage.h"
#include "common.h"

// for sample
extern void game_init();
extern struct flat_vertex *game_update(unsigned int *, float);
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
unsigned int core_stateLength (void) {
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
static void *timer = NULL;
// only called in Main_update when stateSystem not init
static void Main_init() {
  stateSystem |= STATE_SYSTEM_INIT;
  uistage_init();
  game_init();
  timer = global_engine.e.time_start_sec();
}
// only called in Main_update when stateSystem not running
static void Main_resume() {
  stateSystem |= STATE_SYSTEM_RUNNING;
  timer = global_engine.e.time_start_sec();
}

void Main_update() {
  if (!(stateSystem & STATE_SYSTEM_INIT))
    Main_init();
  if (!(stateSystem & STATE_SYSTEM_RUNNING))
    Main_resume();
  float deltaTime = global_engine.e.time_end_sec(timer);
  timer = global_engine.e.time_start_sec();
  
  unsigned int lb;
  struct flat_vertex *v = game_update(&lb, deltaTime);
  global_engine.g.flatRender(0, v, lb);
}
void Main_pause() {
  stateSystem &= ~STATE_SYSTEM_RUNNING;
}

void Main_term() {
  uistage_term();
  stateSystem = 0;
  memset(&core_cache, 0, sizeof(struct core));
}