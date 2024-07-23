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
struct graphics_core {
  virtual float getWidth () = 0;
  virtual float getHeight () = 0;
  virtual void clear (const unsigned int &) = 0;
  virtual void clearcolor (const float &, const float &, const float &, const float &) = 0;
  virtual texture_core *gen_texture (const int &, const int &, unsigned char *) = 0;
  virtual void bind_texture (texture_core *) = 0;
  virtual void set_texture_param (const int &, const int &) = 0;
  virtual void delete_texture (texture_core *) = 0;
  virtual void flat_render (texture_core *, flat_vertex *, const unsigned int) = 0;
  virtual mesh_core *gen_mesh (mesh_core::data *, unsigned int, unsigned short *, unsigned int) = 0;
  virtual void mesh_render (mesh_core **, const unsigned int &) = 0;
  virtual void delete_mesh (mesh_core *) = 0;
  virtual void to_flat_coordinate (float &, float &) = 0;
};
struct sensor_value {
  float x, y, z;
};
struct input_core {
  virtual sensor_value getSensorValue (const char *) const = 0;
  virtual void getPointerPos (float *, unsigned int) = 0;
  virtual void getPointerDelta (float *, unsigned int) = 0;
  virtual bool justTouched () = 0;
  virtual bool onTouched () = 0;
  virtual bool isTouched (unsigned int) = 0;
  virtual float getPressure (unsigned int) = 0;
  virtual bool isButtonPressed (int button) = 0;
  virtual bool isButtonJustPressed (int button) = 0;
  virtual bool isKeyPressed (int key) = 0;
  virtual bool isKeyJustPressed (int key) = 0;
  virtual void process_event () = 0;
  virtual ~input_core () {}
};
struct asset_core {
  virtual size_t read (void *, size_t) = 0;
  virtual void seek (int) = 0;
  virtual bool eof () = 0;
  virtual ~asset_core () {}
};
struct assets_core {
  virtual asset_core *open_asset (const char *) = 0;
  virtual void *asset_buffer (const char *, unsigned int *) = 0;
  virtual ~assets_core () {}
};
struct info_core {
  virtual size_t memory () = 0;
};
extern graphics_core *graph;
extern input_core *inpt;
extern assets_core *asset;
extern info_core *info;
} // namespace engine

#endif //_Included_Engine