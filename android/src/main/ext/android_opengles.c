#include <EGL/egl.h>
#include <GLES3/gl32.h> //API 24

#include "engine.h"
#include "util.h"
#define MAX_RESOURCE 256
// not yet make default mesh
static struct opengles_texture {
  GLint id;
  uivec2 size;
  void *data;
} *textures;
enum {
  MESH_VERTEX_DIRTY = 1,
  MESH_INDEX_DIRTY = 2,
};
static struct opengles_mesh {
  GLint vao;
  GLuint vbo, ibo;
  int flags;
  size_t vertex_len, index_len;
  struct mesh_vertex *vertexs;
  uint16_t *indices;
  float trans[16];
} *meshes;
static struct opengles_data {
  struct {
    GLint shader, uniform_proj, uniform_tex;
    GLuint vao, vbo, ibo;
    float proj[16];
  } ui;
  struct {
    GLint shader, uniform_proj, uniform_transProj;
    float proj[16];
  } world;
} *src;

static void android_opengles_clear (const int m) {
  glClear (
  	(((m & GRAPHICS_CLEAR_COLOR) == GRAPHICS_CLEAR_COLOR) * GL_COLOR_BUFFER_BIT) |
  	(((m & GRAPHICS_CLEAR_DEPTH) == GRAPHICS_CLEAR_DEPTH) * GL_DEPTH_BUFFER_BIT) |
  	(((m & GRAPHICS_CLEAR_STENCIL) == GRAPHICS_CLEAR_STENCIL) * GL_STENCIL_BUFFER_BIT);
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
  memset (texture + t, 0, sizeof (struct opengles_texture));
}
static void android_opengles_flatRender (const texture t, struct flat_vertex *v, const size_t l) {
  glDisable (GL_DEPTH_TEST);
  glUseProgram (src->ui.shader);
  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_2D, textures[t].id);
  glUniform1i (src->ui.uniform_tex, 0);
  if (((struct android_graphicsManager *)get_engine ()->g.data)->flags & PROJ_UI) {
    glUniformMatrix4fv (src->ui.uniform_proj, 1, GL_FALSE, src->ui.proj);
    ((struct android_graphicsManager *)get_engine ()->g.data)->flags &= ~PROJ_UI;
  }
  glBindVertexArray (src->ui.vao);
  glBindBuffer (GL_ARRAY_BUFFER, src->ui.vbo);
  glBufferSubData (GL_ARRAY_BUFFER, 0, 4 * l * sizeof (struct flat_vertex), (void *)v);
  glDrawElements (GL_TRIANGLES, 6 * l, GL_UNSIGNED_SHORT, NULL);
  glBindVertexArray (0);
  glBindTexture (GL_TEXTURE_2D, 0);
  glUseProgram (0);
}
static mesh android_opengles_genMesh (mesh_vertex *v, const size_t vl, const mesh_index *i, const size_t il) {
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
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof (struct mesh_vertex), (void *)((uintptr)0));
  glEnableVertexAttribArray (1);
  glVertexAttribPointer (1, 4, GL_UNSIGNED_BYTE, true, sizeof (struct mesh_vertex), (void *)((uintptr)sizeof (struct vec3)));
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, meshes[m].ibo);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, il * sizeof (mesh_index), (void *)i, GL_STATIC_DRAW);
  glBindVertexArray (0);
  meshes[m].flags |= MESH_VERTEX_DIRTY | MESH_INDEX_DIRTY;
  return m;
}
static void android_opengles_meshRender (mesh *ms, const size_t l) {
  glEnable (GL_DEPTH_TEST);
  glUseProgram (src->world.shader);
  if (((struct android_graphicsManager *)get_engine ()->g.data)->flags & PROJ_WORLD) {
    glUniformMatrix4fv (src->world.uniform_proj, 1, GL_FALSE, src->world.proj);
    ((struct android_graphicsManager *)get_engine ()->g.data)->flags &= ~PROJ_WORLD;
  }
  for (size_t i = 0; i < l; i++) {
    struct opengles_mesh m = meshes[ms[i]];
    glUniformMatrix4fv (src->world.uniform_transProj, 1, GL_FALSE, m.trans);
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

static int wasValid = 0;
static unsigned char nullTextureData[4] = {0xff, 0xff, 0xff, 0xff};
void android_opengles_validateResources () {
  if (wasValid) return;
  // cullface to front
  glEnable (GL_CULL_FACE);
  glCullFace (GL_FRONT);
  // enable depth
  glDepthRangef (0.0f, 1.0f);
  glClearDepthf (1.0f);
  glDepthFunc (GL_LESS);
  // enable blend
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // flat draw
  {
    src->ui.shader = glCreateProgram ();
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
                     "\nlayout(location = 1) in vec4 a_color;"
                     "\nlayout(location = 2) in vec2 a_texCoord;"
                     "\nout vec4 v_color;"
                     "\nout vec2 v_texCoord;"
                     "\nvoid main() {"
                     "\n    v_color = a_color;"
                     "\n    v_texCoord = a_texCoord;"
                     "\n    gl_Position = u_proj * a_position;"
                     "\n}";
    glShaderSource (vi, 1, &vt, 0);
    glCompileShader (vi);
    glAttachShader (src->ui.shader, vi);
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
                     "\nin vec4 v_color;"
                     "\nin vec2 v_texCoord;"
                     "\nlayout(location = 0) out vec4 fragColor;"
                     "\nvoid main() {"
                     "\n    fragColor = v_color * texture(u_tex, v_texCoord);"
                     "\n}";
    glShaderSource (fi, 1, &ft, 0);
    glCompileShader (fi);
    glAttachShader (src->ui.shader, fi);
    glLinkProgram (src->ui.shader);
    glDeleteShader (vi);
    glDeleteShader (fi);
    src->ui.uniform_proj = glGetUniformLocation (src->ui.shader, "u_proj");
    src->ui.uniform_tex = glGetUniformLocation (src->ui.shader, "u_tex");
    glGenVertexArrays (1, &src->ui.vao);
    glGenBuffers (2, &src->ui.vbo);
    glBindVertexArray (src->ui.vao);
    unsigned short indexs[MAX_UI_DRAW * 6];
    for (unsigned short i = 0, j = 0, k = 0; i < MAX_UI_DRAW; i++, j += 6) {
      indexs[j] = k++;
      indexs[j + 1] = indexs[j + 5] = k++;
      indexs[j + 2] = indexs[j + 4] = k++;
      indexs[j + 3] = k++;
    }
    // 0, 1, 2, 3, 2, 1
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, src->ui.ibo);
    glBufferData (GL_ELEMENT_ARRAY_BUFFER, MAX_UI_DRAW * 6 * sizeof (unsigned short), (void *)indexs, GL_STATIC_DRAW);
    glBindBuffer (GL_ARRAY_BUFFER, src->ui.vbo);
    glBufferData (GL_ARRAY_BUFFER, MAX_UI_DRAW * 4 * sizeof (struct flat_vertex), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, sizeof (struct flat_vertex), (void *)0);
    glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, sizeof (struct flat_vertex), (void *)((uintptr)sizeof (struct vec2)));
    glEnableVertexAttribArray (0);
    glEnableVertexAttribArray (1);
  }
  // world draw
  {
    src->world.shader = glCreateProgram ();
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
    glAttachShader (src->world.shader, vi);
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
    glAttachShader (src->world.shader, fi);
    glLinkProgram (src->world.shader);
    glDeleteShader (vi);
    glDeleteShader (fi);
    src->world.uniform_proj = glGetUniformLocation (src->world.shader, "worldview_proj");
    src->world.uniform_transProj = glGetUniformLocation (src->world.shader, "trans_proj");
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
  for (mesh m = 0; m < MAX_RESOURCE) {
    if (meshes[m].vertex_len == 0) continue;
    glGenVertexArrays (1, &meshes[m].vao);
    glGenBuffers (2, &meshes[m].vbo);
    glBindVertexArray (meshes[m].vao);
    glBindBuffer (GL_ARRAY_BUFFER, meshes[m].vbo);
    glBufferData (GL_ARRAY_BUFFER, meshes[m].vertex_len * sizeof (struct mesh_vertex), (void *)meshes[m].vertex, GL_STATIC_DRAW);
    glEnableVertexAttribArray (0);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof (struct mesh_vertex), (void *)((uintptr)0));
    glEnableVertexAttribArray (1);
    glVertexAttribPointer (1, 4, GL_UNSIGNED_BYTE, true, sizeof (struct mesh_vertex), (void *)((uintptr)sizeof (struct vec3)));
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, meshes[m].ibo);
    glBufferData (GL_ELEMENT_ARRAY_BUFFER, meshes[m].index_len * sizeof (mesh_index), (void *)meshes[m].indices, GL_STATIC_DRAW);
  }
  glBindVertexArray (0);

  wasValid = 1;
}
void android_opengles_invalidateResources () {
  if (!wasValid) return;
  // world draw
  glDeleteProgram (src->world.shader);
  // flat draw
  glDeleteProgram (src->ui.shader);
  glDeleteVertexArrays (1, &src->ui.vao);
  glDeleteBuffers (2, &src->ui.vbo);
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

  wasValid = 0;
}

void android_opengles_init () {
  struct engine_graphics *g = get_engine ()->g;

  g->clear = android_opengles_clear;
  g->clearColor = android_opengles_clearColor;
  g->genTexture = android_opengles_genTexture;
  g->bindTexture = android_opengles_bindTexture;
  g->setTextureParam = android_opengles_setTextureParam;
  g->deleteTexture = android_opengles_deleteTexture;
  g->flatRender = android_opengles_flatRender;
  g->genMesh = android_opengles_genMesh;
  g->meshRender = android_opengles_meshRender;
  g->deleteMesh = android_opengles_deleteMesh;

  textures = (struct opengles_texture *)new_imem (sizeof (struct opengles_texture) * MAX_RESOURCE);
  meshes = (struct opengles_mesh *)new_imem (sizeof (struct opengles_mesh) * MAX_RESOURCE);
  src = (struct opengles_data *)new_imem (sizeof (struct opengles_data));
}
void android_opengles_term () {
  if (wasValid) {
    // world draw
    glDeleteProgram (src->world.shader);
    // flat draw
    glDeleteProgram (src->ui.shader);
    glDeleteVertexArrays (1, &src->ui.vao);
    glDeleteBuffers (2, &src->ui.vbo);
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
    wasValid = 0;
  }

  free_mem (textures);
  free_mem (meshes);
  free_mem (src);
}