/*
 *  Util Header
 *
 *  Provide basic function and constan for all source code
 *
 *
 *
 */

#ifndef UTIL_INCLUDED_
#define UTIL_INCLUDED_

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// ================================
//  Constant dan Variabel
// ================================

#ifdef __GNUC__
#define UNUSED(x) x __attribute__ ((unused))
#else
#define UNUSED(x) /* x */
#endif
#define MAX_TEMPORARY_BYTE 2048

struct fcolor {
  float r, g, b, a;
};
struct icolor {
  uint8_t r, g, b, a;
}; // packed color bit
struct vec2 {
  float x, y;
};
struct vec3 {
  float x, y, z;
};
struct uivec2 {
  uint16_t x, y;
};
struct vec4 {
  float x, y, z, w;
};

union temp {
  char chars[MAX_TEMPORARY_BYTE];
  int ints[MAX_TEMPORARY_BYTE / sizeof (int)];
  unsigned int uints[MAX_TEMPORARY_BYTE / sizeof (unsigned int)];
  float floats[MAX_TEMPORARY_BYTE / sizeof (float)];
};

extern union temp stemp;

// ===============================
//  Function
// ===============================

void *new_mem (size_t);
void *new_imem (size_t);
void free_mem (void *);
size_t get_mem_usage ();

// math
void matrix4_idt (float *);

#endif // UTIL_INCLUDED_