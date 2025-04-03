/*
 *  Util Header
 *
 *  Provide basic function and constant for all source code
 *
 *
 *
 */

#ifndef UTIL_INCLUDED_
#define UTIL_INCLUDED_

#include <stddef.h>
#include <stdlib.h>

// ================================
//  Global Macro
// ================================

#if (defined(_MSC_VER) && _MSC_VER < 1600) /*|| defined(__SYMBIAN32__) */
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

#ifdef _WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
#endif

#if defined(_MSC_VER)
#define INLINE        __forceinline
#define CDECL         __cdecl
#define UNUSED(x)     ((void)x)
#define UNUSED_ARG(x) __pragma(warning(suppress : 4100 4101)) x
#elif defined(__GNUC__) || defined(__clang__)
#define INLINE        inline
#define CDECL         /* no translate */
#define UNUSED(x)     ((void)x)
#define UNUSED_ARG(x) __attribute__((unused)) x
#else
#define INLINE        inline
#define CDECL         /* no translate */
#define UNUSED(x)     /* no parameter */
#define UNUSED_ARG(x) /* no parameter */
#endif

#include <assert.h>
#define ASSERT(X) assert(X)

#define MAX(X, Y)      ((X > Y) ? (X) : (Y))
#define MIN(X, Y)      ((X < Y) ? (X) : (Y))
#define CLAMP(X, Y, Z) ((X < Z) ? ((Y > Z) ? (Z) : (Y)) : (X))

// ================================
//  Constant dan Variabel
// ================================

struct fcolor
{
  float r, g, b, a;
};
struct icolor
{
  uint8_t r, g, b, a;
}; // packed color bit
struct vec2
{
  float x, y;
};
struct vec3
{
  float x, y, z;
};
struct uivec2
{
  uint16_t x, y;
};
struct vec4
{
  float x, y, z, w;
};

// ===============================
//  Function
// ===============================

// helper

#ifdef _WIN32
extern int convert_wchar_to_utf8(char *, size_t, const wchar_t *);
#endif

// math
extern int lrotl(int, size_t);
extern int lrotr(int, size_t);
extern void matrix4_idt(float *);
extern void matrix4_rotateDeg(float *, struct vec3);

#endif // UTIL_INCLUDED_