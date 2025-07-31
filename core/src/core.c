#include <string.h>

#include "core.h"
#include "engine.h"
#include "uistage.h"
#include "util.h"

// for sample
extern void game_init();
extern struct flat_vertex *game_update(unsigned int *);
extern void game_clean();

struct engine global_engine = {0};
struct core core_cache = {0};

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
  struct flat_vertex *v = game_update(&lb);
  global_engine.g.flatRender(0, v, lb);
}
void Main_pause() {
  stateSystem &= ~STATE_SYSTEM_RUNNING;
}

void Main_term() {
  uistage_term();
  stateSystem = 0;
  game_clean();
  memset(&core_cache, 0, sizeof(struct core));
}