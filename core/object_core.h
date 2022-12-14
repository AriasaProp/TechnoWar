#ifndef Included_OBJ_Core
#define Included_OBJ_Core 1

//shader core
struct shader_core {
	int id;
	char *v;
	char *f;
	shader_core(int,const char*, const char*);
	~shader_core();
};
//texture core
struct texture_core {
	unsigned int id;
	unsigned int width, height;
	unsigned char *data;
	texture_core(unsigned int, unsigned int, unsigned int, unsigned char*);
	~texture_core();
};
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

#endif //Included_OBJ_Core