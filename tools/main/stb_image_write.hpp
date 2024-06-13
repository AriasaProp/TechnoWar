#ifndef INCLUDE_STB_IMAGE_WRITE_H
#define INCLUDE_STB_IMAGE_WRITE_H

namespace stbi {
namespace write {

  bool png (char const *, int, int, int, const void *, int);
  bool bmp (char const *, int, int, int, const void *);
  bool tga (char const *, int, int, int, const void *);
  bool hdr (char const *, int, int, int, const float *);
  bool jpg (char const *, int, int, int, const void *, int);

#ifdef STBIW_WINDOWS_UTF8
  int convert_wchar_to_utf8 (char *, size_t, const wchar_t *);
#endif

  typedef void img_func (void *, void *, int);

  bool png_to_func (img_func *, void *, int, int, int, const void *, int);
  bool bmp_to_func (img_func *, void *, int, int, int, const void *);
  bool tga_to_func (img_func *, void *, int, int, int, const void *);
  bool hdr_to_func (img_func *, void *, int, int, int, const float *);
  bool jpg_to_func (img_func *, void *, int x, int y, int, const void *, int);

} // namespace write
} // namespace stbi

#endif // INCLUDE_STB_IMAGE_WRITE_H
