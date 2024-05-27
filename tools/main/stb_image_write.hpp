#ifndef INCLUDE_STB_IMAGE_WRITE_H
#define INCLUDE_STB_IMAGE_WRITE_H

namespace stbi {
namespace write {

  int png (char const *, int, int, int, const void *, int);
  int bmp (char const *, int, int, int, const void *);
  int tga (char const *, int, int, int, const void *);
  int hdr (char const *, int, int, int, const float *);
  int jpg (char const *, int x, int y, int, const void *, int);

#ifdef STBIW_WINDOWS_UTF8
  int convert_wchar_to_utf8 (char *, size_t, const wchar_t *);
#endif

  typedef void img_func (void *, void *, int);

  int png_to_func (img_func *, void *, int, int, int, const void *, int);
  int bmp_to_func (img_func *, void *, int, int, int, const void *);
  int tga_to_func (img_func *, void *, int, int, int, const void *);
  int hdr_to_func (img_func *, void *, int, int, int, const float *);
  int jpg_to_func (img_func *, void *, int x, int y, int, const void *, int);

  void flip_vertically_on_write (bool);
  void tga_with_rle (bool);
  void png_compression_level (unsigned int);
  void force_png_filter (int);

} // namespace write
} // namespace stbi
#endif // INCLUDE_STB_IMAGE_WRITE_H
