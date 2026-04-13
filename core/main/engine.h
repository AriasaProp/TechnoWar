#ifndef ENGINE_INCLUDED_
#define ENGINE_INCLUDED_

#include "common.h"
#define MAX_ASSET_READING 256
#define MAX_UI_DRAW       200

enum {
  GRAPHICS_CLEAR_COLOR = 1,
  GRAPHICS_CLEAR_DEPTH = 2,
  GRAPHICS_CLEAR_STENCIL = 4,
};

typedef uint16_t texture; // 16bit uint as texture index
typedef uint16_t mesh;    // use 16bit uint as mesh index
typedef struct {
  vec2 pos, uv;
} flat_vertex;
typedef struct {
  vec3 pos;
  icolor c;
} mesh_vertex;
typedef uint16_t mesh_index;

struct engine {

  const char *(*engine_graphics_info)(void);
  // graphics function
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
  void (*flatRender)(const texture, flat_vertex *, const size_t);
  mesh (*genMesh)(mesh_vertex *, const size_t, mesh_index *, const size_t);
  void (*setMeshTransform)(const mesh, float *);
  void (*meshRender)(mesh *, const size_t);
  void (*deleteMesh)(const mesh);

  // time function
  float (*getDeltaTimeMs)(void);
  size_t (*getFPS)(void);

  // asset function
  void *(*assetBuffer)(const char *, const void **, size_t *);
  void *(*openAsset)(const char *);
  int (*assetRead)(void *, void *, size_t);
  void (*assetSeek)(void *, int);
  size_t (*assetLength)(void *);
  void (*assetClose)(void *);

  // input function
  vec2 (*getTouch)(size_t);
};

extern struct engine global_engine;

#endif // ENGINE_INCLUDED_