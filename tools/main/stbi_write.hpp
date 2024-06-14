#ifndef STBI_WRITE
#define STBI_WRITE

#include <cstdlib>

namespace stbi {
namespace write {

  extern int tga_with_rle;
  extern int png_compression_level;
  extern int force_png_filter;

#ifndef STBI_WRITE_NO_STDIO
  int png (char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
  int bmp (char const *filename, int w, int h, int comp, const void *data);
  int tga (char const *filename, int w, int h, int comp, const void *data);
  int hdr (char const *filename, int w, int h, int comp, const float *data);
  int jpg (char const *filename, int x, int y, int comp, const void *data, int quality);

#ifdef STBIW_WINDOWS_UTF8
  int convert_wchar_to_utf8 (char *buffer, size_t bufferlen, const wchar_t *input);
#endif
#endif

  typedef void write_func (void *context, void *data, int size);

  int png_to_func (write_func *func, void *context, int w, int h, int comp, const void *data, int stride_in_bytes);
  int bmp_to_func (write_func *func, void *context, int w, int h, int comp, const void *data);
  int tga_to_func (write_func *func, void *context, int w, int h, int comp, const void *data);
  int hdr_to_func (write_func *func, void *context, int w, int h, int comp, const float *data);
  int jpg_to_func (write_func *func, void *context, int x, int y, int comp, const void *data, int quality);

  void flip_vertically_on_write (int flip_boolean);
} // namespace write
} // namespace stbi
#endif // STBI_WRITE