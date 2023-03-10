#ifndef _Included_Engine
#define _Included_Engine
	
//maximum 2dbatch render 1000
#define MAX_TEXTURE_UI 100
//maximum output log message in char
#define MAX_GL_MSG 1024

namespace engine {
	//graphics
	//texture core
	struct texture_core {
		unsigned int id;
		unsigned int width, height;
		unsigned char *data;
	};
	//ui_core
	struct flat_vertex {
		float x, y;
		unsigned char r, g, b, a;
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
	
	// grafik
	extern float (*getWidth)();
	extern float (*getHeight)();
	extern void (*clear)(const unsigned int&);
	extern void (*clearcolor)(const float&, const float&, const float&, const float&);
	extern texture_core *(*gen_texture)(const int&, const int&, unsigned char*);
	extern void (*bind_texture)(texture_core*);
	extern void (*set_texture_param)(const int&, const int&);
	extern void (*delete_texture)(texture_core*);
	extern void (*flat_render)(flat_vertex*, unsigned int);
	extern mesh_core *(*gen_mesh)(mesh_core::data*, unsigned int, unsigned short*, unsigned int);
	extern void (*mesh_render)(mesh_core**, const unsigned int&);
	extern void (*delete_mesh)(mesh_core*);
	// input
	extern float *(*getSensorValue)(const char*);
	extern int (*getX)(unsigned int);
	extern int (*getDeltaX)(unsigned int);
	extern int (*getY)(unsigned int);
	extern int (*getDeltaY)(unsigned int);
	extern bool (*justTouched)();
	extern bool (*isTouched)(unsigned int);
	extern float (*getPressure)(unsigned int);
	extern bool (*isButtonPressed)(int button);
	extern bool (*isButtonJustPressed)(int button);
	extern bool (*isKeyPressed)(int key);
	extern bool (*isKeyJustPressed)(int key);
	extern void (*process_event)();

}

#endif //_Included_Engine