#include "console_util.h"
#include "stb/truetype.h"
#include "stb/rectpack.h"
#include "stb/local.h"
#include "stb/image_write.h"

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define TEXTURE_CHANNEL 1
#define FONT_SCALE 42
#define TEXTURE_SIZE 512
#define FONT_HEADER

#define FIRST_CHAR 0x21
#define LAST_CHAR 0x7f
#define LENGTH_CHAR (LAST_CHAR - FIRST_CHAR + 1)

int main(int,char**) {
  struct stat stbuff;
  int result = EXIT_FAILURE;
  uint8_t *mttf;
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
    mttf = (uint8_t*)mmap(NULL, stbuff.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
  }
  if (MAP_FAILED == mttf) {
    PRINT_ERR("Failed to mapping memory, cause %s\n", strerror(errno));
    goto main_end;
  }
#ifdef FONT_HEADER
  FILE *fm = fopen("bin/font_map.h", "w+");
  if (!fm) {
    PRINT_ERR("Failed to write packed font data, cause %s\n", strerror(errno));
    goto main_unmap;
  }
#endif // FONT_HEADER

  uint8_t *pixels = (uint8_t*)calloc(TEXTURE_SIZE * TEXTURE_SIZE, TEXTURE_CHANNEL);
  stbrp_rect *rect_chars = (stbrp_rect*)malloc(LENGTH_CHAR * sizeof(stbrp_rect));
  PRINT_INF("Allocated pixels and char data\n");
  stbtt_fontinfo f = {0};
  // parse ttf
  if (!stbtt_InitFont(&f, mttf, 0)) {
    PRINT_ERR("Failed to pharse ttf file, cause %s\n", strerror(errno));
    goto main_close;
  }
  ivec2 ip, is;
  vec2 scale = VEC2_SQR(stbtt_ScaleForPixelHeight(&f, FONT_SCALE));
  int advance, lsb, g;
  // load all font rect info
  for (uint8_t i = 0; i <= LENGTH_CHAR; ++i) {
    g = stbtt_FindGlyphIndex(&f, FIRST_CHAR + i);
    stbtt_GetGlyphHMetrics(&f, g, &advance, &lsb);
    stbtt_GetGlyphBitmapBox(&f, g, scale, &ip, &is);
    ivec2_subs(&is, ip);
    rect_chars[i].id = i;
    rect_chars[i].w = is.x;
    rect_chars[i].h = is.y;
    // data_chars[i].xadvance = scale.x * advance;
    // data_chars[i].off.x = ip.x;
    // data_chars[i].off.y = ip.y;
  }
  if (!stbrp_pack_rects(rect_chars, LENGTH_CHAR, TEXTURE_SIZE, TEXTURE_SIZE)) {
    PRINT_ERR("Failed to packing\n");
    goto main_close;
  }
  // backed all font rect info
  stbtt__bitmap gbm = { .stride = TEXTURE_SIZE * TEXTURE_CHANNEL };
  for (uint8_t i = 0; i <= LENGTH_CHAR; ++i) {
    g = stbtt_FindGlyphIndex(&f, FIRST_CHAR + i);
    // data_chars[i].p0.x = rect_chars[i].x;
    // data_chars[i].p0.y = rect_chars[i].y;
    // data_chars[i].p0 = data_chars[i].p1;
    gbm.size.x = rect_chars[i].w;
    gbm.size.y = rect_chars[i].h;
    // data_chars[i].p1.x += rect_chars[i].w;
    // data_chars[i].p1.y += rect_chars[i].h;
    gbm.pixels = pixels + (rect_chars[i].x + rect_chars[i].y * TEXTURE_SIZE);
    stbtt_MakeGlyphBitmap(&f, gbm, scale, g);
  }
  
  PRINT_INF("All char is backed in pixels\n");
  if (!stbi_write_png("bin/output/fontTest.png", TEXTURE_SIZE, TEXTURE_SIZE, TEXTURE_CHANNEL, pixels)) {
    PRINT_ERR("Write file was FAILURE, cause %s\n", stb_get_error());
    goto main_close;
  }
  PRINT_INF("Write pixels to file was "GREEN"SUCCESS"RESET"\n");
  result = EXIT_SUCCESS;
main_close:
#ifdef FONT_HEADER
  fclose(fm);
main_unmap:
#endif // FONT_HEADER
  munmap(mttf, stbuff.st_size);
main_end:
  return result;
}