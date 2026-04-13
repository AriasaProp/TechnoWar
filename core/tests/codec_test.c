#include <stdio.h>

#include "codec/codec.h"
#include "console_util.h"
#include "stb/image_read.h"

int main(int,char**) {
  PRINT_INF("Start Compare read between stb and my reader Test\n");
  int x, y, c, res = 0;
  char filename[256 + 1] = {0};
  image_info inf;
  void *a, *b;
  for (size_t i = 0; i < 6; ++i) {
    snprintf(filename, 256, "tests_data/%02zu.png",i);
    PRINT_INF("file: %s\n", filename);
    codec_image_load_fromFile(filename, &inf, &b);
    if (!b) {
      PRINT_ERR("Failed to decode\n");
      res = 1;
      continue;
    }
    a = stbi_read(filename, &x, &y, &c, 0);
    if (!a) {
      free(b);
      PRINT_ERR("Failed to stb read\n");
      res = 1;
      continue;
    }
    PRINT_INF("width: %d - %hhu\n", x, inf.width);
    PRINT_INF("height: %d - %hhu\n", y, inf.height);
    PRINT_INF("channel: %d - %hhu\n", c, inf.channel);
    if (memcmp(a,b,x*y*c)) {
      PRINT_ERR("Bytes result different:\n");
      res = 1;
    }
    free(b);
    free(a);
  }
  return res;
}