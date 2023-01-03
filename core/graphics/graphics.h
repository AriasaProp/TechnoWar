#ifndef Included_Graphics
#define Included_Graphics 1

//maximum 2dbatch render 1000
#define MAX_TEXTURE_UI 100
//maximum output log message in char
#define MAX_GL_MSG 1024

//texture core
struct texture_core {
	unsigned int id;
	unsigned int width, height;
	unsigned char *data;
};
//ui_core
struct flat_vertex {
	float x, y;
	unsigned char color[4];
};
//mesh core
struct mesh_core {
	bool dirty_vertex, dirty_index;
	unsigned int vao;
	unsigned int vbo, ibo;
	unsigned int vertex_len, index_len;
	struct data {
		float x, y, z;
		unsigned char r, g, b, a;
	} *vertex = nullptr;
	unsigned short *index = nullptr;
	float trans[16] = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
};

struct graphics {
	virtual const float getWidth() = 0;
	virtual const float getHeight() = 0;
	//tag with level, 1 is color, 2 is buffer, 4 is stencil
	virtual void clear(const unsigned int&) = 0;
	virtual void clearcolor(const float&, const float&, const float&, const float&) = 0;

	virtual texture_core *gen_texture(const int&, const int&, unsigned char*) = 0;
	virtual void bind_texture(texture_core*) = 0;
	virtual void set_texture_param(const int&, const int&) = 0;
	virtual void delete_texture(texture_core*) = 0;
	
	virtual void flat_render(flat_vertex *, unsigned int) = 0;
	
	virtual mesh_core *gen_mesh(mesh_core::data*,unsigned int, unsigned short*,unsigned int) = 0;
	virtual void mesh_render(mesh_core**,const unsigned int&) = 0;
	virtual void delete_mesh(mesh_core*) = 0;
};

#endif //Included_Graphics

