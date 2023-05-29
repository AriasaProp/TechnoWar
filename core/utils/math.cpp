#include "math.hpp"

#include <cmath>
#include <chrono>
#include <cstring>
#include <sys/resources.h>

static struct rusage myusage;

unsigned long memory_usage::mem_usage () {
  getrusage(RUSAGE_SELF, &myusage);
  return myusage.ru_maxrss;
}

static std::chrono::time_point<std::chrono::high_resolution_clock> start_clock;
static std::chrono::time_point<std::chrono::high_resolution_clock> end_clock;
static float delta_result;
static float delta_count;
static size_t FPS_result;
static size_t frame_count;

void clock_count::start() {
  start_clock = std::chrono::high_resolution_clock::now ();
  frame_count = 0;
  FPS_result = 0;
  delta_count = 0;
  delta_result = 0;
}
void clock_count::render() {
  end_clock = std::chrono::high_resolution_clock::now ();
  delta_result = float (std::chrono::duration_cast<std::chrono::microseconds> (end_clock - start_clock).count ()) / 1000000.f;
  delta_count += delta_result;
  frame_count++;
  if (delta_count >= 1.0f) {
    delta_count -= 1.0f;
    FPS_result = frame_count;
    frame_count = 0;
  }
  start_clock = end_clock;
}
void clock_count::end() {
  frame_count = 0;
  delta_count = 0;
}
size_t clock_count::getFPS() {
  return FPS_result;
}
float clock_count::getDelta() {
  return delta_result;
}

static float tmp[16]{};

//matrix4 definition
void matrix4::idt (float *a) {
  memset (a, 0, sizeof (tmp));
  a[0] = a[5] = a[10] = a[15] = 1;
}
void matrix4::mul (float *a, float *b) {
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
  memcpy (tmp, a, sizeof (tmp));
  a[0] = tmp[0] * pcos * rcos + tmp[4] * (ysin * psin * rcos - ycos * rsin) + tmp[8] * (ycos * psin * rcos + ysin * rsin);
  a[1] = tmp[1] * pcos * rcos + tmp[5] * (ysin * psin * rcos - ycos * rsin) + tmp[9] * (ycos * psin * rcos + ysin * rsin);
  a[2] = tmp[2] * pcos * rcos + tmp[6] * (ysin * psin * rcos - ycos * rsin) + tmp[10] * (ycos * psin * rcos + ysin * rsin);
  a[3] = tmp[3] * pcos * rcos + tmp[7] * (ysin * psin * rcos - ycos * rsin) + tmp[11] * (ycos * psin * rcos + ysin * rsin);
  a[4] = tmp[0] * pcos * rsin + tmp[4] * (ysin * psin * rsin + ycos * rcos) + tmp[8] * (ycos * psin * rsin - ysin * rcos);
  a[5] = tmp[1] * pcos * rsin + tmp[5] * (ysin * psin * rsin + ycos * rcos) + tmp[9] * (ycos * psin * rsin - ysin * rcos);
  a[6] = tmp[2] * pcos * rsin + tmp[6] * (ysin * psin * rsin + ycos * rcos) + tmp[10] * (ycos * psin * rsin - ysin * rcos);
  a[7] = tmp[3] * pcos * rsin + tmp[7] * (ysin * psin * rsin + ycos * rcos) + tmp[11] * (ycos * psin * rsin - ysin * rcos);
  a[8] = (tmp[4] * ysin + tmp[8] * ycos) * pcos - tmp[0] * psin;
  a[9] = (tmp[5] * ysin + tmp[9] * ycos) * pcos - tmp[1] * psin;
  a[10] = (tmp[6] * ysin + tmp[10] * ycos) * pcos - tmp[2] * psin;
  a[11] = (tmp[7] * ysin + tmp[11] * ycos) * pcos - tmp[3] * psin;
}
void matrix4::toOrtho (float *a, float left, float right, float bottom, float top, float near, float far) {
  memset (a, 0, sizeof (tmp));
  a[0] = 2 / (right - left);
  a[5] = 2 / (top - bottom);
  a[10] = 2 / (near - far);
  a[15] = 1;
}
void matrix4::toOrtho2D (float *a, float width, float height) {
  memset (a, 0, sizeof (tmp));
  a[0] = 2 / width;
  a[5] = 2 / height;
  a[10] = 1;
  a[12] = -1;
  a[13] = -1;
  a[14] = -0.001f;
  a[15] = 1;
}
