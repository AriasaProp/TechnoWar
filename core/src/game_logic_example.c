#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "engine.h"
#include "util.h"

struct box {
  vec2 pos, vel;
  float size;
} *boxs = NULL;
struct flat_vertex *rects = NULL;
unsigned int max_box;

void game_init() {
  srand(time(0));
  max_box = 5 + (rand() % 10);
  boxs = (struct box *)malloc(sizeof(struct box) * max_box);
  rects = (struct flat_vertex *)calloc(sizeof(struct flat_vertex), max_box * 4);
  vec2 sZ = global_engine.g.getScreenSize();
  for (int i = 0; i < max_box; ++i) {
    // velocity around 5 to -5
    boxs[i].vel.x = (4.f * (float)rand() / (float)RAND_MAX) - 2.f;
    boxs[i].vel.y = (4.f * (float)rand() / (float)RAND_MAX) - 2.f;
    // size 50 - 200 (square)
    boxs[i].size = 50.f + (150.f * (float)rand() / (float)RAND_MAX);
    // position around inside screen - size
    boxs[i].pos.x = (float)rand() / (float)RAND_MAX * sZ.x;
    boxs[i].pos.x = CLAMP(boxs[i].size, boxs[i].pos.x, sZ.x - boxs[i].size);
    boxs[i].pos.y = (float)rand() / (float)RAND_MAX * sZ.y;
    boxs[i].pos.y = CLAMP(boxs[i].size, boxs[i].pos.y, sZ.y - boxs[i].size);
  }
}
struct flat_vertex *game_update(unsigned int *l) {
  *l = max_box;
  vec2 sZ = global_engine.g.getScreenSize();
  size_t i, j;
  float bis2, distx, disty, mindist;
  float bottom, top, left, right;
  for (i = 0; i < max_box; ++i) {
    // update motion
    boxs[i].pos.x += boxs[i].vel.x;
    boxs[i].pos.y += boxs[i].vel.y;
    // collision detection + velocity update
    bis2 = boxs[i].size * 0.5f;
    ;
    // detect with other box
    for (j = 0; j < max_box; ++j) {
      if (i == j)
        continue;
      distx = boxs[i].pos.x - boxs[j].pos.x;
      disty = boxs[i].pos.y - boxs[j].pos.y;
      mindist = bis2 + boxs[j].size * 0.5f;
      ;
      if (distx <= mindist && disty <= mindist) {
        boxs[i].vel.x *= 0.5f;
        boxs[i].vel.x += boxs[j].vel.x * boxs[j].size / boxs[i].size * 0.5f;
        boxs[i].vel.y *= 0.5f;
        boxs[i].vel.y += boxs[j].vel.y * boxs[j].size / boxs[i].size * 0.5f;
      }
    }
    // detect with walls
    if ((boxs[i].pos.x <= bis2) ||
        (boxs[i].pos.x + bis2 >= sZ.x)) {
      boxs[i].vel.x *= -1.0f;
    }
    if ((boxs[i].pos.y <= bis2) ||
        (boxs[i].pos.y + bis2 >= sZ.y)) {
      boxs[i].vel.y *= -1.0f;
    }
    // draw
    rects[i * 4 + 0].pos = (vec2){boxs[i].pos.x + bis2, boxs[i].pos.y + bis2}; // Bottom-right
    rects[i * 4 + 1].pos = (vec2){boxs[i].pos.x + bis2, boxs[i].pos.y - bis2}; // Top-right
    rects[i * 4 + 2].pos = (vec2){boxs[i].pos.x - bis2, boxs[i].pos.y + bis2}; // Bottom-left
    rects[i * 4 + 3].pos = (vec2){boxs[i].pos.x - bis2, boxs[i].pos.y - bis2}; // Top-left
  }
  return rects;
}
void game_clean() {
  (void)0;
  free(boxs);
  free(rects);
}