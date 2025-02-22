#include "core.h"
#include "engine.h"
#include "util.h"

struct flat_vertex rectangle[] = {
    {(struct vec2){800.0f, 200.0f}, (struct vec2){0.0f, 0.0f}},
    {(struct vec2){800.0f, 500.0f}, (struct vec2){0.0f, 0.0f}},
    {(struct vec2){400.0f, 200.0f}, (struct vec2){0.0f, 0.0f}},
    {(struct vec2){400.0f, 500.0f}, (struct vec2){0.0f, 0.0f}}};
struct core core_cache = {0};


enum {
	STATE_SYSTEM_INIT = 1, 
	STATE_SYSTEM_RUNNING = 2, 
};
static int stateSystem = 0;
// only called in Main_update when stateSystem not init
static void Main_init () {
	stateSystem |= STATE_SYSTEM_INIT;
	
}
// only called in Main_update when stateSystem not running
static void Main_resume () {
	stateSystem |= STATE_SYSTEM_RUNNING;
}

void Main_update () {
	if (!(stateSystem & STATE_SYSTEM_INIT)) Main_init ();
	if (!(stateSystem & STATE_SYSTEM_RUNNING)) Main_resume ();
	
	
	get_engine()->g.flatRender(0, rectangle, 1);
}
void Main_pause () {
	stateSystem &= ~STATE_SYSTEM_RUNNING;
}

void Main_term () {
	stateSystem = 0;
  memset (&core_cache, 0, sizeof(struct core));
}