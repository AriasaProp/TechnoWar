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
	printf("Rect Pack test: ");
	struct timing timer = start_timing();
	int end = 0;
	srand(time(0));
	int total_rect = (rand() % 21) + 15;
	struct stbrp_rect *rects = (struct stbrp_rect* )malloc(sizeof(struct stbrp_rect ) * total_rect);
	if (!rects) {
		printf("\nrects memory allocation failed ");
		end = 1;
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
		printf("\nrect pack is failed, %zu^2 is not fit \n", total_area);
		for (int i = 0; i < total_rect; ++i) {
			printf("{%d, %d, %d, %d}\n", rects[i].x, rects[i].y, rects[i].w, rects[i].h);
		}
		end = 1;
		goto rect_pack_test_end1;
	}
rect_pack_test_end1:
	free(rects);
rect_pack_test_end:
	if (end)
		printf(RED " Failure" RESET " %.2f ms\n", end_timing(timer));
	else
		printf(GREEN " END" RESET " %.2f ms\n", end_timing(timer));
	return end;
}