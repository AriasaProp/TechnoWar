/*
 *  Doing test on stb image result
 *  write and read
 *
 *
 */
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "console_util.h"

#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
  #define ERROR_MSG_LENGTH 512
	printf("Image Codec:\n");
	char errorMsg[ERROR_MSG_LENGTH] = {0};
	int temp[4];
	if (argc <= 1) return 0;
	for (int i = 1; (i < argc) && (!*errorMsg); ++i) {
		printf("  - %s: ", argv[i]);
		unsigned char *load = stbi_load (argv[i], temp, temp + 1, temp + 2, 0);
		if (!load) {
			snprintf(errorMsg, ERROR_MSG_LENGTH, "failed load %s cause %s", argv[i], stbi_get_failure_reason());
		  stbi_clean_failure();
		  continue;
		}
	  // write to png
	  unsigned char *writed = stbi_write_png_to_mem(load, temp[0], temp[1], temp[2], temp+3);
	  if (!writed) {
		  snprintf(errorMsg, ERROR_MSG_LENGTH, "failed write %s", argv[i]);
		  free (load);
		  continue;
	  }
    // to pixel again
    unsigned char *bitmap = stbi_load_from_memory (writed, temp[3], temp, temp + 1, temp + 2, 0);
    if (!bitmap) {
		  snprintf(errorMsg, ERROR_MSG_LENGTH, "failed load from memory %s", argv[i]);
    } else {
      if (memcmp(bitmap, load, temp[0]*temp[1]*temp[2])) {
		    snprintf(errorMsg, ERROR_MSG_LENGTH, "result different %s", argv[i]);
      } else {
        printf("Success");
      }
      free (bitmap);
    }
    free (writed);
		free (load);
	  printf("\n");
	}
  if (*errorMsg) printf(RED "   Failure: %s" RESET, errorMsg);
	else printf(GREEN "   END " RESET);
  printf("\n");
	return (*errorMsg != 0);
}