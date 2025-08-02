#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "engine.h"
#include "math/vec_math.h"

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
    // random 1 to 0 float
    boxs[i].vel = (vec2){(float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX};
    boxs[i].pos = (vec2){(float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX};
    // velocity around 5 to -5
    vec2_sclf(&boxs[i].vel, 10);
    vec2_trnf(&boxs[i].vel, -5);
    // size 50 - 200 (square)
    boxs[i].size = 50.f + (150.f * (float)rand() / (float)RAND_MAX);
    // position around inside screen - 2*size
    vec2_scl(&boxs[i].pos, vec2_addf(sZ, -boxs[i].size * 2));
    vec2_trnf(&boxs[i].pos, boxs[i].size);
  }
}
struct flat_vertex *game_update(unsigned int *l, float dt) {
  *l = max_box;
  vec2 sZ = global_engine.g.getScreenSize();
  size_t i, j;
  vec2 mdist;
  float bis2, mindist;
  float bottom, top, left, right;
  // update motion by velocity / sec
  for (i = 0; i < max_box; ++i) {
    vec2_trn(&boxs[i].pos, vec2_mulf(boxs[i].vel, dt));
  }
  for (i = 0; i < max_box; ++i) {
    // collision detection + velocity update
    bis2 = boxs[i].size * 0.5f;
    // detect with other box
    for (j = i + 1; j < max_box; ++j) {
      mdist = vec2_sub(boxs[j].pos, boxs[i].pos);
      mindist = bis2 + boxs[j].size * 0.5f;
      if (fabs(mdist.x) <= mindist && fabs(mdist.y) <= mindist) {
        // size as mass
        mindist = 0.5f / mindist;
        mdist = boxs[i].size - boxs[j].size;

        vec2 velA = vec2_mulf(boxs[j].vel, 2 * boxs[j].size);
        vec2 velB = vec2_mulf(boxs[i].vel, 2 * boxs[i].size);

        vec2_sclf(&boxs[i].vel, mdist);
        vec2_sclf(&boxs[j].vel, -mdist);

        vec2_trn(&boxs[i].vel, velA);
        vec2_trn(&boxs[j].vel, velB);

        vec2_sclf(&boxs[i].vel, mindist);
        vec2_sclf(&boxs[j].vel, mindist);

        // fix distance that avoid overlap make multiple collision detection
        /*
        boxs[i].pos = vec2_add(boxs[i].pos, mdist);
        boxs[j].pos = vec2_sub(boxs[j].pos, mdist);
        */
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
  }
  for (i = 0; i < max_box; ++i) {
    bis2 = boxs[i].size * 0.5f;
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