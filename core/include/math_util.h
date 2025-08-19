#ifndef MATH_UTIL_
#define MATH_UTIL_

#include "engine.h"

#define VEC2_ZERO \
  (vec2) { 0.f, 0.f }
#define VEC2_ONE \
  (vec2) { 1.f, 1.f }

#ifndef MATH_UTIL_IMPLEMENTATION_
// vec2 math
extern vec2 vec2_add(vec2, vec2);
extern vec2 vec2_sub(vec2, vec2);
extern vec2 vec2_addf(vec2, float);

extern void vec2_trn(vec2 *, vec2);
extern void vec2_trnf(vec2 *, float);

extern vec2 vec2_mul(vec2, vec2);
extern vec2 vec2_mulf(vec2, float);
extern vec2 vec2_div(vec2, vec2);
extern vec2 vec2_inv(vec2);

extern void vec2_scl(vec2 *, vec2);
extern void vec2_sclf(vec2 *, float);

extern float vec2_dot(vec2, vec2);
extern float vec2_crs(vec2, vec2);
extern float vec2_dist(vec2, vec2);
extern float vec2_len(vec2);
extern vec2 vec2_norm(vec2);
extern float vec2_rad(vec2, vec2);

// radian
extern vec2 vec2_fromRad(float);
#endif // MATH_UTIL_IMPLEMENTATION_

#endif // MATH_UTIL_