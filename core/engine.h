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

	extern {
	  float (*getWidth)();
	  float (*getHeight)();
	  void (*clear)(const unsigned int&);
	  void (*clearcolor)(const float&, const float&, const float&, const float&);
	  texture_core *(*gen_texture)(const int&, const int&, unsigned char*);
	  void (*bind_texture)(texture_core*);
	  void (*set_texture_param)(const int&, const int&);
	  void (*delete_texture)(texture_core*);
	  void (*flat_render)(flat_vertex*, unsigned int);
	  mesh_core *(*gen_mesh)(mesh_core::data*, unsigned int, unsigned short*, unsigned int);
	  void (*mesh_render)(mesh_core**, const unsigned int&);
	  void (*delete_mesh)(mesh_core*);
	}
	//input
	extern {
  	float (*getAccelerometerX)();
  	float (*getAccelerometerY)();
  	float (*getAccelerometerZ)();
  	float (*getGyroscopeX)();
  	float (*getGyroscopeY)();
  	float (*getGyroscopeZ)();
  	int (*getX)(unsigned int);
  	int (*getDeltaX)(unsigned int);
  	int (*getY)(unsigned int);
  	int (*getDeltaY)(unsigned int);
  	bool (*justTouched)();
  	bool (*isTouched)(unsigned int);
  	float (*getPressure)(unsigned int);
  	bool (*isButtonPressed)(int button);
  	bool (*isButtonJustPressed)(int button);
  	bool (*isKeyPressed)(int key);
  	bool (*isKeyJustPressed)(int key);
  	void (*process_event)();
	}
}

#endif //_Included_Engine