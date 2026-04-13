#ifndef _VEC_INCLUDED_
#define _VEC_INCLUDED_

#include "common.h"

typedef struct {
  float x, y;
} vec2;
typedef struct {
  float x, y, z;
} vec3;
typedef struct {
  float x, y, z, w;
} vec4;
typedef struct {
  int x, y;
} ivec2;
typedef struct {
  uint16_t x, y;
} uivec2;

#define IVEC2_ZERO    (ivec2){0,0}
#define IVEC2_SQR(X)  (ivec2){X,X}
#define IVEC2_NEW(X,Y) (ivec2){X,Y}
#define IVEC2_IS0(X)  (X->x && X->y)

#define  VEC2_ZERO    (vec2){0.0f,0.0f}
#define  VEC2_SQR(X)  (vec2){X,X}
#define  VEC2_NEW(X,Y) (vec2){X,Y}
#define  VEC2_IS0(X)  ((X->x!=0.0f) && (X->y!=0.0f))



#ifndef _VEC_IMPLEMENTATION_
  // vec2 math
  extern vec2  vec2_fromRad(const float);
  
  extern float vec2_dot(const vec2, const vec2);
  extern float vec2_crs(const vec2, const vec2);
  extern float vec2_dis(const vec2, const vec2);
  extern float vec2_len2(const vec2);
  extern float vec2_len(const vec2);
  extern float vec2_rad(const vec2, const vec2);

  extern vec2 vec2_floor(const vec2);
  extern vec2 vec2_addf(const vec2, const float);
  extern vec2 vec2_subf(const vec2, const float);
  extern vec2 vec2_mulf(const vec2, const float);
  extern vec2 vec2_divf(const vec2, const float);
  extern vec2 vec2_add (const vec2, const vec2);
  extern vec2 vec2_sub (const vec2, const vec2);
  extern vec2 vec2_mul (const vec2, const vec2);
  extern vec2 vec2_div (const vec2, const vec2);
  extern vec2 vec2_inv (const vec2);
  extern vec2 vec2_norm(const vec2);
  
  extern void vec2_floors(vec2*);
  extern void vec2_addsf(vec2 *, const float);
  extern void vec2_subsf(vec2 *, const float);
  extern void vec2_mulsf(vec2 *, const float);
  extern void vec2_divsf(vec2 *, const float);
  extern void vec2_adds (vec2 *, const vec2);
  extern void vec2_subs (vec2 *, const vec2);
  extern void vec2_muls (vec2 *, const vec2);
  extern void vec2_divs (vec2 *, const vec2);
  extern void vec2_invs (vec2 *);
  extern void vec2_norms(vec2 *);
  
  // ivec2 math
  extern ivec2 ivec2_add(const ivec2, const ivec2);
  extern ivec2 ivec2_sub(const ivec2, const ivec2);
  
  extern void ivec2_subs(ivec2 *,const ivec2);
  
  // uivec2 math
  extern uivec2 uivec2_sub(const uivec2, const uivec2);
  extern uivec2 uivec2_add(const uivec2, const uivec2);
  
  extern void uivec2_addsi(uivec2 *, const uint16_t);
  extern void uivec2_subsi(uivec2 *, const uint16_t);
  
#endif // _VEC_IMPLEMENTATION_

#endif // _VEC_INCLUDED_