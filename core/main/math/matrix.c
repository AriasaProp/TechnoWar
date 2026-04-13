#define _MATRIX_IMPLEMENTATION_
#include "math/matrix.h"

#define MATRIX4_SIZE 16
#define MATRIX4_LEN   4
// matrix4 math
void matrix4_idt(float *m) {
  memset(m, 0, MATRIX4_SIZE * sizeof(float));
  m[0] = m[5] = m[10] = m[15] = 1.0f;
}
void matrix4_mul(float *a, const float *b) {
  float temp[MATRIX4_SIZE];
  memset(temp, 0, MATRIX4_SIZE * sizeof(float));
  for (size_t h = 0; h < MATRIX4_LEN; ++h) {
    for (size_t v = 0; v < MATRIX4_LEN; ++v) {
      for (size_t i = 0; i < MATRIX4_LEN; ++i) {
        temp[i] += a[(h * MATRIX4_LEN) + i] * b[v + (i * MATRIX4_LEN)];
      }
    }
  }
  memcpy(a, temp, MATRIX4_SIZE * sizeof(float));
}
void matrix4_rotateDeg(float *m, vec3 deg) {
  const float degConst = M_PI / 180.0f; 
  float temp[MATRIX4_SIZE] = {0};
  float yawSin = sin(deg.x * degConst), yawCos = cos(deg.x * degConst);
  float pitchSin = sin(deg.y * degConst), pitchCos = cos(deg.y * degConst);
  float rollSin = sin(deg.z * degConst), rollCos = cos(deg.z * degConst);

  for (size_t i = 0; i < MATRIX4_SIZE; i += MATRIX4_LEN) {
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
  memcpy(m, temp, MATRIX4_SIZE * sizeof(float));
}
