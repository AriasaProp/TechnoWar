#ifndef _Included_Engine
#define _Included_Engine
// maximum ui draws
#define MAX_UI_DRAW 100
// maximum output log message in char
#define MAX_GL_MSG 1024

namespace engine {
// texture core
struct texture_core {
  virtual unsigned int width() = 0;
  virtual unsigned int height() = 0;
  virtual unsigned char *data() = 0;
  virtual ~texture_core(){};
};
// ui_core
struct flat_vertex {
  float x, y;
  unsigned char color[4];
  float u, v;
};
// mesh core
struct mesh_core {
  bool dirty_vertex, dirty_index;
  unsigned int vao, vbo, ibo;
  unsigned int vertex_len, index_len;
  struct data {
    float x, y, z;
    unsigned char r, g, b, a;
  } *vertex = nullptr;
  unsigned short *index = nullptr;
  float trans[16]{
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1};
};
struct graphics_core {
  virtual float getWidth() = 0;
  virtual float getHeight() = 0;
  virtual void clear(const unsigned int &) = 0;
  virtual void clearcolor(const float &, const float &, const float &, const float &) = 0;
  virtual texture_core *gen_texture(const int &, const int &, unsigned char *) = 0;
  virtual void bind_texture(texture_core *) = 0;
  virtual void set_texture_param(const int &, const int &) = 0;
  virtual void delete_texture(texture_core *) = 0;
  virtual void flat_render(texture_core *, flat_vertex *, const unsigned int) = 0;
  virtual mesh_core *gen_mesh(mesh_core::data *, unsigned int, unsigned short *, unsigned int) = 0;
  virtual void mesh_render(mesh_core **, const unsigned int &) = 0;
  virtual void delete_mesh(mesh_core *) = 0;
};
struct input_core {
  virtual float *getSensorValue(const char *) = 0;
  virtual int getX(unsigned int) = 0;
  virtual int getDeltaX(unsigned int) = 0;
  virtual int getY(unsigned int) = 0;
  virtual int getDeltaY(unsigned int) = 0;
  virtual bool justTouched() = 0;
  virtual bool isTouched(unsigned int) = 0;
  virtual float getPressure(unsigned int) = 0;
  virtual bool isButtonPressed(int button) = 0;
  virtual bool isButtonJustPressed(int button) = 0;
  virtual bool isKeyPressed(int key) = 0;
  virtual bool isKeyJustPressed(int key) = 0;
  virtual void process_event() = 0;
  virtual ~input_core() {}
};
struct asset_core {
  virtual ~asset_core() {}
};
struct assets_core {
  virtual asset_core *open_asset(const char *) = 0;
  virtual unsigned int read_asset(asset_core *, void *, unsigned int) = 0;
  virtual void seek_asset(asset_core *, int) = 0;
  virtual bool eof_asset(asset_core *) = 0;
  virtual void close_asset(asset_core *) = 0;
  virtual ~assets_core() {}
};
extern graphics_core *graph;
extern input_core *inpt;
extern assets_core *asset;
} // namespace engine

#endif //_Included_Engine