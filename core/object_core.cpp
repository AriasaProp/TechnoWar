#include "object_core.h"

#include <cstring>

shader_core::shader_core(int i,const char *, const char*): id(i) {
	v = new char[strlen(V)];
	strcpy(v, V);
	f = new char[strlen(F)];
	strcpy(f, F);
}
shader_core::~shader_core() {
	delete[] v;
	delete[] f;
}
texture_core::texture_core(unsigned int i, unsigned int w, unsigned int h, const unsigned char *d): id(i), width(w), height(h) {
	data = new unsigned char[strlen(d)];
	strcpy(data, d);
}
texture_core::~texture_core() {
	delete[] data;
}
//mesh core
struct mesh_core {
	unsigned int vaoId;
	unsigned int vboV, vboI;
	unsigned int vertex_len, index_len;// based type len, not in byte
	struct data {
		float posx, posy, posz;
		unsigned char r, g, b, a;
	} *vertex;
	unsigned short *index;
};

#endif //Included_OBJ_RES