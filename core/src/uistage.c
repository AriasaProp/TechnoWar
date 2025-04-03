#include "uistage.h"
#include "engine.h"
#include "util.h"

texture texture_pack;
static struct images
{
  int x, y, w, h;
  int *horizontal_patch;
  int *vertical_patch;
  int padding[4];
} *image_packs = NULL;

void uistage_init()
{
  image_packs = (struct images *)malloc(sizeof(struct images) * IMAGE_TYPE_TOTAL);
  /*
  // load uiskin
  {
          char *buf;
          int len;
          int asset = get_engine ()->a.assetBuffer("uiskin/pack_list.txt", (void**)&buf, &len);
          char *read = buf;
          do {
                  struct images im;
                  int id;
                  sscanf(buf, "%d: %d %d %d %d", &id, &im.x, &im.y, &im.w, &im.h);

                  image_packs[id] = im;
          }
          get_engine ()->a.assetClose(asset);
  }
  */
}
void uistage_term()
{
  free(image_packs);
}