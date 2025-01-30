#ifndef QOI_H
#define QOI_H

#define QOI_SRGB 0
#define QOI_LINEAR 1
#include <cstdint>
#include <cstdlib>

struct qoi_desc {
  unsigned int width;
  unsigned int height;
  unsigned char channels;
  unsigned char colorspace;
};

unsigned char *qoi_encode (const unsigned char *pixels, const qoi_desc *desc, size_t *out_len);
unsigned char *qoi_decode (const unsigned char *byte, size_t size, qoi_desc *desc, int channels);

#endif // QOI_H