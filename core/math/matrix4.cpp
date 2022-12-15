#include "matrix4.h"
#include <cstring>
#include <cmath>

#define M00 0
#define M01 4
#define M02 8
#define M03 12
#define M10 1
#define M11 5
#define M12 9
#define M13 13
#define M20 2
#define M21 6
#define M22 10
#define M23 14
#define M30 3
#define M31 7
#define M32 11
#define M33 15

void matrix4::idt(float *a) {
	memset(a, 0, 16 * sizeof(float));
	a[M00] = a[M11] = a[M22] = a[M33] = 1;
}
void matrix4::mul(float *a, float *b) {
  float tmp[16];
  tmp[M00] = a[M00] * b[M00] + a[M01] * b[M10] + a[M02] * b[M20] + a[M03] * b[M30];
  tmp[M01] = a[M00] * b[M01] + a[M01] * b[M11] + a[M02] * b[M21] + a[M03] * b[M31];
  tmp[M02] = a[M00] * b[M02] + a[M01] * b[M12] + a[M02] * b[M22] + a[M03] * b[M32];
  tmp[M03] = a[M00] * b[M03] + a[M01] * b[M13] + a[M02] * b[M23] + a[M03] * b[M33];
  tmp[M10] = a[M10] * b[M00] + a[M11] * b[M10] + a[M12] * b[M20] + a[M13] * b[M30];
  tmp[M11] = a[M10] * b[M01] + a[M11] * b[M11] + a[M12] * b[M21] + a[M13] * b[M31];
  tmp[M12] = a[M10] * b[M02] + a[M11] * b[M12] + a[M12] * b[M22] + a[M13] * b[M32];
  tmp[M13] = a[M10] * b[M03] + a[M11] * b[M13] + a[M12] * b[M23] + a[M13] * b[M33];
  tmp[M20] = a[M20] * b[M00] + a[M21] * b[M10] + a[M22] * b[M20] + a[M23] * b[M30];
  tmp[M21] = a[M20] * b[M01] + a[M21] * b[M11] + a[M22] * b[M21] + a[M23] * b[M31];
  tmp[M22] = a[M20] * b[M02] + a[M21] * b[M12] + a[M22] * b[M22] + a[M23] * b[M32];
  tmp[M23] = a[M20] * b[M03] + a[M21] * b[M13] + a[M22] * b[M23] + a[M23] * b[M33];
  tmp[M30] = a[M30] * b[M00] + a[M31] * b[M10] + a[M32] * b[M20] + a[M33] * b[M30];
  tmp[M31] = a[M30] * b[M01] + a[M31] * b[M11] + a[M32] * b[M21] + a[M33] * b[M31];
  tmp[M32] = a[M30] * b[M02] + a[M31] * b[M12] + a[M32] * b[M22] + a[M33] * b[M32];
  tmp[M33] = a[M30] * b[M03] + a[M31] * b[M13] + a[M32] * b[M23] + a[M33] * b[M33];
  memcpy(a, tmp, sizeof(float) * 16);
}
void matrix4::rotate(float *a, float yaw, float pitch, float roll) {
  const float ycos = cos(yaw), ysin = sin(yaw), pcos = cos(pitch), psin = sin(pitch), rcos = cos(roll), rsin = sin(roll);
  float t[16];
  memcpy(t, a, 16 * sizeof(float));
  a[M00] = t[M00] * pcos * rcos + t[M01] * (ysin * psin * rcos - ycos * rsin) + t[M02] * (ycos * psin * rcos + ysin * rsin);
  a[M10] = t[M10] * pcos * rcos + t[M11] * (ysin * psin * rcos - ycos * rsin) + t[M12] * (ycos * psin * rcos + ysin * rsin);
  a[M20] = t[M20] * pcos * rcos + t[M21] * (ysin * psin * rcos - ycos * rsin) + t[M22] * (ycos * psin * rcos + ysin * rsin);
  a[M30] = t[M30] * pcos * rcos + t[M31] * (ysin * psin * rcos - ycos * rsin) + t[M32] * (ycos * psin * rcos + ysin * rsin);
  a[M01] = t[M00] * pcos * rsin + t[M01] * (ysin * psin * rsin + ycos * rcos) + t[M02] * (ycos * psin * rsin - ysin * rcos);
  a[M11] = t[M10] * pcos * rsin + t[M11] * (ysin * psin * rsin + ycos * rcos) + t[M12] * (ycos * psin * rsin - ysin * rcos);
  a[M21] = t[M20] * pcos * rsin + t[M21] * (ysin * psin * rsin + ycos * rcos) + t[M22] * (ycos * psin * rsin - ysin * rcos);
  a[M31] = t[M30] * pcos * rsin + t[M31] * (ysin * psin * rsin + ycos * rcos) + t[M32] * (ycos * psin * rsin - ysin * rcos);
  a[M02] = (t[M01] * ysin + t[M02] * ycos) * pcos - t[M00] * psin;
  a[M12] = (t[M11] * ysin + t[M12] * ycos) * pcos - t[M10] * psin;
  a[M22] = (t[M21] * ysin + t[M22] * ycos) * pcos - t[M20] * psin;
  a[M32] = (t[M31] * ysin + t[M32] * ycos) * pcos - t[M30] * psin;
}
void matrix4::toOrtho(float *a, float left, float right, float bottom, float top, float near, float far) {
  memset(a, 0, 16 * sizeof(float));
  a[M00] = 2 / (right - left);
  a[M11] = 2 / (top - bottom);
  a[M22] = 2 / (near - far);
  a[M33] = 1;
}
void matrix4::toOrtho2D(float *a, float width, float height) {
  memset(a, 0, 16 * sizeof(float));
  a[M00] = 2 / width;
  a[M11] = 2 / height;
  a[M22] = 1;
  a[M33] = 1;
  a[M03] = -1;
  a[M13] = -1;
  a[M23] = -0.001f;
}

