#define MATH_UTIL_IMPLEMENTATION_
#include "math_util.h"
#include <math.h>

static inline float inv_sqrt(float v) {
  // inverse sqrt by quake III
  union {
    float f;
    int i;
  } w;
  w.f = v;
  w.i = 0x5f3759df - (w.i >> 1);
  w.f *= 1.5f - 0.5f * v * w.f * w.f;
  return w.f;
}

// vec2 math
vec2 vec2_add(vec2 a, vec2 b) {
  return (vec2){
    a.x + b.x,
    a.y + b.y};
}
vec2 vec2_sub(vec2 a, vec2 b) {
  return (vec2){
    a.x - b.x,
    a.y - b.y};
}
vec2 vec2_addf(vec2 a, float f) {
  return (vec2){
    a.x + f,
    a.y + f};
}

void vec2_trn(vec2 *a, vec2 b) {
  a->x += b.x;
  a->y += b.y;
}
void vec2_trnf(vec2 *a, float f) {
  a->x += f;
  a->y += f;
}

vec2 vec2_mul(vec2 a, vec2 b) {
  return (vec2){
    a.x * b.x,
    a.y * b.y};
}
vec2 vec2_div(vec2 a, vec2 b) {
  return (vec2){
    a.x / b.x,
    a.y / b.y};
}
vec2 vec2_inv(vec2 a) {
  return (vec2){1.0f / a.x, 1.0f / a.y};
}
vec2 vec2_mulf(vec2 a, float s) {
  return (vec2){
    a.x * s,
    a.y * s};
}

void vec2_scl(vec2 *a, vec2 b) {
  a->x *= b.x;
  a->y *= b.y;
}
void vec2_sclf(vec2 *a, float s) {
  a->x *= s;
  a->y *= s;
}

float vec2_dot(vec2 a, vec2 b) {
  return a.x * b.x + a.y * b.y;
}
float vec2_crs(vec2 a, vec2 b) {
  return a.x * b.y - a.y * b.x;
}
float vec2_dist(vec2 a, vec2 b) {
  float x = a.x - b.x;
  float y = a.y - b.y;
  return sqrt(x * x + y * y);
}
float vec2_len(vec2 a) {
  return sqrt(a.x * a.x + a.y * a.y);
}
vec2 vec2_norm(vec2 a) {
  return vec2_mulf(a, inv_sqrt(a.x * a.x + a.y * a.y));
}
float vec2_rad(vec2 a, vec2 b) {
  // inverse sqrt by quake III
  float v = a.x * a.x + a.y * a.y;
  v *= b.x * b.x + b.y * b.y;
  return (a.x * b.x + a.y * b.y) * inv_sqrt(v);
}
vec2 vec2_fromRad(float rad) {
  return (vec2){cosf(rad), sinf(rad)};
}