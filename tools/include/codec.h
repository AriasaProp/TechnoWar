#ifndef CODEC_INCLUDED_
#define CODEC_INCLUDED_

#include "util.h"

extern const char *codec_get_error(void);

typedef struct {
  uint8_t bit_depth, channel;
  uint32_t width, height;
} image_info;
// read image by filename
extern void *image_read(char const *, image_info *);

#endif // CODEC_INCLUDED_