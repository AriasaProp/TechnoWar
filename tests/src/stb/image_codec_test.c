#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "console_util.h"
#include "timing.h"

#include <string.h>
#include <stdio.h>

#define FILE_TEST "data"
#define TEST_COUNT 6

int image_codec_test() {
	printf("Image Codec Test: ");
	struct timing timer = start_timing();
	char buffer[256];
	int temp[4];
	for (int i = 0; i < TEST_COUNT; ++i) {
		stbi_clean_failure();
		sprintf(buffer, "%s/%02d.png", FILE_TEST, i);
		FILE *imf = fopen(buffer, "rb");
		if (!imf) {
			printf("\n failed load IO %s", buffer);
			goto image_codec_test_err;
		}
		fseek(imf, 0, SEEK_END);
		size_t mem_len = ftell(imf);
		fseek(imf, 0, SEEK_SET);
		unsigned char *im_mem = (unsigned char*)malloc(mem_len);
		if (im_mem) {
			for (size_t i = 0; i < mem_len; )
				i += fread(im_mem + i, 1, mem_len - i, imf);
			fclose(imf);
		} else {
			fclose(imf);
			printf("\n failed allocate memory %s", buffer);
			goto image_codec_test_err;
		}
		unsigned char *load = stbi_load_from_memory(im_mem, mem_len, temp, temp + 1, temp + 2, 0);
		if (load) {
			unsigned char *encode = stbi_write_png_to_mem(load, temp[0], temp[1], temp[2], temp + 3);
			if (encode) {
				if (temp[3] != mem_len) {
					printf(" failed result len %d != %ld %s", temp[3], mem_len, buffer);
				} else if (memcmp(encode, im_mem, mem_len)) {
					printf(" failed different result %s", buffer);
				}
				free (encode);
			} else printf("\n failed write %s", buffer);
			free (load);
			free(im_mem);
		} else {
			free(im_mem);
			printf("\n failed load %s cause %s", buffer, stbi_get_failure_reason());
			goto image_codec_test_err;
		}
	}
	printf(GREEN " END" RESET " %.2f ms\n", end_timing(timer));
	return 0;
image_codec_test_err:
	printf(RED " Failure" RESET " %.2f ms\n", end_timing(timer));
	return 1;
}