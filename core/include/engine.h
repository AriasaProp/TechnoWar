#ifndef ENGINE_INCLUDED_
#define ENGINE_INCLUDED_

#include "util.h"
#define MAX_ASSET_READING 256

enum {
  GRAPHICS_CLEAR_COLOR = 1,
  GRAPHICS_CLEAR_DEPTH = 2,
  GRAPHICS_CLEAR_STENCIL = 4,
};

typedef uint16_t texture; // 16bit uint as texture index
typedef uint16_t mesh;    // use 16bit uint as mesh index
struct flat_vertex {
  vec2 pos, uv;
};
struct mesh_vertex {
  vec3 pos;
  icolor c;
};
typedef uint16_t mesh_index;

struct engine_graphics {
  vec2 (*getScreenSize)();
  void (*toScreenCoordinate)(vec2 *);
  // graphics clear window
  void (*clear)(const int);
  // set graphics clear color window
  void (*clearColor)(const fcolor);
  texture (*genTexture)(const uivec2, void *);
  void (*bindTexture)(const texture);
  void (*setTextureParam)(const int, const int);
  void (*deleteTexture)(const texture);
  void (*flatRender)(const texture, struct flat_vertex *, const size_t);
  mesh (*genMesh)(struct mesh_vertex *, const size_t, mesh_index *, const size_t);
  void (*setMeshTransform)(const mesh, float *);
  void (*meshRender)(mesh *, const size_t);
  void (*deleteMesh)(const mesh);
};
struct engine_asset {
  void *data;
  int (*assetBuffer)(const char *, const void **, int *);
  int (*openAsset)(const char *);
  int (*assetRead)(int, void *, size_t);
  size_t (*assetLength)(int);
  void (*assetClose)(int);
};
struct engine_input {
  void *data;
  vec2 (*getTouch)(size_t);
  void (*funct2)();
};
struct engine_extras {
  void *data;
  void (*funct2)();
};

struct engine {
  struct engine_graphics g;
  struct engine_asset a;
  struct engine_input i;
  struct engine_extras e;
};

extern struct engine global_engine;

#endif // ENGINE_INCLUDED_