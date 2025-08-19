#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum : uint8_t {
  FONT_BOLD = 1,
  FONT_ITALIC = 2,
  FONT_SMOOTH = 4,
  FONT_AA = 8,
} font_style_flags;

typedef struct {
  char a, b;
  float d;
} kerning;

typedef struct {
  char *name;
  uint8_t flags;
  float size, lineHeight, base;
  struct {
    float x, y, w, h, xoff, yoff, xadv;
  } chs[0xff];
  size_t kerning_length;
  kerning *kearns;
} font_info;

int main(void) {
  FILE *f = fopen("assets/fonts/default/default.fnt", "r");
  if (!f)
    goto error_return;
  char line[512];
  font_info in;
  memset(&in, 0, sizeof(in));
  size_t i;
  int tempi[9];
  while (fgets(line, 511, f)) {
    if (strstr(line, "info")) {
      char tname[64], tchar[64];
      if (9 != sscanf(line, "info face=%s size=%d bold=%d italic=%d charset=%s unicode=%d stretchH=%d smooth=%d aa=%d",
                      tname, tempi, tempi + 1, tempi + 2, tchar, tempi + 3, tempi + 4, tempi + 5, tempi + 6)) {
        printf("info invalid!\n");
        goto error_return;
      }
      in.name = (char *)malloc(strlen(tname) + 1);
      strncpy(in.name, tname, strlen(tname));
      in.flags = tempi[0] * FONT_BOLD | tempi[1] * FONT_ITALIC | tempi[5] * FONT_SMOOTH | tempi[6] * FONT_AA;
    } else if (strstr(line, "common")) {
      if (6 != sscanf(line, "common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=%d packed=%d",
                      tempi, tempi + 1, tempi + 2, tempi + 3, tempi + 4, tempi + 5)) {
        printf("common is invalid!\n");
        goto error_return;
      }
      in.lineHeight = (float)tempi[0];
      in.base = (float)tempi[1];
    } else if (strstr(line, "chars ")) {
      if (!sscanf(line, "chars count=%d", tempi + 8)) {
        printf("chars count can't be found!\n");
        goto error_return;
      }
      for (i = 0; i < tempi[8]; ++i) {
        if (!fgets(line, 511, f) ||
            (8 != sscanf(line, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d",
                         tempi, tempi + 1, tempi + 2, tempi + 3, tempi + 4, tempi + 5, tempi + 6, tempi + 7))) {
          printf("char critical data invalid!\n");
          goto error_return;
        }
        in.chs[tempi[0]].x = (float)tempi[1];
        in.chs[tempi[0]].y = (float)tempi[2];
        in.chs[tempi[0]].w = (float)tempi[3];
        in.chs[tempi[0]].h = (float)tempi[4];
        in.chs[tempi[0]].xoff = (float)tempi[5];
        in.chs[tempi[0]].yoff = (float)tempi[6];
        in.chs[tempi[0]].xadv = (float)tempi[7];
      }
    } else if (strstr(line, "kernings ")) {
      if (!sscanf(line, "kernings count=%lu", &in.kerning_length)) {
        printf("kernings count can't be found!\n");
        goto error_return;
      }
      in.kearns = (kerning *)malloc(sizeof(kerning) * in.kerning_length);
      for (i = 0; i < in.kerning_length; ++i) {
        if (!fgets(line, 511, f) ||
            (3 != sscanf(line, "kerning first=%d second=%d amount=%d", tempi, tempi + 1, tempi + 2))) {
          printf("kerning data invalid can't be found!\n");
          goto error_return;
        }
        in.kearns[i].a = (char)tempi[0];
        in.kearns[i].b = (char)tempi[1];
        in.kearns[i].d = (float)tempi[2];
      }
    }
  }
  {
    printf("name: %s, size: %f, flags: %d, lineHeight: %f, base: %f\n", in.name, in.size, (int)in.flags, in.lineHeight, in.base);
    for (i = 0; i < 0xff; ++i) {
      printf("char: %c, {%f, %f, %f, %f, %f, %f, %f}\n", (char)i,
             in.chs[i].x,
             in.chs[i].y,
             in.chs[i].w,
             in.chs[i].h,
             in.chs[i].xoff,
             in.chs[i].yoff,
             in.chs[i].xadv);
    }
    for (i = 0; i < in.kerning_length; ++i) {
      printf("Kearn: %c-%c : %f\n",
             in.kearns[i].a,
             in.kearns[i].b,
             in.kearns[i].d);
    }
  }
  fclose(f);
  if (in.name)
    free(in.name);
  if (in.kearns)
    free(in.kearns);
  return 0;
error_return:
  if (in.name)
    free(in.name);
  if (in.kearns)
    free(in.kearns);
  return 1;
}