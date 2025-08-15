#include <stdio.h>

#include "decoder.h"
#include "stb/stb_image.h"

int main(int argc, char **argv) {
  int x, y, c;
  image_info inf;
  for (size_t i = 1; i < argc; ++i) {

    printf("file: %s\n", argv[i]);
    void *b = decoder_image_load_fromFile(argv[i], &inf);
    if (!b) {
      // printf("Error: %s", decoder_get_error());
      continue;
    }
    free(b);
    void *a = stbi_load(argv[i], &x, &y, &c, 0);
    printf("width: %d - %hhu\n", x, inf.width);
    printf("height: %d - %hhu\n", y, inf.height);
    printf("channel: %d - %hhu\n", c, inf.channel);
    free(a);
  }
}