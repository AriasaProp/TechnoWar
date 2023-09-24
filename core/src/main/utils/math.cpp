#include "math.hpp"
// math is not multithread safe.
#include <chrono>
#include <cmath>
#include <cstring>
#include <sys/resource.h>

// global temporary for not safe thread
static union {
  float mtx[16];
  struct {
    float x, y, w, h;
  } rct;
  struct {
    float x, y;
  } vec;
  struct rusage myusage;
} tmp;

unsigned long memory_usage::mem_usage () {
  getrusage (RUSAGE_SELF, &tmp.myusage);
  return tmp.myusage.ru_maxrss;
}

static std::chrono::time_point<std::chrono::steady_clock> start_clock;
static float delta_result;
static float delta_count;
static size_t FPS_result;
static size_t frame_count;

void clock_count::start () {
  start_clock = std::chrono::steady_clock::now ();
  frame_count = 0;
  FPS_result = 0;
  delta_count = 0;
  delta_result = 0;
}
void clock_count::render () {
  static std::chrono::time_point<std::chrono::steady_clock> temp_clock;
  temp_clock = std::chrono::steady_clock::now ();
  delta_result = static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(temp_clock - start_clock).count ()) / 1000000.f;
  delta_count += delta_result;
  frame_count++;
  if (delta_count >= 1.0f) {
    delta_count -= 1.0f;
    FPS_result = frame_count;
    frame_count = 0;
  }
  start_clock = temp_clock;
}
void clock_count::end () {
  frame_count = 0;
  delta_count = 0;
}
size_t clock_count::getFPS () {
  return FPS_result;
}
float clock_count::getDelta () {
  return delta_result;
}
// Rect definition
Rect::Rect () : xmin (0.0f), ymin (0.0f), xmax (0.0f), ymax (0.0f) {}
Rect::Rect (float x, float y, const Align &a, float sx, float sy) {
  unsigned char vert = a & 3;
  switch (vert) {
  default:
    xmin = x;
    xmax = x + sx;
    break;
  case 1:
    xmin = x - sx * 0.5f;
    xmax = xmin + sx;
    break;
  case 2:
    xmin = x - sx;
    xmax = x;
    break;
  }
  unsigned char hor = (a >> 2) & 3;
  switch (hor) {
  default:
    ymin = y;
    ymax = y + sy;
    break;
  case 1:
    ymin = y - sy * 0.5f;
    ymax = ymin + sy;
    break;
  case 2:
    ymin = y - sy;
    ymax = y;
    break;
  }
}
bool Rect::insetOf (float x, float y) {
  return (x > xmin) && (x < xmax) && (y > ymin) && (y < ymax);
}
// matrix4 definition
void matrix4::idt (float *a) {
  memset (a, 0, sizeof (tmp.mtx));
  a[0] = a[5] = a[10] = a[15] = 1;
}
/*
a    b    c    d       q    r    s    t       aq+er+is+mt   bq+fr+js+nt   cq+gr+ks+ot   dq+hr+ls+pt
e    f    g    h   *   u    v    w    x   *   au+ev+iw+mx    bq+fr+js+nt   cq+gr+ks+ot   dq+hr+ls+pt
i    j    k    l   *   y    z    A    B   *    aq+er+is+mt   bq+fr+js+nt   cq+gr+ks+ot   dq+hr+ls+pt
m    n    o    p       C    D    E    F        aq+er+is+mt   bq+fr+js+nt   cq+gr+ks+ot   dq+hr+ls+pt
*/
void matrix4::mul (float *a, float *b) {
  tmp.mtx[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
  tmp.mtx[4] = a[0] * b[4] + a[4] * b[5] + a[8] * b[6] + a[12] * b[7];
  tmp.mtx[8] = a[0] * b[8] + a[4] * b[9] + a[8] * b[10] + a[12] * b[11];
  tmp.mtx[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15];
  tmp.mtx[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
  tmp.mtx[5] = a[1] * b[4] + a[5] * b[5] + a[9] * b[6] + a[13] * b[7];
  tmp.mtx[9] = a[1] * b[8] + a[5] * b[9] + a[9] * b[10] + a[13] * b[11];
  tmp.mtx[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
  tmp.mtx[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
  tmp.mtx[6] = a[2] * b[4] + a[6] * b[5] + a[10] * b[6] + a[14] * b[7];
  tmp.mtx[10] = a[2] * b[8] + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
  tmp.mtx[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
  tmp.mtx[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];
  tmp.mtx[7] = a[3] * b[4] + a[7] * b[5] + a[11] * b[6] + a[15] * b[7];
  tmp.mtx[11] = a[3] * b[8] + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];
  tmp.mtx[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];
  memcpy (a, tmp.mtx, sizeof (tmp.mtx));
}
void matrix4::rotate (float *a, float yaw, float pitch, float roll) {
  const float ycos = cos (yaw), ysin = sin (yaw), pcos = cos (pitch), psin = sin (pitch), rcos = cos (roll), rsin = sin (roll);
  memcpy (tmp.mtx, a, sizeof (tmp.mtx));
  a[0] = tmp.mtx[0] * pcos * rcos + tmp.mtx[4] * (ysin * psin * rcos - ycos * rsin) + tmp.mtx[8] * (ycos * psin * rcos + ysin * rsin);
  a[1] = tmp.mtx[1] * pcos * rcos + tmp.mtx[5] * (ysin * psin * rcos - ycos * rsin) + tmp.mtx[9] * (ycos * psin * rcos + ysin * rsin);
  a[2] = tmp.mtx[2] * pcos * rcos + tmp.mtx[6] * (ysin * psin * rcos - ycos * rsin) + tmp.mtx[10] * (ycos * psin * rcos + ysin * rsin);
  a[3] = tmp.mtx[3] * pcos * rcos + tmp.mtx[7] * (ysin * psin * rcos - ycos * rsin) + tmp.mtx[11] * (ycos * psin * rcos + ysin * rsin);
  a[4] = tmp.mtx[0] * pcos * rsin + tmp.mtx[4] * (ysin * psin * rsin + ycos * rcos) + tmp.mtx[8] * (ycos * psin * rsin - ysin * rcos);
  a[5] = tmp.mtx[1] * pcos * rsin + tmp.mtx[5] * (ysin * psin * rsin + ycos * rcos) + tmp.mtx[9] * (ycos * psin * rsin - ysin * rcos);
  a[6] = tmp.mtx[2] * pcos * rsin + tmp.mtx[6] * (ysin * psin * rsin + ycos * rcos) + tmp.mtx[10] * (ycos * psin * rsin - ysin * rcos);
  a[7] = tmp.mtx[3] * pcos * rsin + tmp.mtx[7] * (ysin * psin * rsin + ycos * rcos) + tmp.mtx[11] * (ycos * psin * rsin - ysin * rcos);
  a[8] = (tmp.mtx[4] * ysin + tmp.mtx[8] * ycos) * pcos - tmp.mtx[0] * psin;
  a[9] = (tmp.mtx[5] * ysin + tmp.mtx[9] * ycos) * pcos - tmp.mtx[1] * psin;
  a[10] = (tmp.mtx[6] * ysin + tmp.mtx[10] * ycos) * pcos - tmp.mtx[2] * psin;
  a[11] = (tmp.mtx[7] * ysin + tmp.mtx[11] * ycos) * pcos - tmp.mtx[3] * psin;
}
void matrix4::toOrtho (float *a, float left, float right, float bottom, float top, float near, float far) {
  memset (a, 0, sizeof (tmp.mtx));
  a[0] = 2 / (right - left);
  a[5] = 2 / (top - bottom);
  a[10] = 2 / (near - far);
  a[15] = 1;
}
void matrix4::toOrtho2D (float *a, float width, float height) {
  memset (a, 0, sizeof (tmp.mtx));
  a[0] = 2 / width;
  a[5] = 2 / height;
  a[10] = 1;
  a[12] = -1;
  a[13] = -1;
  a[14] = -0.001f;
  a[15] = 1;
}
