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

static float rands(void) {
  return;
}

void game_init() {
  srand(time(0));
  max_box = 5 + (rand() % 15);
  boxs = (struct box *)malloc(sizeof(struct box) * max_box);
  rects = (struct flat_vertex *)malloc(sizeof(struct flat_vertex) * max_box * 4);
  vec2 sZ = global_engine.g.getScreenSize();
  for (int i = 0; i < max_box; ++i) {
    // velocity around 5 to -5
    boxs[i].vel.x = (10.f * (float)rand() / (float)RAND_MAX) - 5.f;
    boxs[i].vel.y = (10.f * (float)rand() / (float)RAND_MAX) - 5.f;
    // size around 180 to 50 (square)
    boxs[i].size = 50.0f + (130.f * rand());
    // position around inside screen - size
    boxs[i].pos.x = CLAMP(boxs[i].size, rand() * sZ.x, sZ.x - boxs[i].size);
    boxs[i].pos.y = CLAMP(boxs[i].size, rand() * sZ.y, sZ.y - boxs[i].size);
  }
}
struct flat_vertex *game_update(unsigned int *l) {
  *l = max_box;
  vec2 sZ = global_engine.g.getScreenSize();
  // update motion
  size_t i, j;
  for (i = 0; i < max_box; ++i) {
    struct box &b = boxs[i];
    b.pos.x += b.vel.x;
    b.pos.y += b.vel.y;
  }
  // collision detection + velocity update
  for (i = 0; i < max_box; ++i) {
    struct box &bi = boxs[i];
    float bis2 = bi.size / 2;
    // detect with other box
    for (j = 0; j < max_box; ++j) {
      if (i == j)
        continue;
      struct box &bj = boxs[j];
      float distx = bi.pos.x - bj.pos.x;
      float disty = bi.pos.y - bj.pos.y;
      float mindist = bis2 + bj.size / 2;
      if (distx <= mindist && disty <= mindist) {
        bi.vel.x += bj.vel.x * bj.size / bi.size;
        bi.vel.y += bj.vel.y * bj.size / bi.size;
      }
    }
    // detect with walls
    if ((bi.pos.x <= bis2) ||
        (bi.pos.x + bis2 >= sZ.x)) {
      bi.vel.x *= -1
    }
    if ((bi.pos.y <= bis2) ||
        (bi.pos.y + bis2 >= sZ.y)) {
      bi.vel.y *= -1
    }
  }
  for (i = 0; i < max_box; ++i) {
    struct box &bx = boxs[i];

    float bottom = bx.pos.y + (bx.size / 2);
    float right = bx.pos.x + (bx.size / 2);
    float top = bx.pos.y - (bx.size / 2);
    float left = bx.pos.x - (bx.size / 2);

    rects[i * 4 + 0] = (vec2){right, bottom}; // Bottom-right
    rects[i * 4 + 1] = (vec2){right, top};    // Top-right
    rects[i * 4 + 2] = (vec2){left, bottom};  // Bottom-left
    rects[i * 4 + 3] = (vec2){left, top};     // Top-left
  }
  return rects;
}
void game_clean() {
  (void)0;
  free(boxs);
  free(rects);
}