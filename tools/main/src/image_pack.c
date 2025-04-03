#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "stb/stb_rect_pack.h"
#include "uistage.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

#define IMAGE_FILES    4
#define IMAGE_CHANNELS 4
#define IMAGE_MAX_SIDE 2048

const char *files[IMAGE_FILES] = {
  "panel.9",
  "button_life.9",
  "button_pressed.9",
  "button_death.9",
};
const char *android_assets = "../android/src/main/assets/uiskin";
const char *images_assets = "assets/uiskin/default";

int isPNG9Patch(const char *name)
{
  static const char *ex = ".9.png";
  static const size_t ex_len = 6;
  size_t name_len = strlen(name);
  return (name_len >= ex_len) &&
         (memcmp(name + (name_len - ex_len), ex, ex_len) == 0);
}

int image_pack(void)
{
  // nothing todo
  printf("Image RectPack test\n");
  char buffer[2048];
  int ninepatch;
  int temp[3];
  int patch[4] = { 0 }, padding[4] = { 0 };

  struct stbrp_rect *rects = (struct stbrp_rect *)malloc(sizeof(struct stbrp_rect) * IMAGE_FILES);
  if (!rects) {
    printf("memory allocation error\n");
    goto image_pack_test_end;
  }
  // get rects info from image
  for (int i = 0; i < IMAGE_FILES; ++i) {
    sprintf(buffer, "%s/%s.png", images_assets, files[i]);
    rects[i].w = 0;
    if (!stbi_info(buffer, &rects[i].w, &rects[i].h, temp)) {
      printf("stbi load info got error! for file %s because %s\n", buffer, stbi_get_failure_reason());
      goto image_pack_test_end1;
    }
    if (isPNG9Patch(buffer)) {
      rects[i].w -= 2;
      rects[i].h -= 2;
    }
    rects[i].id = i;
  }
  if (!stbrp_pack_rects(rects, IMAGE_FILES, IMAGE_MAX_SIDE, IMAGE_MAX_SIDE)) {
    printf("rect pack is failed, %d^2 is not fit anymore\n", IMAGE_MAX_SIDE);
    goto image_pack_test_end1;
  }
  sprintf(buffer, "%s/pack_list.txt", android_assets);
  FILE *pack_list = fopen(buffer, "w");
  if (!pack_list) {
    printf("file I/O %s error\n", buffer);
    goto image_pack_test_end1;
  }
  int wcontainer = IMAGE_CHANNELS * IMAGE_MAX_SIDE;
  unsigned char *output_image_packs = (unsigned char *)malloc(IMAGE_MAX_SIDE * wcontainer);
  for (int i = 0; i < IMAGE_FILES; ++i) {
    sprintf(buffer, "%s/%s.png", images_assets, files[rects[i].id]);
    unsigned char *result = stbi_load(buffer, &temp[0], &temp[1], &temp[2], IMAGE_CHANNELS);
    if (!result || temp[2] != IMAGE_CHANNELS) {
      printf("fail to load %s/%s.png or channels is %d != 4\n", images_assets, files[rects[i].id], temp[2]);
      continue;
    }
    fprintf(pack_list, "%u: %d %d %d %d",
            rects[i].id,
            rects[i].x, rects[i].y, rects[i].w, rects[i].h);
    ninepatch = isPNG9Patch(buffer);
    int xcontainer = IMAGE_CHANNELS * rects[i].x;
    for (int j = 0; j < rects[i].h; ++j) {
      memmove(
        output_image_packs + xcontainer + (rects[i].y + j) * wcontainer,
        result + (IMAGE_CHANNELS * (temp[0] * (j + ninepatch) + ninepatch)),
        IMAGE_CHANNELS * rects[i].w);
    }
    if (ninepatch) {
      // horizontal patch
      fprintf(pack_list, ", ");
      {
        uint32_t current = 0u;
        int count = 0;
        for (int i = 1, j = temp[0] - 1; i < j; ++i) {
          if (memcmp(&current, result + (i * IMAGE_CHANNELS), IMAGE_CHANNELS)) {
            if (count)
              fprintf(pack_list, "%d ", count);
            count = 0;
            memcpy(&current, result + (i * IMAGE_CHANNELS), IMAGE_CHANNELS);
          }
          ++count;
        }
        fprintf(pack_list, "%d ", count);
      }
      // vertical patch
      fprintf(pack_list, ", ");
      {
        uint32_t current = 0u;
        int count = 0;
        int wres = temp[0] * IMAGE_CHANNELS;
        for (int i = 1, j = temp[1] - 1; i < j; ++i) {
          if (memcmp(&current, result + (i * wres), IMAGE_CHANNELS)) {
            if (count)
              fprintf(pack_list, "%d ", count);
            count = 0;
            memcpy(&current, result + (i * wres), IMAGE_CHANNELS);
          }
          ++count;
        }
        fprintf(pack_list, "%d ", count);
      }
      // horizontal padding
      fprintf(pack_list, ", ");
      {
        uint32_t current = 0u;
        int count = 0;
        unsigned char *res = result + ((temp[1] - 1) * temp[0] * IMAGE_CHANNELS);
        for (int i = 1, j = temp[0] - 1; i < j; ++i) {
          if (memcmp(&current, res + (i * IMAGE_CHANNELS), IMAGE_CHANNELS)) {
            if (count)
              fprintf(pack_list, "%d ", count);
            count = 0;
            memcpy(&current, res + (i * IMAGE_CHANNELS), IMAGE_CHANNELS);
          }
          ++count;
        }
        fprintf(pack_list, "%d ", count);
      }
      // vertical padding
      fprintf(pack_list, ", ");
      {
        uint32_t current = 0u;
        int count = 0;
        unsigned char *res = result + ((temp[0] - 1) * IMAGE_CHANNELS);
        int wres = temp[0] * IMAGE_CHANNELS;
        for (int i = 1, j = temp[1] - 1; i < j; ++i) {
          if (memcmp(&current, res + (i * wres), IMAGE_CHANNELS)) {
            if (count)
              fprintf(pack_list, "%d ", count);
            count = 0;
            memcpy(&current, res + (i * wres), IMAGE_CHANNELS);
          }
          ++count;
        }
        fprintf(pack_list, "%d ", count);
      }
    }
    free(result);
    fprintf(pack_list, "\n");
  }
  fclose(pack_list);
  // write to file
  sprintf(buffer, "%s/packed.png", android_assets);
  if (!stbi_write_png(buffer, IMAGE_MAX_SIDE, IMAGE_MAX_SIDE, 4, output_image_packs))
    printf("failed to write packed image\n");
  free(output_image_packs);
  free(rects);
  printf("Image RectPack test END\n");
  return 0;
image_pack_test_end1:
  free(rects);
image_pack_test_end:
  printf("Image RectPack test ERROR\n");
  return 1;
}