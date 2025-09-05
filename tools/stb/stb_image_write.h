#ifndef INCLUDE_STB_IMAGE_WRITE_H
#define INCLUDE_STB_IMAGE_WRITE_H

extern int stbi_write_tga_with_rle;
extern int stbi_write_png_compression_level;
extern int stbi_write_flip_vertically;

extern int stbi_write_png(char const *filename, int w, int h, int comp, const void *data);
extern int stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data);
extern int stbi_write_tga(char const *filename, int w, int h, int comp, const void *data);
extern int stbi_write_hdr(char const *filename, int w, int h, int comp, const float *data);
extern int stbi_write_jpg(char const *filename, int x, int y, int comp, const void *data, int quality);

extern unsigned char *stbi_write_png_to_mem(const unsigned char *pixels, int w, int h, int comp, int *out_len);

typedef void stbi_write_func(void *context, void *data, int size);

extern int stbi_write_png_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void *data);
extern int stbi_write_bmp_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void *data);
extern int stbi_write_tga_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void *data);
extern int stbi_write_hdr_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const float *data);
extern int stbi_write_jpg_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const void *data, int quality);

extern void stbi_flip_vertically_on_write(int flip_boolean);

#endif // INCLUDE_STB_IMAGE_WRITE_H
