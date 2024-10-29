#ifndef QOI_H
#define QOI_H

#include <cstdint>

#define QOI_SRGB 0
#define QOI_LINEAR 1

struct qoi_desc {
  unsigned int width;
  unsigned int height;
  unsigned char channels;
  unsigned char colorspace;
};

bool qoi_write (const char *, const unsigned char *, const qoi_desc *);
voida *qoi_read (const char *, qoi_desc *, int);
void *qoi_encode (const void *, const qoi_desc *, int *);
void *qoi_decode (const void *, size_t, qoi_desc *, int);

#endif // QOI_H