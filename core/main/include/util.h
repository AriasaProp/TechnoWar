#ifndef UTIL_INCLUDED_
#define UTIL_INCLUDED_

#include <stdint.h>
#define MAX_TEMPORARY_BYTE 2048

union temp {
	char chars[MAX_TEMPORARY_BYTE];
	int ints[MAX_TEMPORARY_BYTE / sizeof(int)];
	unsigned int uints[MAX_TEMPORARY_BYTE / sizeof(unsigned int)];
	float floats[MAX_TEMPORARY_BYTE / sizeof(float)];
};

extern union temp stemp;

void *new_mem(size_t);
void *new_imem(size_t);
void free_mem(void *);
size_t get_mem_usage();

#endif // UTIL_INCLUDED_