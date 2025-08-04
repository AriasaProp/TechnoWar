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
// only called in Main_update when stateSystem not init
static void Main_init() {
  stateSystem |= STATE_SYSTEM_INIT;
  uistage_init();
  game_init();
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
  mesh *v = game_update(&lb, global_engine.g.deltaTime());
  global_engine.g.meshRender(v, lb);
}
void Main_pause() {
  stateSystem &= ~STATE_SYSTEM_RUNNING;
}

void Main_term() {
  uistage_term();
  stateSystem = 0;
  memset(&state, 0, sizeof(state));
}