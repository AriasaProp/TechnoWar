#ifndef QOI_H
#define QOI_H

#include <cstdint>
#include <cstdlib>

#define QOI_SRGB 0
#define QOI_LINEAR 1

struct qoi_desc {
  unsigned int width;
  unsigned int height;
  unsigned char channels;
  unsigned char colorspace;
};

void *qoi_encode (const void *, const qoi_desc *, int *);
void *qoi_decode (const void *, size_t, qoi_desc *, int);

#endif // QOI_H