#define _VEC_IMPLEMENTATION_
#include "math/vec.h"

#include <string.h>
#include <math.h>

// static const uint32_t test_size[(sizeof(float) != 4) * -1];

typedef union {
  float f;
  unsigned int i;
  struct {
    uint8_t s : 1;
    uint8_t e;
    uint32_t m : 23;
  } prs;
} parse_float;

/*
 *      X ^ (0.5)  ->      (M  * 2 ^ E) ^ 0.5
 * log2(X ^ (0.5)) -> (log2(M) +     E) * 0.5
 *
 *
 *
 *
 */

// inverse sqrt by quake III
static inline float inv_sqrt(float v) {
  parse_float w = {.f = v};
  w.i = 0x5f3759df - (w.i >> 1);
  w.f *= 1.5f - 0.5f * v * w.f * w.f;
  return w.f;
}
static inline float qsqrt(float v) {
  // inverse sqrt by quake III
  parse_float w = {.f = v};
  w.i = 0x5f3759df - (w.i >> 1);
  w.f *= 1.5f - 0.5f * v * w.f * w.f;
  return w.f * v;
}

// vec2 math
vec2  vec2_fromRad(float rad) { return (vec2){cosf(rad), sinf(rad)}; }

float vec2_dot(const vec2 a, const vec2 b) { return a.x * b.x + a.y * b.y; }
float vec2_crs(const vec2 a, const vec2 b) { return a.x * b.y - a.y * b.x; }
float vec2_dis(const vec2 a, const vec2 b) {
  float x = a.x - b.x;
  float y = a.y - b.y;
  return qsqrt(x * x + y * y);
}
float vec2_len2(const vec2 a) { return a.x * a.x + a.y * a.y; }
float vec2_len(const vec2 a) { return qsqrt(vec2_len2(a)); }
float vec2_rad(const vec2 a, const vec2 b) {
  /*
   *       ac + bd 
   *       -------
   * √(a2 + b2 + c2 + d2)
   */
  float v = a.x * a.x + a.y * a.y;
  v *= b.x * b.x + b.y * b.y;
  return (a.x * b.x + a.y * b.y) * inv_sqrt(v);
}

vec2  vec2_floor(const vec2 a) { return (vec2){floor(a.x), floor(a.y)}; }
vec2  vec2_addf(const vec2 a, const float f) { return (vec2){ a.x + f, a.y + f}; }
vec2  vec2_subf(const vec2 a, const float f) { return (vec2){ a.x - f, a.y - f}; }
vec2  vec2_mulf(const vec2 a, const float s) { return (vec2){ a.x * s, a.y * s}; }
vec2  vec2_divf(const vec2 a, const float s) { return (vec2){ a.x / s, a.y / s}; }
vec2  vec2_add(const vec2 a, const vec2 b) { return (vec2){a.x + b.x, a.y + b.y}; }
vec2  vec2_sub(const vec2 a, const vec2 b) { return (vec2){ a.x - b.x, a.y - b.y}; }
vec2  vec2_mul(const vec2 a, const vec2 b) {
  return (vec2){
    a.x * b.x,
    a.y * b.y};
}
vec2  vec2_div(const vec2 a, const vec2 b) { return (vec2){ a.x / b.x, a.y / b.y}; }
vec2  vec2_inv(const vec2 a) { return (vec2){1.0f / a.x, 1.0f / a.y}; }
vec2  vec2_norm(const vec2 a) { return vec2_mulf(a, inv_sqrt(vec2_len2(a))); }

void  vec2_floors(vec2 *a) { a->x = floor(a->x), a->y = floor(a->y); }
void  vec2_addsf(vec2 *a, const float f) { a->x += f, a->y += f; }
void  vec2_subsf(vec2 *a, const float f) { a->x -= f, a->y -= f; }
void  vec2_mulsf(vec2 *a, const float s) { a->x *= s, a->y *= s; }
void  vec2_divsf(vec2 *a, const float s) { a->x /= s, a->y /= s; }
void  vec2_adds(vec2 *a, const vec2 b) { a->x += b.x, a->y += b.y; }
void  vec2_subs(vec2 *a, const vec2 b) { a->x -= b.x, a->y -= b.y; }
void  vec2_muls(vec2 *a, const vec2 b) { a->x *= b.x, a->y *= b.y; }
void  vec2_divs(vec2 *a, const vec2 b) { a->x *= b.x, a->y /= b.y; }
void  vec2_invs(vec2 *a) { a->x = 1.0f / a->x, a->y = 1.0f / a->y; }
void  vec2_norms(vec2 *a) { vec2_mulsf(a, inv_sqrt(vec2_len2(*a))); }

// ivec2 math
ivec2 ivec2_add(const ivec2 a, const ivec2 b) { return (ivec2){a.x + b.x, a.y + b.y}; }
ivec2 ivec2_sub(const ivec2 a, const ivec2 b) { return (ivec2){a.x - b.x, a.y - b.y}; }

void ivec2_subs(ivec2 *a, const ivec2 b) { a->x -= b.x, a->y -= b.y; }

// uivec2 math
uivec2 uivec2_add(const uivec2 a, const uivec2 b) { return (uivec2){a.x + b.x, a.y + b.y}; }
uivec2 uivec2_sub(const uivec2 a, const uivec2 b) { return (uivec2){a.x - b.x, a.y - b.y}; }

void uivec2_addsi(uivec2 *a, const uint16_t b) { a->x += b, a->y += b; }
void uivec2_subsi(uivec2 *a, const uint16_t b) { a->x -= b, a->y -= b; }
