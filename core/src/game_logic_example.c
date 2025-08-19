#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "engine.h"
#include "math_util.h"

// at least 4
#define CIRCLE_PRECISION 20

static struct particle {
  vec2 pos,
    vel;
  float r;
} *particles = NULL;
static mesh *particle_meshes = NULL;
static unsigned int max_particle = 0;

void game_init() {
  srand(time(0));
  max_particle = 5 + (rand() % 10);
  particles = (struct particle *)malloc(sizeof(struct particle) * max_particle);
  particle_meshes = (mesh *)malloc(sizeof(mesh) * max_particle);
  vec2 sZ = vec2_mulf(global_engine.getScreenSize(), 0.5f);
  // duplicate common use
  size_t
    vertex_len = CIRCLE_PRECISION * 2,
    vertex_len_byte = vertex_len * sizeof(mesh_vertex),
    index_len = vertex_len * 3,
    index_len_byte = index_len * sizeof(mesh_index);
  mesh_index *is = (mesh_index *)malloc(index_len_byte);
  for (mesh_index i = 0; i < CIRCLE_PRECISION; ++i) {
    is[i * 6 + 0] = i + 1;
    is[i * 6 + 1] = i;
    is[i * 6 + 2] = vertex_len - i - 1;
    is[i * 6 + 3] = i + 1;
    is[i * 6 + 4] = vertex_len - i - 1;
    is[i * 6 + 5] = vertex_len - i - 2;
  }
  for (size_t i = 0, j; i < max_particle; ++i) {
    // random 1 to 0 float
    particles[i].vel = vec2_fromRad(2.f * M_PI * (float)rand() / (float)RAND_MAX);
    particles[i].pos = (vec2){
      2.0f * (float)rand() / (float)RAND_MAX - 1.0f,
      2.0f * (float)rand() / (float)RAND_MAX - 1.0f};
    // velocity around 25 to -25
    vec2_sclf(&particles[i].vel, 50);
    // size 25 - 125
    particles[i].r = 50.f + (100.f * (float)rand() / (float)RAND_MAX);
    // position around inside screen - 2*size
    vec2_scl(&particles[i].pos, vec2_addf(sZ, -particles[i].r));
    // generate mesh
    mesh_vertex *vs = (mesh_vertex *)malloc(vertex_len_byte);
    for (j = 0; j < vertex_len; ++j) {
      float rad = (float)j * M_PI / (float)CIRCLE_PRECISION;
      vs[j].pos = (vec3){particles[i].r * cosf(rad), particles[i].r * sinf(rad), 0.f};
      vs[j].c.u32 = 0xffffffff;
    }
    mesh_index *iss = (mesh_index *)malloc(index_len_byte);
    memcpy(iss, is, index_len_byte);
    particle_meshes[i] = global_engine.genMesh(vs, vertex_len, iss, index_len);
  }
  free(is);
}
mesh *game_update(unsigned int *l, float dt) {
  *l = max_particle;
  vec2 sZ = vec2_mulf(global_engine.getScreenSize(), 0.5f);

  size_t i, j;
  for (i = 0; i < max_particle; ++i) {
    // update motion by velocity / sec
    vec2_trn(&particles[i].pos, vec2_mulf(particles[i].vel, dt));
    // detect with walls
    if (fabs(particles[i].pos.x) > (sZ.x - particles[i].r)) {
      particles[i].vel.x *= -1.0f;
      particles[i].pos.x = (sZ.x - particles[i].r) * (particles[i].pos.x < 0 ? -1.f : 1.f);
    }
    if (fabs(particles[i].pos.y) > (sZ.y - particles[i].r)) {
      particles[i].vel.y *= -1.0f;
      particles[i].pos.y = (sZ.y - particles[i].r) * (particles[i].pos.y < 0 ? -1.f : 1.f);
    }
  }
  for (i = 0; i < max_particle; ++i) {
    // set mesh position
    global_engine.setMeshTransform(particle_meshes[i], (float[]){
                                                         1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         particles[i].pos.x, particles[i].pos.y, 0.f, 1.f});
  }
  return particle_meshes;
}
void game_clean() {
  free(particles);
  for (size_t i = 0; i < max_particle; ++i) {
    global_engine.deleteMesh(particle_meshes[i]);
  }
  free(particle_meshes);
}