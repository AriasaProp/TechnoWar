/*
 *  Util Header
 *
 *  Provide basic function and constant for all source code
 *
 *
 *
 */

#ifndef COMMON_INCLUDED_
#define COMMON_INCLUDED_

#include <stddef.h>
#include <stdlib.h>
#include <math.h>

// ================================
//  Global Macro
// ================================

#if defined(_MSC_VER) && (_MSC_VER < 1600)
  typedef __int8 int8_t;
  typedef __int16 int16_t;
  typedef __int32 int32_t;
  typedef __int64 int64_t;
  typedef unsigned __int8 uint8_t;
  typedef unsigned __int16 uint16_t;
  typedef unsigned __int32 uint32_t;
  typedef unsigned __int64 uint64_t;
  typedef unsigned __int64 size_t;
#else
  #include <stdint.h>
#endif

#if defined(_MSC_VER)
  #define ROTL(X,Y)    _rotl(X,Y)
  #define INLINE        __forceinline
  #define CDECL         __cdecl
  #define UNUSED(x)     ((void)x)
  #define UNUSED_ARG(x) __pragma(warning(suppress : 4100 4101)) x
#elif defined(__GNUC__) || defined(__clang__)
  #if __has_builtin(__builtin_rotateleft32)
    #define ROTL(X,Y)    __builtin_rotateleft32(X, Y)
  #else
    #define ROTL(X,Y)    ((X << Y) | (X >> (~Y & -31)))
  #endif
  
  #define INLINE        inline
  #define CDECL         __attribute__((cdecl))
  #define UNUSED(x)     ((void)x)
  #define UNUSED_ARG(x) __attribute__((unused)) x
#else
  #error "unknown Compiler action"
#endif

#include <assert.h>
#define ASSERT(X) assert(X)

#define MAX(X, Y)      ((X > Y) ? (X) : (Y))
#define MIN(X, Y)      ((X < Y) ? (X) : (Y))
#define CLAMP(X, Y, Z) ((X < Z) ? ((Y > Z) ? (Z) : (Y)) : (X))

// ================================
//  Structure
// ================================

typedef struct {
  float r, g, b, a;
} fcolor; // hex color
typedef union {
  struct {
    uint8_t r, g, b, a;
  } u8;
  struct {
    uint16_t l, a;
  } u16;
  uint32_t u32;
} icolor; // packed color bit
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

#define FCOLOR_ZERO   (fcolor){.r = 0.f, .g = 0.f, .b = 0.f, .a = 0.f}
#define FCOLOR_RED    (fcolor){.r = 1.f, .g = 0.f, .b = 0.f, .a = 1.f}
#define FCOLOR_GREEN  (fcolor){.g = 1.f, .r = 0.f, .b = 0.f, .a = 1.f}
#define FCOLOR_BLUE   (fcolor){.b = 1.f, .r = 0.f, .g = 0.f, .a = 1.f}

#define ICOLOR_ZERO   (icolor){.u32 = 0}
#define ICOLOR_RED    (icolor){.u8.r = 0xff, .u8.g = 0, u8.b = 0, u8.a = 0xff}
#define ICOLOR_GREEN  (icolor){.u8.g = 0xff, .u8.r = 0, u8.b = 0, u8.a = 0xff}
#define ICOLOR_BLUE   (icolor){.u8.b = 0xff, .u8.r = 0, u8.g = 0, u8.a = 0xff}

#define IVEC2_ZERO    (ivec2){0,0}
#define IVEC2_SQR(X)  (ivec2){X,X}

#define  VEC2_ZERO    (vec2){0.0f,0.0f}
#define  VEC2_SQR(X)  (vec2){X,X}

// ===============================
//  Function
// ===============================

// helper
#ifndef COMMON_IMPLEMENTATION_
  #ifdef _WIN32
    extern __declspec(dllimport) int __stdcall MultiByteToWideChar(unsigned int, unsigned long, const char *, int, wchar_t *, int);
    extern __declspec(dllimport) int __stdcall WideCharToMultiByte(unsigned int, unsigned long, const wchar_t *, int, char *, int, const char *, int *);
    extern int convert_wchar_to_utf8(char *, size_t, const wchar_t *);
  #endif
  extern void flipBytes(uint8_t *, const size_t);
  
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
  extern vec2 vec2_fromRad(float);
  
  extern void matrix4_idt(float *);
  extern void matrix4_rotateDeg(float *, vec3);
#endif // COMMON_IMPLEMENTATION_

#endif // COMMON_INCLUDED_