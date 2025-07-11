#include "timing.h"
#include <time.h>

void *start_timing() { return (void *)clock(); }
float end_timing(void *c) {
  static const float unit_time = 1000.0f / CLOCKS_PER_SEC;
  return (float)(clock() - (clock_t)c) * unit_time;
}