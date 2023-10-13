#ifndef Included_MATH_
#define Included_MATH_

// unsafe thread math

#include <cstddef>
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

namespace memory_usage {
const char *mem_usage ();
const char *cpu_time ();
} // resources information
namespace clock_count {
void start ();
void render ();
void end ();

size_t getFPS ();
float getDelta ();
} // namespace clock_count
namespace matrix4 {
void idt (float *);
void mul (float *, float *);
void rotate (float *, float, float, float);
void toOrtho (float *, float, float, float, float, float, float);
void toOrtho2D (float *, float, float);
} // namespace matrix4

struct Vector2 {
private:
  float x, y;
  Align pivot_align;
public:
  Vector2();
  Vector2(float, float, const Align &);
  void getFloats(float *);
};

struct Rect {
private:
  float x, y, sx, sy;
  Align main_align, pivot_align;
public:
  Rect ();
  Rect (float, float, float, float, const Align &, const Align &);
  void getFloats(float *);
  bool insetOf (float, float);
};


#endif // Included_MATH_