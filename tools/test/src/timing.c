#include "timing.h"
#include <time.h>

static const float unit_time = 1000.0f / CLOCKS_PER_SEC;

struct timing start_timing() {
  struct timing t;
  t.p = clock();
  return t;
}
float end_timing(struct timing c) {
  return (float)(clock() - c.p) * unit_time;
}