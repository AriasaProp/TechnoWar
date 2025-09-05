#define DECODER_IMAGE_IMPLEMENTATIONS_
#include "decoder.h"

#include <stdio.h>
#include <string.h>

// ***
// useful tool
#define BUFFER_SIZE64 128
typedef union {
  uint8_t u8_[BUFFER_SIZE64 << 3];
  uint16_t u16_[BUFFER_SIZE64 << 2];
  uint32_t u32_[BUFFER_SIZE64 << 1];
  uint64_t u64_[BUFFER_SIZE64];
} buffer_;

// TODO: make decoder for all image

static void *load_png(callback f, image_info *out, bool *sig) {
#define MAX_CHUNK_TYPE 256
  static const uint32_t crc_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0eDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D};
  size_t i, j, k;
  buffer_ buff;
  void *ret = NULL;
  uint8_t plte_len = 0;
  uint8_t *plte = NULL;
  uint8_t *idat = NULL;
  size_t idat_len = 0;
  size_t chunk_added = 0;
  uint32_t once_chunk[MAX_CHUNK_TYPE] = {0};
  uint32_t crc;
#define CRC_APPEND(X) crc = (crc >> 8) ^ crc_table[X ^ (crc & 0xff)]
#define CRC_APPENDS(X, Y) \
  for (k = 0; k < Y; ++k) \
  CRC_APPEND(X[i])
#define COMBO(A, B, C, D)   (uint32_t)(A | (B << 8) | (C << 16) | (D << 24))
#define COMBINE(A, B, C, D) case COMBO(A, B, C, D)
#define ERROR_PNG(...)                \
  do {                                \
    decoder_write_error(__VA_ARGS__); \
    goto end_load_png;                \
  } while (0)

#define READ_PNG(X)                    \
  do {                                 \
    if (!f.read(f.user, buff.u8_, X))  \
      ERROR_PNG("PNG: error on read length %zu", X); \
  } while (0)

#define CHUNK_CHECK(X, Y)                                   \
  do {                                                      \
    uint32_t *x = (uint32_t *)X;                            \
    for (i = 0; i < chunk_added; ++i) {                     \
      for (j = 0; j < Y; ++j) {                             \
        if (once_chunk[i] == x[j]) {                        \
          ERROR_PNG("PNG: Chunk was showed invalid %x", X); \
        }                                                   \
      }                                                     \
    }                                                       \
  } while (0)

  // check signature
  READ_PNG(8);
  *sig = !memcmp(buff.u8_, "\x89PNG\r\n\x1A\n", 8);
  if (!*sig)
    goto end_load_png;

  do {
    READ_PNG(8);
    crc = ~0u;
    CRC_APPENDS((buff.u8_ + 4), 4);
    switch (buff.u32_[1]) {
      COMBINE('I', 'H', 'D', 'R') : {
        // IHDR showd once and first
        if (once_chunk[0]) {
          if (once_chunk[0] != buff.u32_[1]) {
            ERROR_PNG("PNG: IHDR not showed first");
          } else {
            ERROR_PNG("PNG: IHDR showed twice");
          }
        }
        // IHDR len must be 13
        flipBytes(buff.u8_, 4);
        if (buff.u32_[0] != 13) {
          ERROR_PNG("PNG: IHDR length is %d not 13", buff.u32_[0]);
        }
        once_chunk[0] = buff.u32_[1];

        READ_PNG(buff.u32_[0] + 4);
        flipBytes(buff.u8_, 8);
        out->height = buff.u32_[0];
        out->width = buff.u32_[1];
        out->bpc = buff.u8_[9];
        if (out->bpc < 8) {
          ERROR_PNG("PNG: i not ready for %zu bit depth < 8", out->bpc);
        }
        // allow 8, 16
        if (!out->bpc || (out->bpc > 16) || (out->bpc & (out->bpc - 1))) {
          ERROR_PNG("PNG: %d bit-depth is invalid", out->bpc);
        }
        // valid pair of color_type & bpc
        uint8_t color_type = buff.u8_[10];
        if (color_type) {
          if (color_type & 1) {
            if (out->bpc & 16) {
              ERROR_PNG("PNG: %d depth %d color type is not valid", out->bpc, color_type);
            }
          } else {
            if (out->bpc & 7) {
              ERROR_PNG("PNG: %d depth %d color type is not valid", out->bpc, color_type);
            }
          }
        }
        out->channel = 1 + (color_type & 2) + ((color_type >> 2) & 1);
        ret = malloc(out->width * out->height * out->bpc * out->channel / 8);
        // todo: compress support
        if (buff.u8_[11]) {
          ERROR_PNG("PNG: not ready for compressed method");
        }
        // todo: filter support
        if (buff.u8_[12]) {
          ERROR_PNG("PNG: not ready for filter method");
        }
        uint8_t interlace = buff.u8_[13];
        // todo: interlace support
        if (interlace > 1) {
          ERROR_PNG("PNG: not ready for interlace %d method", interlace);
        }
        // todo: need crc care
        CRC_APPENDS(buff.u8_, 13);
        break;
      } // header
      COMBINE('C', 'g', 'B', 'I') : {
        // iphone png format
        // no detail
        // skip for now with crc, is iphone
        f.seek(f.user, buff.u32_[0] + 4);
        continue;
      } // skip now
      COMBINE('c', 'H', 'R', 'M') : {
        // before PLTE & IDAT
        CHUNK_CHECK("PLTE"
                    "IDAT"
                    "cHRM",
                    3);
        break;
      } // not yet
      COMBINE('c', 'I', 'C', 'P') : {
        // before PLTE & IDAT
        break;
      } // not yet
      COMBINE('g', 'A', 'M', 'A') : {
        // before PLTE & IDAT
        break;
      } // not yet
      COMBINE('i', 'C', 'C', 'P') : {
        // before PLTE & IDAT
        // sRGB not present
        break;
      } // not yet
      COMBINE('m', 'D', 'C', 'V') : {
        // before PLTE & IDAT
        break;
      } // not yet
      COMBINE('c', 'L', 'L', 'I') : {
        // before PLTE & IDAT
        break;
      } // not yet
      COMBINE('s', 'B', 'I', 'T') : {
        // before PLTE & IDAT
        break;
      } // not yet
      COMBINE('s', 'R', 'G', 'B') : {
        // before PLTE & IDAT
        // iCCP not present
        break;
      } // not yet
      COMBINE('P', 'L', 'T', 'E') : {
        CHUNK_CHECK("PLTE"
                    "IDAT",
                    2);
        if ((buff.u32_[0] > 256 * 3) || (buff.u32_[0] % 3)) {
          ERROR_PNG("PNG: invalid PLTE");
        }
        plte_len = buff.u32_[0] / 3;
        plte = (uint8_t *)malloc(4 * plte_len);
        memset(plte, 0xff, plte_len * 4);
        for (i = 0, j = 4 * plte_len; i < j; i += 4) {
          if (!f.read(f.user, plte + i, 3)) {
            ERROR_PNG("PNG: error on read");
          }
          CRC_APPENDS((plte + i), 3);
        }
        break;
      }
      COMBINE('b', 'K', 'G', 'D') : {
        // after PLTE before IDAT
        break;
      } // not yet
      COMBINE('h', 'I', 'S', 'T') : {
        // after PLTE before IDAT
        break;
      } // not yet
      COMBINE('t', 'R', 'N', 'S') : {
        // after PLTE if exists or before IDAT
        CHUNK_CHECK("tRNS"
                    "IDAT",
                    2);
        if (plte) {
          if (plte_len > buff.u32_[0]) {
            ERROR_PNG("PNG: bad tRNS len");
          }
          for (i = 3, j = plte_len * 4 + 3; i < j; i += 4) {
            uint8_t *pltec = plte + i;
            if (!f.read(f.user, pltec, 1)) {
              ERROR_PNG("PNG: error on read");
            }
            CRC_APPEND(*pltec);
          }
          out->channel += 1;
          ret = realloc(ret, out->width * out->height * out->bpc * out->channel / 8);
        } else {
          if (!(out->channel & 1)) {
            ERROR_PNG("PNG: color type not match for tRNS");
          }
          if (buff.u32_[0] != (out->channel * 2)) {
            ERROR_PNG("PNG: bad tRNS len");
          }
          // TODO: here is why i am not ready for 1,2,4 bit depth
          uint8_t bytes = out->bpc / 8; // only 1 or 2
          uint8_t *trns = (uint8_t *)malloc(bytes * out->channel);
          READ_PNG(2 * out->channel);
          for (i = 0; i < out->channel; ++i) {
            flipBytes(buff.u8_ + (i << 1), 2);
            memcpy(trns + (i * bytes), buff.u16_ + i, bytes);
          }
          CRC_APPENDS(buff.u8_, 2 * out->channel);
          uint8_t *u8ret = (uint8_t *)ret;
          for (i = 0; i < (out->width * out->height); ++i) {
            memcpy(u8ret + (i * bytes * out->channel), trns, bytes * out->channel);
          }
        }
        break;
      }
      COMBINE('e', 'X', 'I', 'f') : {
        // before IDAT
        break;
      } // not yet
      COMBINE('f', 'c', 'T', 'L') : {
        // first one before IDAT, others shall after IDAT
        // may multiple
        break;
      } // not yet
      COMBINE('a', 'c', 'T', 'L') : {
        // before IDAT
        break;
      } // not yet
      COMBINE('p', 'H', 'Y', 's') : {
        // before IDAT
        break;
      } // not yet
      COMBINE('s', 'P', 'L', 'T') : {
        // before IDAT
        // may multiple
        break;
      } // not yet
      COMBINE('I', 'D', 'A', 'T') : {
        // may multiple
        uint8_t *cidat;
        if (!idat) {
          idat = (uint8_t *)malloc(buff.u32_[0]);
          idat_len = buff.u32_[0];
          cidat = idat;
        } else {
          idat = (uint8_t *)realloc(idat, buff.u32_[0] + idat_len);
          cidat = idat + idat_len;
          idat_len += buff.u32_[0];
        }
        if (!cidat) {
          ERROR_PNG("PNG: out of memory");
        }
        if (!f.read(f.user, cidat, buff.u32_[0])) {
          ERROR_PNG("PNG: error on read");
        }
        CRC_APPENDS(cidat, buff.u32_[0]);
        break;
      } // not yet
      COMBINE('f', 'd', 'A', 'T') : {
        // after IDAT
        break;
      } // not yet
      COMBINE('t', 'I', 'M', 'E') : {
        f.seek(f.user, buff.u32_[0] + 4);
        continue;
      }
      COMBINE('i', 'T', 'X', 't') : {
        f.seek(f.user, buff.u32_[0] + 4);
        continue;
      }
      COMBINE('t', 'E', 'X', 't') : {
        f.seek(f.user, buff.u32_[0] + 4);
        continue;
      }
      COMBINE('z', 'T', 'X', 't') : {
        f.seek(f.user, buff.u32_[0] + 4);
        continue;
      }
      COMBINE('I', 'E', 'N', 'D') : {
        goto calc_load_png;
      }
    default: { // unknown
      ERROR_PNG("PNG: unkown chunk");
    }
    }
    READ_PNG(4);
    flipBytes(buff.u8_, 4);
    if (crc ^ ~buff.u32_[0])
      ERROR_PNG("PNG: crc invalid");
  } while (f.eof(f.user));
calc_load_png:

end_load_png:
  return ret;
#undef CHUNK_ONCE
#undef READ_PNG
#undef COMBINE
#undef ERROR_PNG

#undef CRC_APPENDS
#undef CRC_APPEND

#undef MAX_CHUNK_TYPE
}

void *decoder_image_load_fromCallback(callback f, image_info *out) {
  bool sig = false;
  void *ret = NULL;

  ret = load_png(f, out, &sig);
  if (sig)
    goto end_main_load;
  // stb : 406
  /*
  if (stbi__png_test(s))  return stbi__png_load(s,x,y,comp,req_comp, ri);

  if (stbi__bmp_test(s))  return stbi__bmp_load(s,x,y,comp,req_comp, ri);
  if (stbi__psd_test(s))  return stbi__psd_load(s,x,y,comp,req_comp, ri, bpc);
  if (stbi__pic_test(s))  return stbi__pic_load(s,x,y,comp,req_comp, ri);
  if (stbi__jpeg_test(s)) return stbi__jpeg_load(s,x,y,comp,req_comp, ri);
  if (stbi__pnm_test(s))  return stbi__pnm_load(s,x,y,comp,req_comp, ri);
  if (stbi__hdr_test(s)) {
     float *hdr = stbi__hdr_load(s, x,y,comp,req_comp, ri);
     return stbi__hdr_to_ldr(hdr, *x, *y, req_comp ? req_comp : *comp);
  }
  if (stbi__tga_test(s)) return stbi__tga_load(s,x,y,comp,req_comp, ri);
*/
end_main_load:
  return ret;
}

static bool f_read(void *user, void *data, size_t size) {
  return fread(data, 1, size, (FILE *)user) == size;
}
static void f_seek(void *user, size_t n) {
  int ch;
  fseek((FILE *)user, n, SEEK_CUR);
  ch = fgetc((FILE *)user); /* have to read a byte to reset feof()'s flag */
  if (ch != EOF) {
    ungetc(ch, (FILE *)user); /* push byte back onto stream if valid. */
  }
}
static bool f_eof(void *user) {
  return feof((FILE *)user) || ferror((FILE *)user);
}
void *decoder_image_load_fromFile(char const *filename, image_info *out) {
  FILE *f;
#if defined(_WIN32)
  wchar_t wMode[64];
  wchar_t wFilename[1024];
  if ((!MultiByteToWideChar(65001, 0, filename, -1, wFilename, sizeof(wFilename) / sizeof(*wFilename))) ||
      (!MultiByteToWideChar(65001, 0, "rb", -1, wMode, sizeof(wMode) / sizeof(*wMode))))
    return 0;
#if defined(_MSC_VER) && _MSC_VER >= 1400
  if (_wfopen_s(&f, wFilename, wMode))
    f = 0;
#else
  f = _wfopen(wFilename, wMode);
#endif

#elif defined(_MSC_VER) && _MSC_VER >= 1400
  if (fopen_s(&f, filename, "rb"))
    f = 0;
#else
  f = fopen(filename, "rb");
#endif
  void *ret = NULL;
  if (f) {
    callback c = {.user = (void *)f, .read = f_read, .seek = f_seek, .eof = f_eof};
    ret = decoder_image_load_fromCallback(c, out);
    fclose(f);
  } else
    decoder_write_error("decoder cannot open %s", filename);
  return ret;
}
