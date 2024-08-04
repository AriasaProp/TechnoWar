#ifndef _Included_Engine
#define _Included_Engine 1
// maximum ui draws
#define MAX_UI_DRAW 1024
// maximum output log message in char
#define MAX_GL_MSG 1024

#include <cstddef>
#include <cstdint>

namespace engine {
// texture core
struct texture_core {
  virtual unsigned int width () = 0;
  virtual unsigned int height () = 0;
  virtual ~texture_core (){};
};
// ui_core
struct flat_vertex {
  float x, y;
  uint32_t color;
  float u, v;
};
// mesh core
struct mesh_core {
  bool dirty_vertex, dirty_index;
  unsigned int vao, vbo, ibo;
  unsigned int vertex_len, index_len;
  struct data {
    float pos[3];
    uint32_t color;
  } * vertex;
  unsigned short *index;
  float trans[16]{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
};
struct sensor_value {
  float x, y, z;
};
struct asset {
  virtual size_t read (void *, size_t) = 0;
  virtual void seek (int) = 0;
  virtual bool eof () = 0;
  virtual ~asset_core () {}
};

// Pointer definitions for graphics namespace
namespace graphics {
  extern float (*getWidth)();
  extern float (*getHeight)();
  extern void (*clear)(const unsigned int &);
  extern void (*clearcolor)(const float &, const float &, const float &, const float &);
  extern texture_core* (*gen_texture)(const int &, const int &, unsigned char *);
  extern void (*bind_texture)(texture_core *);
  extern void (*set_texture_param)(const int &, const int &);
  extern void (*delete_texture)(texture_core *);
  extern void (*flat_render)(texture_core *, flat_vertex *, const unsigned int);
  extern mesh_core* (*gen_mesh)(mesh_core::data *, unsigned int, unsigned short *, unsigned int);
  extern void (*mesh_render)(mesh_core **, const unsigned int &);
  extern void (*delete_mesh)(mesh_core *);
  extern void (*to_flat_coordinate)(float &, float &);
}

// Pointer definitions for input namespace
namespace input {
  extern sensor_value (*getSensorValue)(const char *);
  extern void (*getPointerPos)(float *, unsigned int);
  extern void (*getPointerDelta)(float *, unsigned int);
  extern bool (*justTouched)();
  extern bool (*onTouched)();
  extern bool (*isTouched)(unsigned int);
  extern float (*getPressure)(unsigned int);
  extern bool (*isButtonPressed)(int button);
  extern bool (*isButtonJustPressed)(int button);
  extern bool (*isKeyPressed)(int key);
  extern bool (*isKeyJustPressed)(int key);
  extern void (*process_event)();
}

// Pointer definitions for assets namespace
namespace assets {
  extern asset* (*open_asset)(const char *);
  extern void* (*asset_buffer)(const char *, unsigned int *);
}

// Pointer definitions for info namespace
namespace info {
  extern const char* (*get_platform_info)();
  extern long (*memory)();
}

} // namespace engine

#endif //_Included_Engine