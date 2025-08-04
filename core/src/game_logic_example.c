#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "engine.h"
#include "math/vec_math.h"

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
  particle_meshes = (mesh*) malloc(sizeof(mesh)*max_particle);
  vec2 sZ = vec2_mulf(global_engine.g.getScreenSize(), 0.5f);
  // duplicate common use
  size_t vertex_len = CIRCLE_PRECISION * 2;
  size_t index_len = vertex_len * 3;
  mesh_index *is = (mesh_index *)malloc(sizeof(mesh_index) * index_len);
  for (mesh_index i = 0, a = 0; i < CIRCLE_PRECISION; ++i) {
    mesh_index j = i + 1, k = vertex_len - 1;
    is[a++] = j;
    is[a++] = i;
    is[a++] = k;
    is[a++] = j;
    is[a++] = k;
    is[a++] = k - 1;
  }
  for (size_t i = 0; i < max_particle; ++i) {
    // random 1 to 0 float
    particles[i].vel = (vec2){
      (float)rand() / (float)RAND_MAX,
      (float)rand() / (float)RAND_MAX};
    particles[i].pos = (vec2){
      2.0f * (float)rand() / (float)RAND_MAX - 1.0f,
      2.0f * (float)rand() / (float)RAND_MAX - 1.0f};
    // velocity around 25 to -25
    vec2_sclf(&particles[i].vel, 50);
    vec2_trnf(&particles[i].vel, -25);
    // size 50 - 200 (square)
    particles[i].r = 50.f + (150.f * (float)rand() / (float)RAND_MAX);
    // position around inside screen - 2*size
    vec2_scl(&particles[i].pos, vec2_addf(sZ, -particles[i].r));
    // generate mesh
    mesh_vertex *vs = (mesh_vertex *)malloc(sizeof(mesh_vertex) * vertex_len);
    for (size_t j = 0; j < vertex_len; ++j) {
      float rad = j * M_PI / CIRCLE_PRECISION;
      vs[j] = (mesh_vertex){
        .pos = (vec3){
          particles[i].r * cosf(rad),
          particles[i].r * sinf(rad),
          0.f},
        .c.u32 = 0xffffffff};
    }
    mesh_index *iss = (mesh_index *)malloc(sizeof(mesh_index) * index_len);
    memcpy(iss, is, sizeof(mesh_index) * index_len);
    particle_meshes[i] = global_engine.g.genMesh(vs, vertex_len, iss, index_len);
  }
  free(is);
}
mesh *game_update(unsigned int *l, float dt) {
  *l = max_particle;
  vec2 sZ = global_engine.g.getScreenSize();
  size_t i, j;
  for (i = 0; i < max_particle; ++i) {
    // update motion by velocity / sec
    vec2_trn(&particles[i].pos, vec2_mulf(particles[i].vel, dt));
    // detect with walls
    if ((particles[i].pos.x <= particles[i].r) ||
        (particles[i].pos.x + particles[i].r >= sZ.x)) {
      particles[i].vel.x *= -1.0f;
      particles[i].pos.x = CLAMP(particles[i].r + 7e-12, particles[i].pos.x, sZ.x - particles[i].r - 7e-12);
    }
    if ((particles[i].pos.y <= particles[i].r) ||
        (particles[i].pos.y + particles[i].r >= sZ.y)) {
      particles[i].vel.y *= -1.0f;
      particles[i].pos.y = CLAMP(particles[i].r + 7e-12, particles[i].pos.y, sZ.x - particles[i].r - 7e-12);
    }
  }
  for (i = 0; i < max_particle; ++i) {
    // set mesh position
    global_engine.g.setMeshTransform(particle_meshes[i], (float[]){
                                                           1.f, 0.f, 0.f, 0.f,
                                                           0.f, 1.f, 0.f, 0.f,
                                                           0.f, 0.f, 1.f, 0.f,
                                                           particles[i].pos.x, particles[i].pos.y, 0.f, 1.f});
  }
  return particle_meshes;
}
void game_clean() {
  (void)0;
  free(particles);
  for (size_t i = 0; i < max_particle; ++i) {
    global_engine.g.deleteMesh(particle_meshes[i]);
  }
  free(particle_meshes);
}