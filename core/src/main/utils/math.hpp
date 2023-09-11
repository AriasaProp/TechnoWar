#ifndef Included_MATH_
#define Included_MATH_

//unsafe thread math

#include <cstdint>

// xaxis(0 left, 1 center, 2 right), yaxis (0 top, 1 center, 2 bottom)
enum Align : unsigned char {
  ALIGN_TOP_LEFT = 0,
  ALIGN_LEFT = 4,
  ALIGN_BOTTOM_LEFT = 8,
  ALIGN_TOP = 1,
  ALIGN_CENTER = 5,
  ALIGN_BOTTOM = 9,
  ALIGN_TOP_RIGHT = 2,
  ALIGN_RIGHT = 6,
  ALIGN_BOTTOM_RIGHT = 10
};

//resident memory usage
namespace memory_usage {
  unsigned long mem_usage ();
}

namespace clock_count {
  void start();
  void render();
  void end();
  
  size_t getFPS();
  float getDelta();
}

struct Rect {
public:
  float xmin, ymin, xmax, ymax;
  Rect();
  Rect(float, float, const Align&, float, float);
  bool insetOf(float, float);
};

namespace matrix4 {
  void idt (float *);
  void mul (float *, float *);
  void rotate (float *, float, float, float);
  void toOrtho (float *, float, float, float, float, float, float);
  void toOrtho2D (float *, float, float);
} // namespace matrix4

#endif //Included_MATH_