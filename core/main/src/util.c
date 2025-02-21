#include "util.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

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
			watched_memory_list[MAX_WATCHED_MEMORY - 1].p = 0;
			watched_memory_list[MAX_WATCHED_MEMORY - 1].l = 0;
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

// math
void matrix4_idt(float *m) {
	memset(m, 0, 16 * sizeof(float));
	m[0] = m[5] = m[10] = m[15] = 1.0f;
}
void matrix4_mul(float *a, float *b) {
	memset(stemp.mat, 0, 16 * sizeof(float));
	for (size_t h = 0; h < 4; ++h) {
		for (size_t v = 0; v < 4; ++v) {
			for (size_t i = 0; i < 4; ++i) {
				stemp.mat[i] += a[(h * 4) + i] * b[v + (i * 4)];
			}
		}
	}
}
void matrix4_rotateDeg(float *m, struct vec3 deg) {
	// yaw
	float yawSin = sin(deg.x / 180.0f * M_PI), yawCos = cos(deg.x / 180.0f * M_PI);
	// pitch
	float pitchSin = sin(deg.y / 180.0f * M_PI), pitchCos = cos(deg.y / 180.0f * M_PI);
	// roll
	float rollSin = sin(deg.z / 180.0f * M_PI), rollCos = cos(deg.z / 180.0f * M_PI);
	
	for (size_t i = 0; i < 16; i += 4;) {
		t[i] = m[i] * pitchCos * rollCos + m[i + 1] * pitchCos * rollSin - m[i + 2] * pitchSin;
		t[i + 1] = m[i] * (yawSin * pitchSin * rollCos - yawCos * rollSin) + 
					 m[i + 1] * (yawSin * pitchSin * rollSin + yawCos * rollCos) +
					 m[i + 2] * (pitchCos * rollSin);
		t[i + 2] = m[i] * (yawCos * pitchSin * rollCos + yawSin * rollSin) + 
					 m[i + 1] * (yawSin * pitchSin * rollCos - yawCos * rollSin) +
					 m[i + 2] * (pitchCos * rollCos);
		t[i + 3] = m[i + 3];
	}
}