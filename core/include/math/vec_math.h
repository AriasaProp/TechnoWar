#ifndef VEC_MATH
#define VEC_MATH

#include "engine.h"

// vec2 math

extern vec2 vec2_add(vec2, vec2);
extern vec2 vec2_sub(vec2, vec2);
extern vec2 vec2_addf(vec2, float);

extern void vec2_trn(vec2*, vec2);
extern void vec2_trnf(vec2*, float);

extern vec2 vec2_mul(vec2, vec2);
extern vec2 vec2_mulf(vec2, float);
extern vec2 vec2_div(vec2, vec2);

extern void vec2_scl(vec2*, vec2);
extern void vec2_sclf(vec2*, float);

extern float vec2_dot(vec2, vec2);
extern float vec2_crs(vec2, vec2);
extern float vec2_dist(vec2, vec2);
extern float vec2_len(vec2);
extern vec2 vec2_norm(vec2, float);
extern float vec2_rad(vec2, vec2);


#endif // VEC_MATH