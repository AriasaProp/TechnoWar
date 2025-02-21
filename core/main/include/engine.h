#ifndef ENGINE_INCLUDED_
#define ENGINE_INCLUDED_

#include "util.h"

enum {
  GRAPHICS_CLEAR_COLOR = 1,
  GRAPHICS_CLEAR_DEPTH = 2,
  GRAPHICS_CLEAR_STENCIL = 4,
};

typedef uint16_t texture; // 16bit uint as texture index
typedef uint16_t mesh;    // use 16bit uint as mesh index
struct flat_vertex {
  struct vec2 pos, uv;
};
struct mesh_vertex {
  struct vec3 pos;
  struct icolor c;
};
typedef uint16_t mesh_index;

struct engine_graphics {
  struct vec2 (*getScreenSize) ();
  void (*toScreenCoordinate) (struct vec2 *);
  // graphics clear window
  void (*clear) (const int);
  // set graphics clear color window
  void (*clearColor) (const struct fcolor);
  texture (*genTexture) (const struct uivec2, void *);
  void (*bindTexture) (const texture);
  void (*setTextureParam) (const int, const int);
  void (*deleteTexture) (const texture);
  void (*flatRender) (const texture, struct flat_vertex *, const size_t);
  mesh (*genMesh) (struct mesh_vertex *, const size_t, mesh_index *, const size_t);
  void (*meshRender) (mesh *, const size_t);
  void (*deleteMesh) (const mesh);
};
struct engine_asset {
  void *data;
  void (*funct1) ();
  void (*funct2) ();
};
struct engine_input {
  void *data;
  vec2 (*getTouch) (size_t);
  void (*funct2) ();
};
struct engine_extras {
  void *data;
  void (*funct2) ();
};

struct engine {
  struct engine_graphics g;
  struct engine_asset a;
  struct engine_input i;
  struct engine_extras e;
};

struct engine *engine_init ();
struct engine *get_engine ();

#endif // ENGINE_INCLUDED_