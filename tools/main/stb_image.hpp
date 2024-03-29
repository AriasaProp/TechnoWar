#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

#include <cstdio>

#define STBI_VERSION 1

#include <cstdlib>

namespace stbi {

enum channel {
 none = 0, // only used for desired_channels

 grey = 1,
 grey_alpha = 2,
 rgb = 3,
 rgb_alpha = 4
};

//////////////////////////////////////////////////////////////////////////////
//
// PRIMARY API - works on images of any type
//

//
// load image by filename, open file, or memory buffer
//

struct io_callbacks {
 int (*read) (void *user,char *data,int size); // fill 'data' with 'size' bytes. return number of bytes actually read
 void (*skip) (void *user,int n); // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
 int (*eof) (void *user); // returns nonzero if we are at end of file/data
};

////////////////////////////////////
//
// 8-bits-per-channel interface
//

unsigned char *load_from_memory ( unsigned char const *buffer, int len , int *x, int *y, int *channels_in_file, int desired_channels);
unsigned char *load_from_callbacks(io_callbacks const *clbk , void *user, int *x, int *y, int *channels_in_file, int desired_channels);

unsigned char *load (char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
unsigned char *load_from_file (FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
// for stbi_load_from_file, file pointer is left pointing immediately after image

#ifndef STBI_NO_GIF
unsigned char *load_gif_from_memory( unsigned char const *buffer, int len, int **delays, int *x, int *y, int *z, int *comp, int req_comp);
#endif

#ifdef STBI_WINDOWS_UTF8
int convert_wchar_to_utf8(char *buffer, size_t bufferlen, const wchar_t* input);
#endif

////////////////////////////////////
//
// 16-bits-per-channel interface
//

unsigned short *load_16_from_memory ( unsigned char const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
unsigned short *load_16_from_callbacks(io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels);

unsigned short *load_16 (char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
unsigned short *load_from_file_16(FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);

////////////////////////////////////
//
// float-per-channel interface
//
#ifndef STBI_NO_LINEAR
 float *loadf_from_memory ( unsigned char const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
 float *loadf_from_callbacks (io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels);

 float *loadf (char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
 float *loadf_from_file (FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
#endif

#ifndef STBI_NO_HDR
 void hdr_to_ldr_gamma(float gamma);
 void hdr_to_ldr_scale(float scale);
#endif // STBI_NO_HDR

#ifndef STBI_NO_LINEAR
 void ldr_to_hdr_gamma(float gamma);
 void ldr_to_hdr_scale(float scale);
#endif // STBI_NO_LINEAR

// is_hdr is always defined, but always returns false if STBI_NO_HDR
int is_hdr_from_callbacks(io_callbacks const *clbk, void *user);
int is_hdr_from_memory( unsigned char const *buffer, int len);
int is_hdr (char const *filename);
int is_hdr_from_file(FILE *f);

// get a VERY brief reason for failure
// on most compilers (and ALL modern mainstream compilers) this is threadsafe
const char *failure_reason (void);

// free the loaded image -- this is just free()
void image_free (void *retval_from_stbi_load);

// get image dimensions & components without fully decoding
int info_from_memory( unsigned char const *buffer, int len, int *x, int *y, int *comp);
int info_from_callbacks(io_callbacks const *clbk, void *user, int *x, int *y, int *comp);
int is_16_bit_from_memory( unsigned char const *buffer, int len);
int is_16_bit_from_callbacks(io_callbacks const *clbk, void *user);

int info (char const *filename, int *x, int *y, int *comp);
int info_from_file (FILE *f, int *x, int *y, int *comp);
int is_16_bit (char const *filename);
int is_16_bit_from_file(FILE *f);

// for image formats that explicitly notate that they have premultiplied alpha,
// we just return the colors as stored in the file. set this flag to force
// unpremultiplication. results are undefined if the unpremultiply overflow.
void set_unpremultiply_on_load(int flag_true_if_should_unpremultiply);

// indicate whether we should process iphone images back to canonical format,
// or just pass them through "as-is"
void convert_iphone_png_to_rgb(int flag_true_if_should_convert);

// flip the image vertically, so the first pixel in the output array is the bottom left
void set_flip_vertically_on_load(int flag_true_if_should_flip);

// as above, but only applies to images loaded on the thread that calls the function
// this function is only available if your compiler supports thread-local variables;
// calling it will fail to link if your compiler doesn't
void set_unpremultiply_on_load_thread(int flag_true_if_should_unpremultiply);
void convert_iphone_png_to_rgb_thread(int flag_true_if_should_convert);
void set_flip_vertically_on_load_thread(int flag_true_if_should_flip);

// ZLIB client - used by PNG, available for other purposes

char *zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen);
char *zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen, int parse_header);
char *zlib_decode_malloc(const char *buffer, int len, int *outlen);
int zlib_decode_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

char *zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);
int zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

} // namespace stbi

#endif // STBI_INCLUDE_STB_IMAGE_H
