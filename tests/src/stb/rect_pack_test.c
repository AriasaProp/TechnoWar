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
	total_area = (size_t) (sqrt(total_area) * 1.3);
	if (!stbrp_pack_rects(rects, total_rect, total_area, total_area)) {
		snprintf(errorMsg, ERROR_MSG_LENGTH,  "rect is not fit");
		printf("\n   Rect %zu^2: \n", total_area);
		for (int i = 0; i < total_rect; ++i)
			printf("    %d: {%d, %d, %d, %d}\n", i, rects[i].x, rects[i].y, rects[i].w, rects[i].h);
	}
rect_pack_test_end1:
	free(rects);
rect_pack_test_end:
  if (*errorMsg)
	  printf(RED "\n   Failure: %s" RESET, errorMsg);
	else
	  printf(GREEN "\n   END " RESET);
  printf(" %.2f ms\n", end_timing(timer));
	return (*errorMsg != 0);
}