#include <stdio.h>

#include "decoder.h"
#include "console_util.h"
#include "stb/stb_image.h"

int decoder_test(void) {
  PRINT_INF("Start Decoder Test\n");
  int x, y, c, res = 0;
  char filename[256 + 1] = {0};
  image_info inf;
  for (size_t i = 0; i < 6; ++i) {
    snprintf(filename, 256, "tests_data/%02zu.png",i);
    PRINT_INF("file: %s\n", filename);
    void *b = decoder_image_load_fromFile(filename, &inf);
    if (!b) {
      PRINT_ERR("Failed to decode cause, %s\n", decoder_get_error());
      res = 1;
      continue;
    }
    free(b);
    void *a = stbi_load(filename, &x, &y, &c, 0);
    PRINT_INF("width: %d - %hhu\n", x, inf.width);
    PRINT_INF("height: %d - %hhu\n", y, inf.height);
    PRINT_INF("channel: %d - %hhu\n", c, inf.channel);
    free(a);
  }
  return res;
}