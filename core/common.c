#define COMMON_IMPLEMENTATION_
#include "common.h"

#include <string.h>

typedef union {
  float f;
  unsigned int i;
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

// helper
#ifdef _WIN32
int convert_wchar_to_utf8(char *buffer, size_t bufferlen, const wchar_t *input) {
  return WideCharToMultiByte(65001 /* UTF8 */, 0, input, -1, buffer, (int)bufferlen, NULL, NULL);
}
#endif

void flipBytes(uint8_t *b, const size_t l) {
  if (l <= 1) return;
  for (size_t i = 0, j = l - 1, k = l >> 1; i < k; ++i, --j) {
    b[i] ^= b[j];
    b[j] ^= b[i];
    b[i] ^= b[j];
  }
}


// vec2 math
vec2  vec2_add(vec2 a, vec2 b) { return (vec2){a.x + b.x, a.y + b.y}; }
vec2  vec2_sub(vec2 a, vec2 b) { return (vec2){ a.x - b.x, a.y - b.y}; }
vec2  vec2_addf(vec2 a, float f) { return (vec2){ a.x + f, a.y + f}; }
void  vec2_trn(vec2 *a, vec2 b) {
  a->x += b.x;
  a->y += b.y;
}
void  vec2_trnf(vec2 *a, float f) {
  a->x += f;
  a->y += f;
}
vec2  vec2_mul(vec2 a, vec2 b) {
  return (vec2){
    a.x * b.x,
    a.y * b.y};
}
vec2  vec2_div(vec2 a, vec2 b) { return (vec2){ a.x / b.x, a.y / b.y}; }
vec2  vec2_inv(vec2 a) { return (vec2){1.0f / a.x, 1.0f / a.y}; }
vec2  vec2_mulf(vec2 a, float s) { return (vec2){ a.x * s, a.y * s}; }
void  vec2_scl(vec2 *a, vec2 b) {
  a->x *= b.x;
  a->y *= b.y;
}
void  vec2_sclf(vec2 *a, float s) {
  a->x *= s;
  a->y *= s;
}
float vec2_dot(vec2 a, vec2 b) { return a.x * b.x + a.y * b.y; }
float vec2_crs(vec2 a, vec2 b) { return a.x * b.y - a.y * b.x; }
float vec2_dist(vec2 a, vec2 b) {
  float x = a.x - b.x;
  float y = a.y - b.y;
  return qsqrt(x * x + y * y);
}
float vec2_len(vec2 a) { return qsqrt(a.x * a.x + a.y * a.y); }
vec2  vec2_norm(vec2 a) { return vec2_mulf(a, inv_sqrt(a.x * a.x + a.y * a.y)); }
float vec2_rad(vec2 a, vec2 b) {
  // inverse sqrt by quake III
  float v = a.x * a.x + a.y * a.y;
  v *= b.x * b.x + b.y * b.y;
  return (a.x * b.x + a.y * b.y) * inv_sqrt(v);
}
vec2  vec2_fromRad(float rad) { return (vec2){cosf(rad), sinf(rad)}; }
// matrix4 math
void matrix4_idt(float *m) {
  memset(m, 0, 16 * sizeof(float));
  m[0] = m[5] = m[10] = m[15] = 1.0f;
}
void matrix4_mul(float *a, float *b) {
  float temp[16];
  memset(temp, 0, 16 * sizeof(float));
  for (size_t h = 0; h < 4; ++h) {
    for (size_t v = 0; v < 4; ++v) {
      for (size_t i = 0; i < 4; ++i) {
        temp[i] += a[(h * 4) + i] * b[v + (i * 4)];
      }
    }
  }
  memcpy(a, temp, 16 * sizeof(float));
}
void matrix4_rotateDeg(float *m, vec3 deg) {
  const float degConst = M_PI / 180.0f; 
  float temp[16];
  float yawSin = sin(deg.x * degConst), yawCos = cos(deg.x * degConst);
  float pitchSin = sin(deg.y * degConst), pitchCos = cos(deg.y * degConst);
  float rollSin = sin(deg.z * degConst), rollCos = cos(deg.z * degConst);

  for (size_t i = 0; i < 16; i += 4) {
    temp[i]     = m[i] * pitchCos * rollCos +
                  m[i + 1] * pitchCos * rollSin -
                  m[i + 2] * pitchSin;
    temp[i + 1] = m[i] * (yawSin * pitchSin * rollCos - yawCos * rollSin) +
                  m[i + 1] * (yawSin * pitchSin * rollSin + yawCos * rollCos) +
                  m[i + 2] * (pitchCos * rollSin);
    temp[i + 2] = m[i] * (yawCos * pitchSin * rollCos + yawSin * rollSin) +
                  m[i + 1] * (yawSin * pitchSin * rollCos - yawCos * rollSin) +
                  m[i + 2] * (pitchCos * rollCos);
    temp[i + 3] = m[i + 3];
  }
  memcpy(m, temp, 16 * sizeof(float));
}
