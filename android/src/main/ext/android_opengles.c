#include <EGL/egl.h>
#include <GLES3/gl32.h> //API 24

#include "engine.h"
#include "manager.h"
#include "util.h"

#include <inttypes.h>

#define MAX_UI_DRAW 200
#define MAX_RESOURCE 256
// mesh flags for uniform update
enum {
  MESH_VERTEX_DIRTY = 1,
  MESH_INDEX_DIRTY = 2,
};
// flags global 2d/3d uniform update
enum {
  UI_UPDATE = 1,
  WORLD_UPDATE = 2,
  VIEWPORT_UPDATE = 4,
  VALID_RESOURCES = 8,
};

static struct opengles_texture {
  GLuint id;
  struct uivec2 size;
  void *data;
} *textures;
static struct opengles_mesh {
  GLuint vao, vbo, ibo;
  int flags;
  size_t vertex_len, index_len;
  struct mesh_vertex *vertexs;
  mesh_index *indices;
  float trans[16];
} *meshes;
static struct opengles_data {
  int flags;
  struct {
    GLint shader, uniform_proj, uniform_tex;
    GLuint vao, vbo, ibo;
  } ui;
  struct {
    GLint shader, uniform_proj, uniform_transProj;
  } world;

  struct vec2 viewportSize; //
  struct vec2 screenSize;   //
  struct vec4 insets;
} src = {0};

// core implementation
static struct vec2 android_opengles_getScreenSize () { return src.screenSize; }
static void android_opengles_toScreenCoordinate (struct vec2 *v) {
  v->x -= src.insets.x;
  v->y = src.viewportSize.y - v->y - src.insets.w;
}
static void android_opengles_clear (const int m) {
  glClear (
      (((m & GRAPHICS_CLEAR_COLOR) == GRAPHICS_CLEAR_COLOR) * GL_COLOR_BUFFER_BIT) |
      (((m & GRAPHICS_CLEAR_DEPTH) == GRAPHICS_CLEAR_DEPTH) * GL_DEPTH_BUFFER_BIT) |
      (((m & GRAPHICS_CLEAR_STENCIL) == GRAPHICS_CLEAR_STENCIL) * GL_STENCIL_BUFFER_BIT));
}
static void android_opengles_clearColor (const struct fcolor c) {
  glClearColor (c.r, c.g, c.b, c.a);
}
static texture android_opengles_genTexture (const struct uivec2 size, void *data) {
  texture i = 1;
  while (i < MAX_RESOURCE) {
    if (textures[i].size.x == 0)
      break;
    ++i;
  }
  if (i >= MAX_RESOURCE)
    return 0; // reach limit texture total so return default texture
  textures[i].size = size;
  textures[i].data = data;
  glGenTextures (1, &textures[i].id);
  glBindTexture (GL_TEXTURE_2D, textures[i].id);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture (GL_TEXTURE_2D, 0);
  return i;
}
static void android_opengles_bindTexture (const texture t) {
  glBindTexture (GL_TEXTURE_2D, textures[t].id);
}
static void android_opengles_setTextureParam (const int param, const int val) {
  glTexParameteri (GL_TEXTURE_2D, param, val);
}
static void android_opengles_deleteTexture (const texture t) {
  glDeleteTextures (1, &textures[t].id);
  free_mem (textures[t].data);
  memset (textures + t, 0, sizeof (struct opengles_texture));
}
static void android_opengles_flatRender (const texture t, struct flat_vertex *v, const size_t l) {
  glDisable (GL_DEPTH_TEST);
  glUseProgram (src.ui.shader);
  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_2D, textures[t].id);
  glUniform1i (src.ui.uniform_tex, 0);
  if (src.flags & UI_UPDATE) {
  	memset(stemp.mat, 0, 16 * sizeof(float));
    stemp.mat[0] = 2.f / src.viewportSize.x;
    stemp.mat[5] = 2.f / src.viewportSize.y;
    stemp.mat[10] = 0.00001f;
    stemp.mat[12] = (2.0f * src.insets.x / src.viewportSize.x) - 1.0f;
    stemp.mat[13] = (2.0f * src.insets.w / src.viewportSize.y) - 1.0f;
    stemp.mat[15] = 1.f;
    glUniformMatrix4fv (src.ui.uniform_proj, 1, GL_FALSE, stemp.mat);
    src.flags &= ~UI_UPDATE;
  }
  glBindVertexArray (src.ui.vao);
  glBindBuffer (GL_ARRAY_BUFFER, src.ui.vbo);
  glBufferSubData (GL_ARRAY_BUFFER, 0, 4 * l * sizeof (struct flat_vertex), (void *)v);
  glDrawElements (GL_TRIANGLES, 6 * l, GL_UNSIGNED_SHORT, NULL);
  glBindVertexArray (0);
  glBindTexture (GL_TEXTURE_2D, 0);
  glUseProgram (0);
}
static mesh android_opengles_genMesh (struct mesh_vertex *v, const size_t vl, mesh_index *i, const size_t il) {
  mesh m = 0;
  while (m < MAX_RESOURCE) {
    if (meshes[m].vertex_len == 0)
      break;
    ++m;
  }
  if (m >= MAX_RESOURCE)
    return -1; // reach limit mesh total so return invalid number
  meshes[m].vertex_len = vl;
  meshes[m].vertexs = v;
  meshes[m].index_len = vl;
  meshes[m].indices = i;
  matrix4_idt (meshes[m].trans);

  glGenVertexArrays (1, &meshes[m].vao);
  glGenBuffers (2, &meshes[m].vbo);
  glBindVertexArray (meshes[m].vao);
  glBindBuffer (GL_ARRAY_BUFFER, meshes[m].vbo);
  glBufferData (GL_ARRAY_BUFFER, vl * sizeof (struct mesh_vertex), (void *)v, GL_STATIC_DRAW);
  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof (struct mesh_vertex), (void *)0);
  glEnableVertexAttribArray (1);
  glVertexAttribPointer (1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (struct mesh_vertex), (void *)sizeof (struct vec3));
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, meshes[m].ibo);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, il * sizeof (mesh_index), (void *)i, GL_STATIC_DRAW);
  glBindVertexArray (0);
  meshes[m].flags |= MESH_VERTEX_DIRTY | MESH_INDEX_DIRTY;
  return m;
}
static void android_opengles_setMeshTransform (const mesh ms, float *mat) {
  memcpy (meshes[ms].trans, mat, 16 * sizeof (float));
}
static void android_opengles_meshRender (mesh *ms, const size_t l) {
  glEnable (GL_DEPTH_TEST);
  glUseProgram (src.world.shader);
  if (src.flags & WORLD_UPDATE) {
    memset(stemp.mat, 0, 16 * sizeof(float));
    stemp.mat[0] = 2.f / src.viewportSize.x;
    stemp.mat[5] = 2.f / src.viewportSize.y;
    stemp.mat[10] = 1.f;
    stemp.mat[15] = 1.f;
    glUniformMatrix4fv (src.world.uniform_proj, 1, GL_FALSE, stemp.mat);
    src.flags &= ~WORLD_UPDATE;
  }
  for (size_t i = 0; i < l; i++) {
    struct opengles_mesh m = meshes[ms[i]];
    glUniformMatrix4fv (src.world.uniform_transProj, 1, GL_FALSE, m.trans);
    glBindVertexArray (m.vao);
    if (m.flags) {
      if (m.flags & MESH_VERTEX_DIRTY) {
        glBindBuffer (GL_ARRAY_BUFFER, m.vbo);
        glBufferSubData (GL_ARRAY_BUFFER, 0, m.vertex_len * sizeof (struct mesh_vertex), (void *)m.vertexs);
      }
      if (m.flags & MESH_INDEX_DIRTY) {
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m.ibo);
        glBufferSubData (GL_ELEMENT_ARRAY_BUFFER, 0, m.index_len * sizeof (mesh_index), (void *)m.indices);
      }
      m.flags = 0;
    }
    glDrawElements (GL_TRIANGLES, m.index_len, GL_UNSIGNED_SHORT, NULL);
  }
  glBindVertexArray (0);
  glUseProgram (0);
}
static void android_opengles_deleteMesh (mesh m) {
  glDeleteVertexArrays (1, &meshes[m].vao);
  glDeleteBuffers (2, &meshes[m].vbo);
  free_mem (meshes[m].vertexs);
  free_mem (meshes[m].indices);
  memset (meshes + m, 0, sizeof (struct opengles_mesh));
}

static unsigned char nullTextureData[4] = {0xff, 0xff, 0xff, 0xff};
void android_opengles_validateResources () {
  if (src.flags & VALID_RESOURCES) return;
  // cullface to front
  glEnable (GL_CULL_FACE);
  glCullFace (GL_FRONT);
  // enable depth
  glDepthRangef (0.0f, 1.0f);
  glDepthFunc (GL_LESS);
  glClearColor (1.0f, 0.0f, 0.0f, 1.0f);
  // enable blend
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // flat draw
  {
    src.ui.shader = glCreateProgram ();
    GLuint vi = glCreateShader (GL_VERTEX_SHADER);
    const char *vt = "#version 300 es"
                     "\n#define LOW lowp"
                     "\n#define MED mediump"
                     "\n#ifdef GL_FRAGMENT_PRECISION_HIGH"
                     "\n    #define HIGH highp"
                     "\n#else"
                     "\n    #define HIGH mediump"
                     "\n#endif"
                     "\nuniform mat4 u_proj;"
                     "\nlayout(location = 0) in vec4 a_position;"
                     "\nlayout(location = 1) in vec2 a_texCoord;"
                     "\nout vec2 v_texCoord;"
                     "\nvoid main() {"
                     "\n    v_texCoord = a_texCoord;"
                     "\n    gl_Position = u_proj * a_position;"
                     "\n}";
    glShaderSource (vi, 1, &vt, 0);
    glCompileShader (vi);
    glAttachShader (src.ui.shader, vi);
    GLuint fi = glCreateShader (GL_FRAGMENT_SHADER);
    const char *ft = "#version 300 es"
                     "\n#define LOW lowp"
                     "\n#define MED mediump"
                     "\n#ifdef GL_FRAGMENT_PRECISION_HIGH"
                     "\n    #define HIGH highp"
                     "\n#else"
                     "\n    #define HIGH mediump"
                     "\n#endif"
                     "\nprecision MED float;"
                     "\nuniform sampler2D u_tex;"
                     "\nin vec2 v_texCoord;"
                     "\nlayout(location = 0) out vec4 fragColor;"
                     "\nvoid main() {"
                     "\n    fragColor = texture(u_tex, v_texCoord);"
                     "\n}";
    glShaderSource (fi, 1, &ft, 0);
    glCompileShader (fi);
    glAttachShader (src.ui.shader, fi);
    glLinkProgram (src.ui.shader);
    glDeleteShader (vi);
    glDeleteShader (fi);
    src.ui.uniform_proj = glGetUniformLocation (src.ui.shader, "u_proj");
    src.ui.uniform_tex = glGetUniformLocation (src.ui.shader, "u_tex");
    glGenVertexArrays (1, &src.ui.vao);
    glGenBuffers (2, &src.ui.vbo);
    glBindVertexArray (src.ui.vao);
    unsigned short indexs[MAX_UI_DRAW * 6];
    for (unsigned short i = 0, j = 0, k = 0; i < MAX_UI_DRAW; i++, j += 6) {
      indexs[j] = k++;
      indexs[j + 1] = indexs[j + 5] = k++;
      indexs[j + 2] = indexs[j + 4] = k++;
      indexs[j + 3] = k++;
    }
    // 0, 1, 2, 3, 2, 1
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, src.ui.ibo);
    glBufferData (GL_ELEMENT_ARRAY_BUFFER, MAX_UI_DRAW * 6 * sizeof (unsigned short), (void *)indexs, GL_STATIC_DRAW);
    glBindBuffer (GL_ARRAY_BUFFER, src.ui.vbo);
    glBufferData (GL_ARRAY_BUFFER, MAX_UI_DRAW * 4 * sizeof (struct flat_vertex), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, sizeof (struct flat_vertex), (void *)0);
    glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, sizeof (struct flat_vertex), (void *)sizeof (struct vec2));
    glEnableVertexAttribArray (0);
    glEnableVertexAttribArray (1);
  }
  // world draw
  {
    src.world.shader = glCreateProgram ();
    GLuint vi = glCreateShader (GL_VERTEX_SHADER);
    const char *vt = "#version 300 es"
                     "\n#define LOW lowp"
                     "\n#define MED mediump"
                     "\n#ifdef GL_FRAGMENT_PRECISION_HIGH"
                     "\n    #define HIGH highp"
                     "\n#else"
                     "\n    #define HIGH mediump"
                     "\n#endif"
                     "\nuniform mat4 worldview_proj;"
                     "\nuniform mat4 trans_proj;"
                     "\nlayout(location = 0) in vec4 a_position;"
                     "\nlayout(location = 1) in vec4 a_color;"
                     "\nout vec4 v_color;"
                     "\nvoid main() {"
                     "\n    v_color = a_color;"
                     "\n    gl_Position = worldview_proj * trans_proj * a_position;"
                     "\n}";
    glShaderSource (vi, 1, &vt, 0);
    glCompileShader (vi);
    glAttachShader (src.world.shader, vi);
    GLuint fi = glCreateShader (GL_FRAGMENT_SHADER);
    const char *ft = "#version 300 es"
                     "\n#define LOW lowp"
                     "\n#define MED mediump"
                     "\n#ifdef GL_FRAGMENT_PRECISION_HIGH"
                     "\n    #define HIGH highp"
                     "\n#else"
                     "\n    #define HIGH mediump"
                     "\n#endif"
                     "\nprecision MED float;"
                     "\nin vec4 v_color;"
                     "\nlayout(location = 0) out vec4 fragColor;"
                     "\nvoid main() {"
                     "\n    fragColor = v_color;"
                     "\n}";
    glShaderSource (fi, 1, &ft, 0);
    glCompileShader (fi);
    glAttachShader (src.world.shader, fi);
    glLinkProgram (src.world.shader);
    glDeleteShader (vi);
    glDeleteShader (fi);
    src.world.uniform_proj = glGetUniformLocation (src.world.shader, "worldview_proj");
    src.world.uniform_transProj = glGetUniformLocation (src.world.shader, "trans_proj");
  }
  // texture
  // made for 0 texture test
  glGenTextures (1, &textures[0].id);
  textures[0].size.x = 1;
  textures[0].size.y = 1;
  glBindTexture (GL_TEXTURE_2D, textures[0].id);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)nullTextureData);

  for (texture t = 1; t < MAX_RESOURCE; ++t) {
    if (textures[t].size.x == 0) continue;
    glGenTextures (1, &textures[t].id);
    glBindTexture (GL_TEXTURE_2D, textures[t].id);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, textures[t].size.x, textures[t].size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, textures[t].data);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
  glBindTexture (GL_TEXTURE_2D, 0);
  // mesh
  for (mesh m = 0; m < MAX_RESOURCE; ++m) {
    if (meshes[m].vertex_len == 0) continue;
    glGenVertexArrays (1, &meshes[m].vao);
    glGenBuffers (2, &meshes[m].vbo);
    glBindVertexArray (meshes[m].vao);
    glBindBuffer (GL_ARRAY_BUFFER, meshes[m].vbo);
    glBufferData (GL_ARRAY_BUFFER, meshes[m].vertex_len * sizeof (struct mesh_vertex), (void *)meshes[m].vertexs, GL_STATIC_DRAW);
    glEnableVertexAttribArray (0);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof (struct mesh_vertex), (void *)0);
    glEnableVertexAttribArray (1);
    glVertexAttribPointer (1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (struct mesh_vertex), (void *)sizeof (struct vec3));
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, meshes[m].ibo);
    glBufferData (GL_ELEMENT_ARRAY_BUFFER, meshes[m].index_len * sizeof (mesh_index), (void *)meshes[m].indices, GL_STATIC_DRAW);
  }
  glBindVertexArray (0);
  src.flags |= VALID_RESOURCES;
}
void android_opengles_preRender () {
  glClearDepthf (1.0f);
  glClear (GL_COLOR_BUFFER_BIT);
}
void android_opengles_resizeInsets (float x, float y, float z, float w) {
  src.insets.x = x;
  src.insets.y = y;
  src.insets.z = z;
  src.insets.w = w;
  src.screenSize.x = src.viewportSize.x - x - z;
  src.screenSize.y = src.viewportSize.y - y - w;
  src.flags |= UI_UPDATE;
}
void android_opengles_resizeWindow (float w, float h) {
  src.viewportSize.x = w;
  src.viewportSize.y = h;
  glViewport (0, 0, w, h);
  src.screenSize.x = w - src.insets.x - src.insets.z;
  src.screenSize.y = h - src.insets.y - src.insets.w;
  src.flags |= WORLD_UPDATE;
}
void android_opengles_invalidateResources () {
  if (!(src.flags & VALID_RESOURCES)) return;
  // world draw
  glDeleteProgram (src.world.shader);
  // flat draw
  glDeleteProgram (src.ui.shader);
  glDeleteVertexArrays (1, &src.ui.vao);
  glDeleteBuffers (2, &src.ui.vbo);
  // mesh
  for (mesh i = 0; i < MAX_RESOURCE; ++i) {
    if (meshes[i].vertex_len == 0) continue;
    glDeleteVertexArrays (1, &meshes[i].vao);
    glDeleteBuffers (2, &meshes[i].vbo);
  }
  // texture
  for (texture i = 0; i < MAX_RESOURCE; ++i) {
    if (textures[i].size.x == 0) continue;
    glDeleteTextures (1, &textures[i].id);
  }

  src.flags &= ~VALID_RESOURCES;
}

void android_opengles_init () {
  get_engine ()->g.getScreenSize = android_opengles_getScreenSize;
  get_engine ()->g.toScreenCoordinate = android_opengles_toScreenCoordinate;
  get_engine ()->g.clear = android_opengles_clear;
  get_engine ()->g.clearColor = android_opengles_clearColor;
  get_engine ()->g.genTexture = android_opengles_genTexture;
  get_engine ()->g.bindTexture = android_opengles_bindTexture;
  get_engine ()->g.setTextureParam = android_opengles_setTextureParam;
  get_engine ()->g.deleteTexture = android_opengles_deleteTexture;
  get_engine ()->g.flatRender = android_opengles_flatRender;
  get_engine ()->g.genMesh = android_opengles_genMesh;
  get_engine ()->g.setMeshTransform = android_opengles_setMeshTransform;
  get_engine ()->g.meshRender = android_opengles_meshRender;
  get_engine ()->g.deleteMesh = android_opengles_deleteMesh;

  textures = (struct opengles_texture *)new_imem (sizeof (struct opengles_texture) * MAX_RESOURCE);
  meshes = (struct opengles_mesh *)new_imem (sizeof (struct opengles_mesh) * MAX_RESOURCE);
}
void android_opengles_term () {
  if (src.flags & VALID_RESOURCES) {
    // world draw
    glDeleteProgram (src.world.shader);
    // flat draw
    glDeleteProgram (src.ui.shader);
    glDeleteVertexArrays (1, &src.ui.vao);
    glDeleteBuffers (2, &src.ui.vbo);
    // texture
    for (texture i = 0; i < MAX_RESOURCE; ++i) {
      if (textures[i].size.x == 0) continue;
      glDeleteTextures (1, &textures[i].id);
    }
    // mesh
    for (mesh i = 0; i < MAX_RESOURCE; ++i) {
      if (meshes[i].vertex_len == 0) continue;
      glDeleteVertexArrays (1, &meshes[i].vao);
      glDeleteBuffers (2, &meshes[i].vbo);
    }
    src.flags &= ~VALID_RESOURCES;
  }

  free_mem (textures);
  free_mem (meshes);
  memset (&src, 0, sizeof (struct opengles_data));
}