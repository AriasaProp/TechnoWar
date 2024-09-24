#ifndef QOI_H
#define QOI_H

#define QOI_SRGB 0
#define QOI_LINEAR 1

struct qoi_desc {
  unsigned int width;
  unsigned int height;
  unsigned char channels;
  unsigned char colorspace;
};

bool qoi_write (const char *filename, const unsigned char *data, const qoi_desc *desc);

unsigned char *qoi_from_asset(const char *filename, qoi_desc *desc, int channels);
unsigned char *qoi_read (const char *filename, qoi_desc *desc, int channels);

unsigned char *qoi_encode (const unsigned char *pixels, const qoi_desc *desc, int *out_len);
unsigned char *qoi_decode (const unsigned char *byte, int size, qoi_desc *desc, int channels);

#endif // QOI_H