#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "console_util.h"
#include "timing.h"

#include <string.h>
#include <stdio.h>

#define TEST_COUNT 6

int image_codec_test() {
  #define ERROR_MSG_LENGTH 512
	printf("  Image Codec:\n");
	void *timer = start_timing();
	char buffer[256];
	char errorMsg[ERROR_MSG_LENGTH] = {0};
	int temp[4];
	for (int i = 0; (i < TEST_COUNT) && (!*errorMsg); ++i) {
		stbi_clean_failure();
		sprintf(buffer, "tests/data/%02d.png", i);
		FILE *imf = fopen(buffer, "rb");
		if (!imf) {
			snprintf(errorMsg, ERROR_MSG_LENGTH, "failed load I/O file %s", buffer);
			continue;
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
			snprintf(errorMsg, ERROR_MSG_LENGTH,  "failed allocate memory for image %s", buffer);
			continue;
		}
		printf("  - %s: ", buffer);
		unsigned char *load = stbi_load_from_memory(im_mem, mem_len, temp, temp + 1, temp + 2, 0);
		if (load) {
			unsigned char *encode = stbi_write_png_to_mem(load, temp[0], temp[1], temp[2], temp + 3);
			if (encode) {
				if (temp[3] != mem_len) {
				  float prcnt = (float)mem_len;
				  prcnt = (float)temp[3]/prcnt;
				  prcnt *= 100.0f;
					printf("different length %.2f %%", prcnt);
				} else if (memcmp(encode, im_mem, mem_len))
					printf("different data");
				else
			    printf("no different");
				free (encode);
			} else {
			  printf("failed write");
			}
			free (load);
			free(im_mem);
		} else {
			free(im_mem);
			printf("failed load %s cause %s", buffer, stbi_get_failure_reason());
		}
	  printf("\n");
	}
  if (*errorMsg) printf(RED "   Failure: %s" RESET, errorMsg);
	else printf(GREEN "   END " RESET);
	printf("%.2f ms\n", end_timing(timer));
	return (*errorMsg != 0);
}