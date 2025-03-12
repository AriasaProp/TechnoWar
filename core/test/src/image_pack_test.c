#include "stb/stb_rect_pack.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "util.h"
#include "uistage.h"

#include <stdio.h>
#include <string.h>

#define IMAGE_FILES 4
#define IMAGE_CHANNELS 4
#define IMAGE_MAX_SIDE 2048

const char *files[IMAGE_FILES] = {
	"background_label",
	"button_active.9",
	"button_pressed.9",
	"button_disable",
};

int image_pack_test (void) {
	// nothing todo
	printf("Image RectPack test\n");
	char buffer[1024];
	int temp[3];
	
	struct stbrp_rect *rects = (struct stbrp_rect* )malloc(sizeof(struct stbrp_rect )*IMAGE_FILES);
	//generate random rects
	for (int i = 0; i < IMAGE_FILES; ++i) {
		rects[i].was_packed = 0;
		sprintf(buffer, "assets/uiskin/default/%s.png", files[i]);
		ASSERT(stbi_info(buffer, &rects[i].w, &rects[i].h, temp));
		rects[i].name = malloc(strlen(buffer) + 1);
		memcpy(rects[i].name, buffer, strlen(buffer) + 1);
	}
	ASSERT (stbrp_pack_rects(rects, IMAGE_FILES, IMAGE_MAX_SIDE, IMAGE_MAX_SIDE));
	unsigned char *output_image_packs = (unsigned char*)malloc(IMAGE_MAX_SIDE*IMAGE_MAX_SIDE*IMAGE_CHANNELS);
	for (int i = 0; i < IMAGE_FILES; ++i) {
		unsigned char *result = stbi_load(rects[i].name, &temp[0], &temp[1], &temp[2], IMAGE_CHANNELS);
		ASSERT(result);
		printf("loaded %s\n", rects[i].name);
		for(int j = 0; j < rects[i].h; ++j) {
			int xres = rects[i].x * IMAGE_CHANNELS;
			int wcontainer = IMAGE_CHANNELS * IMAGE_MAX_SIDE;
			int yres = rects[i].y;
			int wres = IMAGE_CHANNELS * rects[i].w;
			for (int k = 0; k < rects[i].h; ++k) {
				memcpy(output_image_packs + xres + (rects[i].y + k) * wcontainer, result + (wres * k), wres);
			}
		}
		free(result);
		free(rects[i].name);
	}
	free(rects);
	
	// write to file
	ASSERT(stbi_write_png("uiskin_pack.png", IMAGE_MAX_SIDE, IMAGE_MAX_SIDE, 4, output_image_packs, 0));
	free(output_image_packs);
	
	printf("Image RectPack test end\n");
	return 0;
}