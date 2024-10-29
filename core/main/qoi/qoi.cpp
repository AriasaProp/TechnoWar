#include "qoi.hpp"
#include "../engine.hpp"
#include "../utils/value.hpp"
#include <cstring>

#define QOI_OP_INDEX 0x00 /* 00xxxxxx */
#define QOI_OP_DIFF 0x40  /* 01xxxxxx */
#define QOI_OP_LUMA 0x80  /* 10xxxxxx */
#define QOI_OP_RUN 0xc0   /* 11xxxxxx */
#define QOI_OP_RGB 0xfe   /* 11111110 */
#define QOI_OP_RGBA 0xff  /* 11111111 */

#define QOI_MASK_2 0xc0 /* 11000000 */

#define QOI_COLOR_HASH(C) (C.rgba.r * 3 + C.rgba.g * 5 + C.rgba.b * 7 + C.rgba.a * 11)
#define QOI_MAGIC                                  \
  (((uint32_t)'q') << 24 | ((uint32_t)'o') << 16 | \
   ((uint32_t)'i') << 8 | ((uint32_t)'f'))
#define QOI_HEADER_SIZE 14

/* 2GB is the max file size that this implementation can safely handle. We guard
against anything larger than that, assuming the worst case with 5 bytes per
pixel, rounded down to a nice clean value. 400 million pixels ought to be
enough for anybody. */
#define QOI_PIXELS_MAX ((uint32_t)400000000)

static const unsigned char qoi_padding[8] = {0, 0, 0, 0, 0, 0, 0, 1};

static void qoi_write_32 (unsigned char **bytes, uint32_t v) {
  *((*bytes)++) = (0xff000000 & v) >> 24;
  *((*bytes)++) = (0x00ff0000 & v) >> 16;
  *((*bytes)++) = (0x0000ff00 & v) >> 8;
  *((*bytes)++) = (0x000000ff & v);
}

static uint32_t qoi_read_32 (const unsigned char **bytes) {
  uint32_t a = *((*bytes)++);
  uint32_t b = *((*bytes)++);
  uint32_t c = *((*bytes)++);
  uint32_t d = *((*bytes)++);
  return a << 24 | b << 16 | c << 8 | d;
}

void *qoi_encode (const void *p_, const qoi_desc *desc, size_t *out_len) {
  if (!p_ || !desc || !desc->width || !desc->height ||
      desc->channels < 3 || desc->channels > 4 || desc->colorspace > 1 ||
      desc->height >= QOI_PIXELS_MAX / desc->width) {
    return NULL;
  }

  const unsigned char *pixels = (const unsigned char *)p_;
  void *result = malloc (desc->width * desc->height * (desc->channels + 1) + QOI_HEADER_SIZE + sizeof (qoi_padding));
  
  /*
  size_t i, run;
  size_t px_len, px_end, px_pos, channels;
  color32_t index[64];
  color32_t px, px_prev;


  unsigned char *bytes = (unsigned char *)result;

  qoi_write_32 (&bytes, QOI_MAGIC);
  qoi_write_32 (&bytes, desc->width);
  qoi_write_32 (&bytes, desc->height);
  *(bytes++) = desc->channels;
  *(bytes++) = desc->colorspace;

  memset (index, 0, sizeof (index));

  run = 0;
  px_prev.rgba = {0, 0, 0, 255};
  px = px_prev;

  px_len = desc->width * desc->height * desc->channels;
  px_end = px_len - desc->channels;
  channels = desc->channels;

  for (px_pos = 0; px_pos < px_len; px_pos += channels) {
    px.rgba.r = pixels[px_pos + 0];
    px.rgba.g = pixels[px_pos + 1];
    px.rgba.b = pixels[px_pos + 2];

    if (channels == 4) {
      px.rgba.a = pixels[px_pos + 3];
    }

    if (px.color == px_prev.color) {
      run++;
      if (run == 62 || px_pos == px_end) {
        *(bytes++) = QOI_OP_RUN | (run - 1);
        run = 0;
      }
    } else {
      int index_pos;

      if (run > 0) {
        *(bytes++) = QOI_OP_RUN | (run - 1);
        run = 0;
      }

      index_pos = QOI_COLOR_HASH (px) % 64;

      if (index[index_pos].color == px.color) {
        *(bytes++) = QOI_OP_INDEX | index_pos;
      } else {
        index[index_pos] = px;

        if (px.rgba.a == px_prev.rgba.a) {
          signed char vr = px.rgba.r - px_prev.rgba.r;
          signed char vg = px.rgba.g - px_prev.rgba.g;
          signed char vb = px.rgba.b - px_prev.rgba.b;

          signed char vg_r = vr - vg;
          signed char vg_b = vb - vg;

          if (
              vr > -3 && vr < 2 &&
              vg > -3 && vg < 2 &&
              vb > -3 && vb < 2) {
            *(bytes++) = QOI_OP_DIFF | (vr + 2) << 4 | (vg + 2) << 2 | (vb + 2);
          } else if (
              vg_r > -9 && vg_r < 8 &&
              vg > -33 && vg < 32 &&
              vg_b > -9 && vg_b < 8) {
            *(bytes++) = QOI_OP_LUMA | (vg + 32);
            *(bytes++) = (vg_r + 8) << 4 | (vg_b + 8);
          } else {
            *(bytes++) = QOI_OP_RGB;
            *(bytes++) = px.rgba.r;
            *(bytes++) = px.rgba.g;
            *(bytes++) = px.rgba.b;
          }
        } else {
          *(bytes++) = QOI_OP_RGBA;
          *(bytes++) = px.rgba.r;
          *(bytes++) = px.rgba.g;
          *(bytes++) = px.rgba.b;
          *(bytes++) = px.rgba.a;
        }
      }
    }
    px_prev = px;
  }

  for (i = 0; i < sizeof (qoi_padding); i++) {
    *(bytes++) = qoi_padding[i];
  }
  *out_len = bytes - (unsigned char *)result;
  result = realloc (result, *out_len);
  */
  return result;
}

void *qoi_decode (const void *b_, size_t size, qoi_desc *desc, int channels) {
  if (!b_ || !desc ||
      (channels && channels != 3 && channels != 4) ||
      size < QOI_HEADER_SIZE + (int)sizeof (qoi_padding)) {
    return NULL;
  }
  const unsigned char *bytes = (const unsigned char *)b_;
  const unsigned char *bytes_end = bytes + size;
  
  color32_t index[64];
  color32_t px;
  int px_len, px_pos;
  int run = 0;

  if (
      qoi_read_32 (&bytes) != QOI_MAGIC ||
      !(desc->width = qoi_read_32 (&bytes)) ||
      !(desc->height = qoi_read_32 (&bytes)) ||
      (desc->channels = *(bytes++)) ||
      (desc->channels < 3) ||
      (desc->channels > 4) ||
      (desc->colorspace = *(bytes++)) > 1 ||
      desc->height >= QOI_PIXELS_MAX / desc->width) {
    return NULL;
  }

  if (!channels) channels = desc->channels;

  px_len = desc->width * desc->height * channels;
  unsigned char *pixels = (unsigned char *)malloc (px_len);
  /*

  memset (index, 0, sizeof (index));
  px.rgba = {0, 0, 0, 255};

  for (px_pos = 0; px_pos < px_len; px_pos += channels) {
    if (run > 0) {
      run--;
    } else if (bytes < bytes_end) {
      int b1 = *(bytes++);

      if (b1 == QOI_OP_RGB) {
        px.rgba.r = *(bytes++);
        px.rgba.g = *(bytes++);
        px.rgba.b = *(bytes++);
      } else if (b1 == QOI_OP_RGBA) {
        px.rgba.r = *(bytes++);
        px.rgba.g = *(bytes++);
        px.rgba.b = *(bytes++);
        px.rgba.a = *(bytes++);
      } else if ((b1 & QOI_MASK_2) == QOI_OP_INDEX) {
        px = index[b1];
      } else if ((b1 & QOI_MASK_2) == QOI_OP_DIFF) {
        px.rgba.r += ((b1 >> 4) & 0x03) - 2;
        px.rgba.g += ((b1 >> 2) & 0x03) - 2;
        px.rgba.b += (b1 & 0x03) - 2;
      } else if ((b1 & QOI_MASK_2) == QOI_OP_LUMA) {
        int b2 = *(bytes++);
        int vg = (b1 & 0x3f) - 32;
        px.rgba.r += vg - 8 + ((b2 >> 4) & 0x0f);
        px.rgba.g += vg;
        px.rgba.b += vg - 8 + (b2 & 0x0f);
      } else if ((b1 & QOI_MASK_2) == QOI_OP_RUN) {
        run = (b1 & 0x3f);
      }

      index[QOI_COLOR_HASH (px) % 64] = px;
    }
    pixels[px_pos + 0] = px.rgba.r;
    pixels[px_pos + 1] = px.rgba.g;
    pixels[px_pos + 2] = px.rgba.b;

    if (channels == 4) {
      pixels[px_pos + 3] = px.rgba.a;
    }
  }
  */
  return pixels;
}
