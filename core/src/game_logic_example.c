#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "engine.h"
#include "util.h"

#define MAX_BOX 10

vec2 vel[MAX_BOX] = {0};
struct flat_vertex rects[MAX_BOX * 4] = {0};

static float rands(void) {
  return (float)rand() / (float)RAND_MAX;
}

void game_init() {
  srand(time(0));
  for (int i = 0; i < MAX_BOX; ++i) {
    vel[i].x = (2.f * rands()) - 1.f;
    vel[i].y = (2.f * rands()) - 1.f;
    float x = 2800.f * rands(), y = 720.f * rands();
    rects[i * 4 + 0] = (vec2){x + 25.f, y + 25.f}; // Bottom-right
    rects[i * 4 + 1] = (vec2){x + 25.f, y - 25.f}; // Top-right
    rects[i * 4 + 2] = (vec2){x - 25.f, y + 25.f}; // Bottom-left
    rects[i * 4 + 3] = (vec2){x - 25.f, y - 25.f}; // Top-left
  }
}
struct flat_vertex *game_update(unsigned int *l) {
  *l = MAX_BOX;
  for (int i = 0; i < MAX_BOX; ++i) {
    rects[i * 4 + 0].pos.x += vel[i].x;
    rects[i * 4 + 0].pos.y += vel[i].y;
    rects[i * 4 + 1].pos.x += vel[i].x;
    rects[i * 4 + 1].pos.y += vel[i].y;
    rects[i * 4 + 2].pos.x += vel[i].x;
    rects[i * 4 + 2].pos.y += vel[i].y;
    rects[i * 4 + 3].pos.x += vel[i].x;
    rects[i * 4 + 3].pos.y += vel[i].y;

    if ((rects[i * 4 + 3].pos.y < -50.f) ||
        (rects[i * 4 + 3].pos.y > 700.f)) {
      vel[i].y *= -1.f;
    }
    if ((rects[i * 4 + 3].pos.x < -100.f) ||
        (rects[i * 4 + 3].pos.x > 3200.f)) {
      vel[i].x *= -1.f;
    }
  }
  return rects;
}
void game_clean() {
  (void)0;
}