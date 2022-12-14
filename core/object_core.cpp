#include "object_core.h"

#include <cstring>

shader_core::shader_core(int i,const char *V, const char *F): id(i) {
	v = new char[strlen(V)];
	strcpy(v, V);
	f = new char[strlen(F)];
	strcpy(f, F);
}
shader_core::~shader_core() {
	delete[] v;
	delete[] f;
}
texture_core::texture_core(unsigned int i, unsigned int w, unsigned int h, unsigned char *d): id(i), width(w), height(h) {
	data = new unsigned char[w*h];
	memcpy(data, d, sizeof(data));
}
texture_core::~texture_core() {
	delete[] data;
}