#include "stb/stb_rect_pack.h"
#include "util.h"
#include "console_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <timing.h>

int rect_pack_test (void) {
  #define ERROR_MSG_LENGTH 512
	printf("  Rect Pack:");
	char errorMsg[ERROR_MSG_LENGTH] = {0};
	void *timer = start_timing();
	srand(time(0));
	int total_rect = (rand() % 21) + 15;
	struct stbrp_rect *rects = (struct stbrp_rect* )malloc(sizeof(struct stbrp_rect ) * total_rect);
	if (!rects) {
		snprintf(errorMsg, ERROR_MSG_LENGTH,  "memory allocation failed");
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
	size_t min = sqrt(total_area), used_side = min * 1.3f;
	for (; !*errorMsg && (used_side > min); used_side -= (min / 50)) {
    if (stbrp_pack_rects(rects, total_rect, used_side, used_side)) {
      prcnt = (float)used_side;
      prcnt /= (float)min;
      prcnt = 2.0f - prcnt;
      prcnt *= 100.0f;
  	}
	}
	printf ("%.2f%% efficient ", prcnt);
rect_pack_test_end1:
	free(rects);
rect_pack_test_end:
  if (*errorMsg)
	  printf(RED " Error: %s" RESET, errorMsg);
	else
	  printf(GREEN " Done " RESET);
  printf(" %.2f ms\n", end_timing(timer));
	return (*errorMsg != 0);
}