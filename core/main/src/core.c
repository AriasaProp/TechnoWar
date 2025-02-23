#include "core.h"
#include "engine.h"
#include "util.h"

struct flat_vertex rectangle[] = {
    // Persegi 1
    {(struct vec2){200.0f, 300.0f}, (struct vec2){0.0f, 0.0f}}, // Top-left
    {(struct vec2){250.0f, 350.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-right
    {(struct vec2){250.0f, 300.0f}, (struct vec2){0.0f, 0.0f}}, // Top-right
    {(struct vec2){200.0f, 350.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-left

    // Persegi 2
    {(struct vec2){500.0f, 700.0f}, (struct vec2){0.0f, 0.0f}}, // Top-left
    {(struct vec2){550.0f, 750.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-right
    {(struct vec2){500.0f, 750.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-left
    {(struct vec2){550.0f, 700.0f}, (struct vec2){0.0f, 0.0f}}, // Top-right

    // Persegi 3
    {(struct vec2){1000.0f, 500.0f}, (struct vec2){0.0f, 0.0f}}, // Top-left
    {(struct vec2){1050.0f, 500.0f}, (struct vec2){0.0f, 0.0f}}, // Top-right
    {(struct vec2){1000.0f, 550.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-left
    {(struct vec2){1050.0f, 550.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-right

    // Persegi 4
    {(struct vec2){1700.0f, 200.0f}, (struct vec2){0.0f, 0.0f}}, // Top-left
    {(struct vec2){1750.0f, 200.0f}, (struct vec2){0.0f, 0.0f}}, // Top-right
    {(struct vec2){1750.0f, 250.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-right
    {(struct vec2){1700.0f, 250.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-left

    // Persegi 5
    {(struct vec2){400.0f, 100.0f}, (struct vec2){0.0f, 0.0f}}, // Top-left
    {(struct vec2){450.0f, 100.0f}, (struct vec2){0.0f, 0.0f}}, // Top-right
    {(struct vec2){450.0f, 150.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-right
    {(struct vec2){400.0f, 150.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-left

    // Persegi 6
    {(struct vec2){800.0f, 900.0f}, (struct vec2){0.0f, 0.0f}}, // Top-left
    {(struct vec2){850.0f, 950.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-right
    {(struct vec2){800.0f, 950.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-left
    {(struct vec2){850.0f, 900.0f}, (struct vec2){0.0f, 0.0f}}, // Top-right

    // Persegi 7
    {(struct vec2){1900.0f, 800.0f}, (struct vec2){0.0f, 0.0f}}, // Top-left
    {(struct vec2){1950.0f, 800.0f}, (struct vec2){0.0f, 0.0f}}, // Top-right
    {(struct vec2){1950.0f, 850.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-right
    {(struct vec2){1900.0f, 850.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-left

    // Persegi 8
    {(struct vec2){2200.0f, 600.0f}, (struct vec2){0.0f, 0.0f}}, // Top-left
    {(struct vec2){2250.0f, 650.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-right
    {(struct vec2){2250.0f, 600.0f}, (struct vec2){0.0f, 0.0f}}, // Top-right
    {(struct vec2){2200.0f, 650.0f}, (struct vec2){0.0f, 0.0f}}, // Bottom-left
};

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

  get_engine ()->g.flatRender (0, rectangle, 8);
}
void Main_pause () {
  stateSystem &= ~STATE_SYSTEM_RUNNING;
}

void Main_term () {
  stateSystem = 0;
  memset (&core_cache, 0, sizeof (struct core));
}