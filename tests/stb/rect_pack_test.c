#include "common.h"
#include "console_util.h"
#include "stb/stb_rect_pack.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

int rect_pack_test(void) {
  int res = 1;
  PRINT_INF("Starting Rect Pack Test \n");
  srand(time(0));
  int total_rect = (rand() % 21) + 15;
  stbrp_rect *rects = (stbrp_rect *)malloc(sizeof(stbrp_rect) * total_rect);
  if (!rects) {
    PRINT_ERR("Memory allocation failed\n");
    goto rect_pack_test_end;
  }
  size_t total_area = 0;
  for (int i = 0; i < total_rect; ++i) {
    rects[i].w = (rand() % 200) + 15;
    rects[i].h = (rand() % 200) + 15;
    total_area += rects[i].w * rects[i].h;
    rects[i].id = i;
  }
  float prcnt = 0.0f;
  size_t min = sqrt(total_area),
  used_side = min * 1.3f;
  for (; used_side > min; used_side -= (min / 50)) {
    if (stbrp_pack_rects(rects, total_rect, used_side, used_side)) {
      prcnt = (float)used_side;
      prcnt /= (float)min;
      prcnt = 2.0f - prcnt;
      prcnt *= 100.0f;
    }
  }
  PRINT_INF("%.2f%% efficient\n", prcnt);
  free(rects);
  res = 0;
rect_pack_test_end:
  return res;
}