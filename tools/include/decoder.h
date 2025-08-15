#ifndef DECODER_INCLUDED_
#define DECODER_INCLUDED_

#include "common.h"
#include <stdbool.h>
// all objects
typedef struct {
  uint8_t bpc, channel;
  uint32_t width, height;
} image_info;
typedef struct {
  void *user;
  // is read succes
  bool (*read)(void *, void *, size_t);
  void (*seek)(void *, size_t);
  // is end
  bool (*eof)(void *);
} callback;

// read image
#ifndef DECODER_IMAGE_IMPLEMENTATIONS_
extern void *decoder_image_load_fromFile(char const *, image_info *);
extern void *decoder_image_load_fromCallback(callback, image_info *);
#endif // DECODER_IMAGE_IMPLEMENTATIONS_

#if defined(DECODER_IMAGE_IMPLEMENTATIONS_)
extern void decoder_write_error(const char *, ...);
#endif // other decoder

#ifndef DECODER_IMPLEMENTATIONS_
extern const char *decoder_get_error(void);
#endif // DECODER_IMPLEMENTATIONS_

#endif // DECODER_INCLUDED_