#ifndef CODEC_INCLUDED_
#define CODEC_INCLUDED_

#include "common.h"
// all objects
typedef struct {
  uint8_t bpc, channel;
  uint32_t width, height;
} image_info;
typedef struct {
  void *user;
  // is read succes
  int (*read)(void *, void *, size_t);
  void (*seek)(void *, size_t);
  // is end
  int (*eof)(void *);
} callback;

// read image
#ifndef CODEC_IMAGE_IMPLEMENTATIONS_

extern void codec_image_load_fromFile(char const*, image_info*, void**);
extern void codec_image_load_fromCallback(callback, image_info*, void**);

#endif // CODEC_IMAGE_IMPLEMENTATIONS_

#endif // CODEC_INCLUDED_