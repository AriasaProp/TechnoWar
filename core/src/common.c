#define COMMON_IMPLEMENTATION_
#include "common.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// helper
#ifdef _WIN32
int convert_wchar_to_utf8(char *buffer, size_t bufferlen, const wchar_t *input) {
  return WideCharToMultiByte(65001 /* UTF8 */, 0, input, -1, buffer, (int)bufferlen, NULL, NULL);
}
#endif

// math
void flipBytes(uint8_t *b, const size_t l) {
  if (l <= 1)
    return;
  for (size_t i = 0, j = l - 1, k = l >> 1; i < k; ++i, --j) {
    b[i] ^= b[j];
    b[j] ^= b[i];
    b[i] ^= b[j];
  }
}
int lrotl(int x, size_t n) {
#if (defined(__GNUC__) || defined(__clang__)) && __has_builtin(__builtin_rotateleft32) // GCC 12+ / Clang 14+
  return __builtin_rotateleft32(x, n);
#elif defined(_MSC_VER)
  return _lrotl(x, n);
#else
  return (x << n) | (x >> (-n & 31));
#endif
}
int lrotr(int x, size_t n) {
#if (defined(__GNUC__) || defined(__clang__)) && __has_builtin(__builtin_rotateright32) // GCC 12+ / Clang 14+
  return __builtin_rotateright32(x, n);
#elif defined(_MSC_VER)
  return _lrotr(x, n);
#else
  return (x >> n) | (x << (-n & 31));
#endif
}
void matrix4_idt(float *m) {
  memset(m, 0, 16 * sizeof(float));
  m[0] = m[5] = m[10] = m[15] = 1.0f;
}
void matrix4_mul(float *a, float *b) {
  static float temp[16];
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
  static float temp[16];
  // yaw
  float yawSin = sin(deg.x / 180.0f * M_PI), yawCos = cos(deg.x / 180.0f * M_PI);
  // pitch
  float pitchSin = sin(deg.y / 180.0f * M_PI), pitchCos = cos(deg.y / 180.0f * M_PI);
  // roll
  float rollSin = sin(deg.z / 180.0f * M_PI), rollCos = cos(deg.z / 180.0f * M_PI);

  for (size_t i = 0; i < 16; i += 4) {
    temp[i] = m[i] * pitchCos * rollCos + m[i + 1] * pitchCos * rollSin - m[i + 2] * pitchSin;
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