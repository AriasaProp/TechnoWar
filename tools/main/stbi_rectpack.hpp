#ifndef STBI_RECTPACK_HPP
#define STBI_RECTPACK_HPP

namespace stbi {
namespace rectpack {

enum heuristic {
   skylineBL_sortHeight,
   skylineBF_sortHeight
};

struct rect {
	// input:
	unsigned int w, h;
	// output:
	unsigned int x = 0, y = 0;
	int was_packed = 0;  // non-zero if valid packing
	rect(unsigned int,unsigned int);
}; // 16 bytes, nominally
struct node {
   unsigned int x,y;
   node *next;
};

struct context {
	context(unsigned int,unsigned int);
// Initialize a rectangle packer to:
//    pack a rectangle that is 'width' by 'height' in dimensions
//
// You must call this function every time you start packing into a new target.
//
// There is no "shutdown" function. The 'nodes' memory must stay valid for
// the following pack_rects() call (or calls), but can be freed after
// the call (or calls) finish.
// If you don't do either of the above things, widths will be quantized to multiples
// of small integers to guarantee the algorithm doesn't run out of temporary storage.
//
// If you do #2, then the non-quantized algorithm will be used, but the algorithm
// may run out of temporary storage and be unable to pack some rectangles.

	~context();
	unsigned int width;
	unsigned int height;
	heuristic hr = heuristic::skylineBL_sortHeight;
	node *active_head;
	node *free_head;
	node *nodes;
	node extra[2]; // we allocate two extra nodes so optimal user-node-count is 'width' not 'width+2'
};


bool pack_rects (context *, rect *, int);
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
