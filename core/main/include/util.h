#ifndef UTIL_INCLUDED_
#define UTIL_INCLUDED_

#include <stdint.h>

extern void *new_mem(size_t);
extern void *new_imem(size_t);
extern void free_mem(void *);
extern size_t get_mem_usage();

#endif // UTIL_INCLUDED_