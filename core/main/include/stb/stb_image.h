#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

typedef struct
{
   int      (*read)  (void *user,char *data,int size);   // fill 'data' with 'size' bytes.  return number of bytes actually read
   void     (*skip)  (void *user,int n);                 // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
   int      (*eof)   (void *user);                       // returns nonzero if we are at end of file/data
} stbi_io_callbacks;

extern unsigned char *stbi_load_from_memory   (unsigned char           const *buffer, int len   , int *x, int *y, int *channels_in_file, int desired_channels);
extern unsigned char *stbi_load_from_callbacks(stbi_io_callbacks const *clbk  , void *user, int *x, int *y, int *channels_in_file, int desired_channels);
extern unsigned char *stbi_load            (char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);

#ifndef STBI_NO_GIF
extern unsigned char *stbi_load_gif_from_memory(unsigned char const *buffer, int len, int **delays, int *x, int *y, int *z, int *comp, int req_comp);
#endif

#ifdef STBI_WINDOWS_UTF8
extern int stbi_convert_wchar_to_utf8(char *buffer, size_t bufferlen, const wchar_t* input);
#endif

////////////////////////////////////
//
// 16-bits-per-channel interface
//

extern unsigned short *stbi_load_16_from_memory   (unsigned char const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
extern unsigned short *stbi_load_16_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels);
extern unsigned short *stbi_load_16          (char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);

////////////////////////////////////
//
// float-per-channel interface
//
#ifndef STBI_NO_LINEAR
   extern float *stbi_loadf_from_memory     (unsigned char const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
   extern float *stbi_loadf_from_callbacks  (stbi_io_callbacks const *clbk, void *user, int *x, int *y,  int *channels_in_file, int desired_channels);
   extern float *stbi_loadf            (char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
#endif

#ifndef STBI_NO_HDR
   extern void   stbi_hdr_to_ldr_gamma(float gamma);
   extern void   stbi_hdr_to_ldr_scale(float scale);
#endif // STBI_NO_HDR

#ifndef STBI_NO_LINEAR
   extern void   stbi_ldr_to_hdr_gamma(float gamma);
   extern void   stbi_ldr_to_hdr_scale(float scale);
#endif // STBI_NO_LINEAR

// stbi_is_hdr is always defined, but always returns false if STBI_NO_HDR
extern int    stbi_is_hdr_from_callbacks(stbi_io_callbacks const *clbk, void *user);
extern int    stbi_is_hdr_from_memory(unsigned char const *buffer, int len);
extern int    stbi_is_hdr          (char const *filename);


// get a VERY brief reason for failure
// on most compilers (and ALL modern mainstream compilers) this is threadsafe
extern const char *stbi_failure_reason  (void);

// free the loaded image -- this is just free()
extern void     stbi_image_free      (void *retval_from_stbi_load);

// get image dimensions & components without fully decoding
extern int      stbi_info_from_memory(unsigned char const *buffer, int len, int *x, int *y, int *comp);
extern int      stbi_info_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp);
extern int      stbi_is_16_bit_from_memory(unsigned char const *buffer, int len);
extern int      stbi_is_16_bit_from_callbacks(stbi_io_callbacks const *clbk, void *user);
extern int      stbi_info               (char const *filename,     int *x, int *y, int *comp);
extern int      stbi_is_16_bit          (char const *filename);



// for image formats that explicitly notate that they have premultiplied alpha,
// we just return the colors as stored in the file. set this flag to force
// unpremultiplication. results are undefined if the unpremultiply overflow.
extern void stbi_set_unpremultiply_on_load(int flag_true_if_should_unpremultiply);

// indicate whether we should process iphone images back to canonical format,
// or just pass them through "as-is"
extern void stbi_convert_iphone_png_to_rgb(int flag_true_if_should_convert);

// flip the image vertically, so the first pixel in the output array is the bottom left
extern void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);

// as above, but only applies to images loaded on the thread that calls the function
// this function is only available if your compiler supports thread-local variables;
// calling it will fail to link if your compiler doesn't
extern void stbi_set_unpremultiply_on_load_thread(int flag_true_if_should_unpremultiply);
extern void stbi_convert_iphone_png_to_rgb_thread(int flag_true_if_should_convert);
extern void stbi_set_flip_vertically_on_load_thread(int flag_true_if_should_flip);

// ZLIB client - used by PNG, available for other purposes

extern char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen);
extern char *stbi_zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen, int parse_header);
extern char *stbi_zlib_decode_malloc(const char *buffer, int len, int *outlen);
extern int   stbi_zlib_decode_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

extern char *stbi_zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);
extern int   stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);
#endif // STBI_INCLUDE_STB_IMAGE_H
