#ifndef _MATRIX_INCLUDED_
#define _MATRIX_INCLUDED_

#include "common.h"
#include "math/vec.h"

#ifndef _MATRIX_IMPLEMENTATION_
  extern void matrix4_idt(float *);
  extern float matrix4_mul(float*,const float*);
  extern void matrix4_rotateDeg(float *, const vec3);
#endif // _MATRIX_IMPLEMENTATION_

#endif // _MATRIX_INCLUDED_