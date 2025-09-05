#include "console_util.h"
#include "stb/stb_truetype.h"
#include "stb/stb_image_write.h"

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define TEXTURE_CHANNEL 2
#define FONT_SCALE 42
#define TEXTURE_SIZE 1024

#define FIRST_CHAR 0x21
#define LAST_CHAR 0x7f
#define LENGTH_CHAR (LAST_CHAR - FIRST_CHAR)

int fast_test(void) {
  struct stat stbuff;
  int result = EXIT_FAILURE;
  unsigned char *mttf;
  {
    int fd = open("raw_asset/fonts/ARIAL.TTF", O_RDONLY);
    if (fd < 0) {
      PRINT_ERR("Failed to open file descriptor to read ttf\n");
      goto main_end;
    }
    if (fstat(fd, &stbuff) < 0) {
      close(fd);
      PRINT_ERR("Failed to gain file stat\n");
      goto main_end;
    }
    mttf = (unsigned char*)mmap(NULL, stbuff.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
  }
  if (MAP_FAILED == mttf) {
    PRINT_ERR("Failed to mapping memory, cause %s\n", strerror(errno));
    goto main_end;
  }
  FILE *fm = fopen("core/font_map.h", "w");
  if (!fm) {
    PRINT_ERR("Failed to write packed font data, cause %s\n", strerror(errno));
    goto main_unmap;
  }
  unsigned char *pixels = (unsigned char*) calloc(TEXTURE_SIZE * TEXTURE_SIZE * TEXTURE_CHANNEL, 1);
  if (NULL == pixels) {
    PRINT_ERR("Failed to allocate memory, cause %s\n", strerror(errno));
    goto main_close;
  }
  stbtt_bakedchar chardata[LENGTH_CHAR] = {0};
  PRINT_INF("Allocated pixels and char data\n");
  {
    if (0 >= fwrite("Hello World\0", 12, 1, fm)) {
      PRINT_ERR("Write file was FAILURE\n");
      goto main_free;
    }
    stbtt_fontinfo f = {0};
    // parse ttf
    if (!stbtt_InitFont(&f, mttf, 0)) {
      PRINT_ERR("Failed to pharse ttf file, cause %s\n", strerror(errno));
      goto main_free;
    }
    int x = 1, y = 1, bottom_y = 1;
    vec2 scale = VEC2_SQR(stbtt_ScaleForPixelHeight(&f, FONT_SCALE));
    int advance,lsb,gw,gh, g;
    ivec2 ip0, ip1;
    for (int i = FIRST_CHAR; i <= LAST_CHAR; ++i) {
      g = stbtt_FindGlyphIndex(&f, i);
      stbtt_GetGlyphHMetrics(&f, g, &advance, &lsb);
      stbtt_GetGlyphBitmapBox(&f, g, scale, &ip0, &ip1);
      gw = ip1.x - ip0.x;
      gh = ip1.y - ip0.y;
      if (x + gw + 1 >= TEXTURE_SIZE) y = bottom_y, x = 1; // advance to next row
      if (y + gh + 1 >= TEXTURE_SIZE) { // check if it fits vertically AFTER potentially moving to next row
        PRINT_ERR("Failed to packing font image at %d\n", i);
        goto main_free;
      }
      ASSERT(x + gw < TEXTURE_SIZE);
      ASSERT(y + gh < TEXTURE_SIZE);
      stbtt_MakeGlyphBitmap(&f, pixels + x + y * TEXTURE_SIZE, gw, gh, TEXTURE_SIZE, scale.x, scale.y, g);
      chardata[i].x0 = (int16_t)x;
      chardata[i].y0 = (int16_t)y;
      chardata[i].x1 = (int16_t)(x + gw);
      chardata[i].y1 = (int16_t)(y + gh);
      chardata[i].xadvance = scale.x * advance;
      chardata[i].xoff = (float)ip0.x;
      chardata[i].yoff = (float)ip0.y;
      x = x + gw + 1;
      if (y + gh + 1 > bottom_y)
        bottom_y = y + gh + 1;
    }
    PRINT_INF("All char is backed remain bottom is %d pixels\n", bottom_y);
  }
  if (!stbi_write_png("build/fontA.png", TEXTURE_SIZE, TEXTURE_SIZE, TEXTURE_CHANNEL, pixels)) {
    PRINT_ERR("Write file was FAILURE\n");
    goto main_free;
  }
  PRINT_INF("Write pixels to file was SUCCESS\n");
  result = EXIT_SUCCESS;
main_free:
  free (pixels);
main_close:
  fclose(fm);
main_unmap:
  munmap(mttf, stbuff.st_size);
main_end:
  return result;
}