#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

#include <cstdlib>

namespace stbi {
enum channel {
  none = 0, // only used for desired_channels
  grey = 1,
  grey_alpha = 2,
  rgb = 3,
  rgb_alpha = 4
};

struct io_callbacks {
  int (*read) (void *user, char *data, unsigned int size); // fill 'data' with 'size' bytes.  return number of bytes actually read
  void (*skip) (void *user, int n);                        // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
  bool (*eof) (void *user);                                // returns nonzero if we are at end of file/data
};
// 8-bit per channels
unsigned char *load_from_memory (unsigned char const *, int, int *, int *, int *, int);
unsigned char *load_from_callbacks (io_callbacks const *, void *, int *, int *, int *, int);
// 16-bit per channels
unsigned short *load_16_from_memory (unsigned char const *, int, int *, int *, int *, int);
unsigned short *load_16_from_callbacks (io_callbacks const *, void *, int *, int *, int *, int);
#ifdef STBI_WINDOWS_UTF8
int convert_wchar_to_utf8 (char *, size_t, const wchar_t *);
#endif

#ifndef STBI_NO_LINEAR
float *loadf_from_memory (unsigned char const *, int, int *, int *, int *, int);
float *loadf_from_callbacks (io_callbacks const *, void *, int *, int *, int *, int);
#endif

#ifndef STBI_NO_HDR
void hdr_to_ldr_gamma (float);
void hdr_to_ldr_scale (float);
#endif // STBI_NO_HDR

#ifndef STBI_NO_LINEAR
void ldr_to_hdr_gamma (float);
void ldr_to_hdr_scale (float);
#endif // STBI_NO_LINEAR

// stbi_is_hdr is always defined, but always returns false if STBI_NO_HDR
int is_hdr_from_callbacks (stbi::io_callbacks const *, void *);
int is_hdr_from_memory (unsigned char const *, int);

// get a VERY brief reason for failure
// on most compilers (and ALL modern mainstream compilers) this is threadsafe
const char *failure_reason (void);

// free the loaded image -- this is just free()
void image_free (void *);

// get image dimensions & components without fully decoding
int info_from_memory (unsigned char const *, int, int *, int *, int *);
int info_from_callbacks (stbi::io_callbacks const *, void *, int *, int *, int *);
int is_16_bit_from_memory (unsigned char const *, int);
int is_16_bit_from_callbacks (stbi::io_callbacks const *, void *);

// for image formats that explicitly notate that they have premultiplied alpha,
// we just return the colors as stored in the file. set this flag to force
// unpremultiplication. results are undefined if the unpremultiply overflow.
void set_unpremultiply_on_load (int);

// indicate whether we should process iphone images back to canonical format,
// or just pass them through "as-is"
void convert_iphone_png_to_rgb (int);

// flip the image vertically, so the first pixel in the output array is the bottom left
void set_flip_vertically_on_load (int);

// as above, but only applies to images loaded on the thread that calls the function
// this function is only available if your compiler supports thread-local variables;
// calling it will fail to link if your compiler doesn't
void set_unpremultiply_on_load_thread (int);
void convert_iphone_png_to_rgb_thread (int);
void set_flip_vertically_on_load_thread (int);
// ZLIB client - used by PNG, available for other purposes
char *zlib_decode_malloc_guesssize (const char *, int, int, int *);
char *zlib_decode_malloc_guesssize_headerflag (const char *, int, int, int *, int);
char *zlib_decode_malloc (const char *, int, int *);
int zlib_decode_buffer (char *, int, const char *, int);

char *zlib_decode_noheader_malloc (const char *, int, int *);
int zlib_decode_noheader_buffer (char *, int, const char *, int);
}

#endif // STBI_INCLUDE_STB_IMAGE_H
