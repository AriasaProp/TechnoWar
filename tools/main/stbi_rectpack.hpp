#ifndef STBI_RECTPACK_HPP
#define STBI_RECTPACK_HPP

namespace stbi {
namespace rectpack {

  struct rect {
    // input:
    unsigned int w, h;
    int id;
    // output:
    unsigned int x, y;
    int was_packed; // non-zero if valid packing
  };                // 16 bytes, nominally

  bool pack_rects (unsigned int, unsigned int, rect *, unsigned int);
  // Assign packed locations to rectangles. The rectangles are of type
  // 'rect' defined below, stored in the array 'rects', and there
  // are 'num_rects' many of them.
  //
  // Rectangles which are successfully packed have the 'was_packed' flag
  // set to a non-zero value and 'x' and 'y' store the minimum location
  // on each axis (i.e. bottom-left in cartesian coordinates, top-left
  // if you imagine y increasing downwards). Rectangles which do not fit
  // have the 'was_packed' flag set to 0.
  //
  // You should not try to access the 'rects' array from another thread
  // while this function is running, as the function temporarily reorders
  // the array while it executes.
  //
  // To pack into another rectangle, you need to call init_target
  // again. To continue packing into the same rectangle, you can call
  // this function again. Calling this multiple times with multiple rect
  // arrays will probably produce worse packing results than calling it
  // a single time with the full rectangle array, but the option is
  // available.
  //
  // The function return true when all rect packed

} // namespace rectpack
} // namespace stbi
#endif // STBI_RECTPACK_HPP
