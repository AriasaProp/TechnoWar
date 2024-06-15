#ifndef STBI_WRITE
#define STBI_WRITE

#include <cstdlib>

namespace stbi {
namespace write {

  extern int tga_with_rle;
  extern int png_compression_level;
  extern int force_png_filter;

#ifndef STBI_WRITE_NO_STDIO
	//filename, width, height, channel type, data
  int tga (char const *, int, int, int, const void*);
	//filename, width, height, channel type, data
  int bmp (char const *, int, int, int, const void*);
	//filename, width, height, channel type, data, stride-in-bytes (channel size)
  int png (char const *, int, int, int, const void*, int);
	//filename, width, height, channel type, data, quality
  int jpg (char const *, int, int, int, const void*, int);
	//filename, width, height, channel type, data
  int hdr (char const *, int, int, int, const float*);

#ifdef STBIW_WINDOWS_UTF8
  int convert_wchar_to_utf8 (char *buffer, size_t bufferlen, const wchar_t *input);
#endif
#endif

  typedef void write_func (void*, void *data, int size);
	//func, context, width, height, channel type, data, stride_in_bytes
  int png_to_func (write_func*, void*, int, int, int, const void *, int);
	//func, context, width, height, channel type, data
  int bmp_to_func (write_func*, void*, int, int, int, const void *);
	//func, context, width, height, channel type, data
  int tga_to_func (write_func*, void*, int, int, int, const void *);
	//func, context, width, height, channel type, data
  int hdr_to_func (write_func*, void*, int, int, int, const float *);
	//func, context, width, height, channel type, data, quality
  int jpg_to_func (write_func*, void*, int, int, int, const void *, int);

  void flip_vertically_on_write (bool flip_boolean);
} // namespace write
} // namespace stbi
#endif // STBI_WRITE