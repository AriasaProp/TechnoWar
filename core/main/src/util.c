#include "util.h"
#include <string.h>
#include <stdlib.h>

union temp stemp;

#ifndef NDEBUG
#define MAX_WATCHED_MEMORY 2048
static size_t total_mem = 0;
static struct watched_memory {void*p; size_t l;} watched_memory_list[MAX_WATCHED_MEMORY] = { 0 };
#endif

void *new_mem(size_t l) {
#ifndef NDEBUG
	for (size_t i = 0; i < MAX_WATCHED_MEMORY; ++i) {
		if (watched_memory_list[i].p == 0) {
			watched_memory_list[i].p = malloc(l);
			watched_memory_list[i].l = l;
			total_mem += l;
			return watched_memory_list[i].p;
		}
	}
#endif
	return malloc(l);
}
void *new_imem(size_t l) {
#ifndef NDEBUG
	for (size_t i = 0; i < MAX_WATCHED_MEMORY; ++i) {
		if (watched_memory_list[i].p == 0) {
			watched_memory_list[i].p = calloc(1, l);
			watched_memory_list[i].l = l;
			total_mem += l;
			return watched_memory_list[i].p;
		}
	}
#endif
	return calloc(1, l);
}
void free_mem(void *a) {
#ifndef NDEBUG
	for (size_t i = 0, j; i < MAX_WATCHED_MEMORY; ++i) {
		if (watched_memory_list[i].p == 0) break;
		else if (watched_memory_list[i].p == a) {
			total_mem -= watched_memory_list[i].l;
			free(watched_memory_list[i].p);
			j = i + 1;
			if (j < MAX_WATCHED_MEMORY) {
				memmove(watched_memory_list + i, watched_memory_list + j, sizeof(struct watched_memory) * (MAX_WATCHED_MEMORY - j));
			}
			watched_memory_list[MAX_WATCHED_MEMORY - 1] = { 0, 0 };
		}
	}
#endif
	free (a);
}
size_t get_mem_usage() {
#ifndef NDEBUG
	return total_mem;
#else
	return 0;
#endif
}