#ifndef STBI_RECTPACK_HPP
#define STBI_RECTPACK_HPP

#define STBRP__MAXVAL 0x7fffffff
// Mostly for internal use, but this is the maximum supported coordinate value.
namespace stbi {
namespace rectpack {
  struct rect {
    // reserved for your use:
    int id;

    // input:
    int w, h;

    // output:
    int x, y;
    int was_packed; // non-zero if valid packing

  }; // 16 bytes, nominally
  struct node {
    int x, y;
    node *next;
  };

  struct context {
    int width;
    int height;
    int align;
    int init_mode;
    int heuristic;
    int num_nodes;
    node *active_head;
    node *free_head;
    node extra[2]; // we allocate two extra nodes so optimal user-node-count is 'width' not 'width+2'
  };

  bool pack_rects (context *, rect *rects, int num_rects);
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
  // The function returns 1 if all of the rectangles were successfully
  // packed and 0 otherwise.

  void init_target (context *, int width, int height, node *nodes, int num_nodes);
  // Initialize a rectangle packer to:
  //    pack a rectangle that is 'width' by 'height' in dimensions
  //    using temporary storage provided by the array 'nodes', which is 'num_nodes' long
  //
  // You must call this function every time you start packing into a new target.
  //
  // There is no "shutdown" function. The 'nodes' memory must stay valid for
  // the following pack_rects() call (or calls), but can be freed after
  // the call (or calls) finish.
  //
  // Note: to guarantee best results, either:
  //       1. make sure 'num_nodes' >= 'width'
  //   or  2. call stbrp_allow_out_of_mem() defined below with 'allow_out_of_mem = 1'
  //
  // If you don't do either of the above things, widths will be quantized to multiples
  // of small integers to guarantee the algorithm doesn't run out of temporary storage.
  //
  // If you do #2, then the non-quantized algorithm will be used, but the algorithm
  // may run out of temporary storage and be unable to pack some rectangles.

  void setup_allow_out_of_mem (context *, int allow_out_of_mem);
  // Optionally call this function after init but before doing any packing to
  // change the handling of the out-of-temp-memory scenario, described above.
  // If you call init again, this will be reset to the default (false).

  void setup_heuristic (context *, int heuristic);
  // Optionally select which packing heuristic the library should use. Different
  // heuristics will produce better/worse results for different data sets.
  // If you call init again, this will be reset to the default.

  enum HEURISTIC_Skyline {
    none = 0,
    BL_sortHeight = none,
    BF_sortHeight
  };

} // namespace rectpack
} // namespace stbi
#endif // STBI_RECTACK_HPP
