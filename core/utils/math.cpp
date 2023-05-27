#include "math.hpp"

#include <cmath>
#include <cstring>




//Point definition
template<typename T, size_t N>
Point::Point() {
  for (size_t i = 0; i < N; i++)
    data[i] = T();
}

template<typename T, size_t N>
T &Point::operator[](size_t i) {
  return data[i];
}

template<typename T, size_t N>
const T & Point::operator[](size_t i) const {
  return data[i];
}




//matrix4 definition

void matrix4::idt (float *a) {
  memset (a, 0, 16 * sizeof (float));
  a[0] = a[5] = a[10] = a[15] = 1;
}
void matrix4::mul (float *a, float *b) {
  float tmp[16];
  tmp[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
  tmp[4] = a[0] * b[4] + a[4] * b[5] + a[8] * b[6] + a[12] * b[7];
  tmp[8] = a[0] * b[8] + a[4] * b[9] + a[8] * b[10] + a[12] * b[11];
  tmp[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15];
  tmp[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
  tmp[5] = a[1] * b[4] + a[5] * b[5] + a[9] * b[6] + a[13] * b[7];
  tmp[9] = a[1] * b[8] + a[5] * b[9] + a[9] * b[10] + a[13] * b[11];
  tmp[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
  tmp[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
  tmp[6] = a[2] * b[4] + a[6] * b[5] + a[10] * b[6] + a[14] * b[7];
  tmp[10] = a[2] * b[8] + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
  tmp[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
  tmp[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];
  tmp[7] = a[3] * b[4] + a[7] * b[5] + a[11] * b[6] + a[15] * b[7];
  tmp[11] = a[3] * b[8] + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];
  tmp[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];
  memcpy (a, tmp, sizeof (tmp));
}
void matrix4::rotate (float *a, float yaw, float pitch, float roll) {
  const float ycos = cos (yaw), ysin = sin (yaw), pcos = cos (pitch), psin = sin (pitch), rcos = cos (roll), rsin = sin (roll);
  float t[16];
  memcpy (t, a, sizeof (t));
  a[0] = t[0] * pcos * rcos + t[4] * (ysin * psin * rcos - ycos * rsin) + t[8] * (ycos * psin * rcos + ysin * rsin);
  a[1] = t[1] * pcos * rcos + t[5] * (ysin * psin * rcos - ycos * rsin) + t[9] * (ycos * psin * rcos + ysin * rsin);
  a[2] = t[2] * pcos * rcos + t[6] * (ysin * psin * rcos - ycos * rsin) + t[10] * (ycos * psin * rcos + ysin * rsin);
  a[3] = t[3] * pcos * rcos + t[7] * (ysin * psin * rcos - ycos * rsin) + t[11] * (ycos * psin * rcos + ysin * rsin);
  a[4] = t[0] * pcos * rsin + t[4] * (ysin * psin * rsin + ycos * rcos) + t[8] * (ycos * psin * rsin - ysin * rcos);
  a[5] = t[1] * pcos * rsin + t[5] * (ysin * psin * rsin + ycos * rcos) + t[9] * (ycos * psin * rsin - ysin * rcos);
  a[6] = t[2] * pcos * rsin + t[6] * (ysin * psin * rsin + ycos * rcos) + t[10] * (ycos * psin * rsin - ysin * rcos);
  a[7] = t[3] * pcos * rsin + t[7] * (ysin * psin * rsin + ycos * rcos) + t[11] * (ycos * psin * rsin - ysin * rcos);
  a[8] = (t[4] * ysin + t[8] * ycos) * pcos - t[0] * psin;
  a[9] = (t[5] * ysin + t[9] * ycos) * pcos - t[1] * psin;
  a[10] = (t[6] * ysin + t[10] * ycos) * pcos - t[2] * psin;
  a[11] = (t[7] * ysin + t[11] * ycos) * pcos - t[3] * psin;
}
void matrix4::toOrtho (float *a, float left, float right, float bottom, float top, float near, float far) {
  memset (a, 0, 16 * sizeof (float));
  a[0] = 2 / (right - left);
  a[5] = 2 / (top - bottom);
  a[10] = 2 / (near - far);
  a[15] = 1;
}
void matrix4::toOrtho2D (float *a, float width, float height) {
  memset (a, 0, 16 * sizeof (float));
  a[0] = 2 / width;
  a[5] = 2 / height;
  a[10] = 1;
  a[12] = -1;
  a[13] = -1;
  a[14] = -0.001f;
  a[15] = 1;
}
