#include "engine.h"
#include "log.h"
#include "util.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <EGL/egl.h>
#include <GLES3/gl32.h> //API 24
#include <android/native_window.h>

// opengles wrapper

#define MAX_GL_ERR_MSG 512
static GLint success;
static GLchar msg[MAX_GL_ERR_MSG];
static GLenum error;

#define check(X)                 \
  X;                             \
  while ((error = glGetError())) \
  LOGE("GLErr in %s with (0x%x)\n", #X, error)
static void checkLinkProgram(GLint X) {
  glLinkProgram(X);
  glGetProgramiv(X, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(X, MAX_GL_ERR_MSG, NULL, msg);
    LOGE("Program shader linking error: %s", msg);
  }
}
static void checkCompileShader(GLint X) {
  glCompileShader(X);
  glGetShaderiv(X, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(X, MAX_GL_ERR_MSG, NULL, msg);
    LOGE("Shader compiling error: %s", msg);
  }
}

#define MAX_UI_DRAW  200
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
};

static struct opengles_texture {
  GLuint id;
  uivec2 size;
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
  struct
  {
    GLint shader, uniform_proj, uniform_tex;
    GLuint vao, vbo, ibo;
  } ui;
  struct
  {
    GLint shader, uniform_proj, uniform_transProj;
  } world;

  vec2 viewportSize; //
  vec2 screenSize;   //
  vec4 insets;
} src = {0};

// core implementation
static vec2 android_opengles_getScreenSize() { return src.screenSize; }
static void android_opengles_toScreenCoordinate(vec2 *v) {
  v->x -= src.insets.x;
  v->y = src.viewportSize.y - v->y - src.insets.w;
}
static void android_opengles_clear(const int m) {
  check(glClear(
    (((m & GRAPHICS_CLEAR_COLOR) == GRAPHICS_CLEAR_COLOR) * GL_COLOR_BUFFER_BIT) |
    (((m & GRAPHICS_CLEAR_DEPTH) == GRAPHICS_CLEAR_DEPTH) * GL_DEPTH_BUFFER_BIT) |
    (((m & GRAPHICS_CLEAR_STENCIL) == GRAPHICS_CLEAR_STENCIL) * GL_STENCIL_BUFFER_BIT)));
}
static void android_opengles_clearColor(const fcolor c) {
  check(glClearColor(c.r, c.g, c.b, c.a));
}
static texture android_opengles_genTexture(const uivec2 size, void *data) {
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
  check(glGenTextures(1, &textures[i].id));
  check(glBindTexture(GL_TEXTURE_2D, textures[i].id));
  check(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
  check(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
  check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  check(glBindTexture(GL_TEXTURE_2D, 0));
  return i;
}
static void android_opengles_bindTexture(const texture t) {
  check(glBindTexture(GL_TEXTURE_2D, textures[t].id));
}
static void android_opengles_setTextureParam(const int param, const int val) {
  check(glTexParameteri(GL_TEXTURE_2D, param, val));
}
static void android_opengles_deleteTexture(const texture t) {
  check(glDeleteTextures(1, &textures[t].id));
  free(textures[t].data);
  memset(textures + t, 0, sizeof(struct opengles_texture));
}
static void android_opengles_flatRender(const texture t, struct flat_vertex *v, const size_t l) {
  check(glDisable(GL_DEPTH_TEST));
  check(glUseProgram(src.ui.shader));
  if (src.flags & UI_UPDATE) {
    float mat[16];
    matrix4_idt(mat);
    mat[0] = 2.f / src.viewportSize.x;
    mat[5] = 2.f / src.viewportSize.y;
    mat[12] = (2.0f * src.insets.x / src.viewportSize.x) - 1.0f;
    mat[13] = (2.0f * src.insets.w / src.viewportSize.y) - 1.0f;
    check(glUniformMatrix4fv(src.ui.uniform_proj, 1, GL_FALSE, mat));
    src.flags &= ~UI_UPDATE;
  }
  check(glActiveTexture(GL_TEXTURE0));
  check(glBindTexture(GL_TEXTURE_2D, textures[t].id));
  check(glUniform1i(src.ui.uniform_tex, 0));
  check(glBindVertexArray(src.ui.vao));
  check(glBindBuffer(GL_ARRAY_BUFFER, src.ui.vbo));
  check(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * l * sizeof(struct flat_vertex), (void *)v));
  check(glDrawElements(GL_TRIANGLES, 6 * l, GL_UNSIGNED_SHORT, NULL));
  check(glBindVertexArray(0));
  check(glBindTexture(GL_TEXTURE_2D, 0));
  check(glUseProgram(0));
}
static mesh android_opengles_genMesh(struct mesh_vertex *v, const size_t vl, mesh_index *i, const size_t il) {
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
  matrix4_idt(meshes[m].trans);

  check(glGenVertexArrays(1, &meshes[m].vao));
  check(glGenBuffers(2, &meshes[m].vbo));
  check(glBindVertexArray(meshes[m].vao));
  check(glBindBuffer(GL_ARRAY_BUFFER, meshes[m].vbo));
  check(glBufferData(GL_ARRAY_BUFFER, vl * sizeof(struct mesh_vertex), (void *)v, GL_STATIC_DRAW));
  check(glEnableVertexAttribArray(0));
  check(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct mesh_vertex), (void *)0));
  check(glEnableVertexAttribArray(1));
  check(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct mesh_vertex), (void *)sizeof(vec3)));
  check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[m].ibo));
  check(glBufferData(GL_ELEMENT_ARRAY_BUFFER, il * sizeof(mesh_index), (void *)i, GL_STATIC_DRAW));
  check(glBindVertexArray(0));
  meshes[m].flags |= MESH_VERTEX_DIRTY | MESH_INDEX_DIRTY;
  return m;
}
static void android_opengles_setMeshTransform(const mesh ms, float *mat) {
  memcpy(meshes[ms].trans, mat, 16 * sizeof(float));
}
static void android_opengles_meshRender(mesh *ms, const size_t l) {
  check(glEnable(GL_DEPTH_TEST));
  check(glUseProgram(src.world.shader));
  if (src.flags & WORLD_UPDATE) {
    float mat[16];
    matrix4_idt(mat);
    mat[0] = 2.f / src.viewportSize.x;
    mat[5] = 2.f / src.viewportSize.y;
    check(glUniformMatrix4fv(src.world.uniform_proj, 1, GL_FALSE, mat));
    src.flags &= ~WORLD_UPDATE;
  }
  for (size_t i = 0; i < l; i++) {
    struct opengles_mesh m = meshes[ms[i]];
    check(glUniformMatrix4fv(src.world.uniform_transProj, 1, GL_FALSE, m.trans));
    check(glBindVertexArray(m.vao));
    if (m.flags & MESH_VERTEX_DIRTY) {
      check(glBindBuffer(GL_ARRAY_BUFFER, m.vbo));
      check(glBufferSubData(GL_ARRAY_BUFFER, 0, m.vertex_len * sizeof(struct mesh_vertex), (void *)m.vertexs));
      m.flags &= ~MESH_VERTEX_DIRTY;
    }
    if (m.flags & MESH_INDEX_DIRTY) {
      check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ibo));
      check(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m.index_len * sizeof(mesh_index), (void *)m.indices));
      m.flags &= ~MESH_INDEX_DIRTY;
    }
    m.flags = 0;
    check(glDrawElements(GL_TRIANGLES, m.index_len, GL_UNSIGNED_SHORT, NULL));
  }
  check(glBindVertexArray(0));
  check(glUseProgram(0));
}
static void android_opengles_deleteMesh(mesh m) {
  check(glDeleteVertexArrays(1, &meshes[m].vao));
  check(glDeleteBuffers(2, &meshes[m].vbo));
  free(meshes[m].vertexs);
  free(meshes[m].indices);
  memset(meshes + m, 0, sizeof(struct opengles_mesh));
}

static void android_opengles_validateResources() {
  if (textures[0].id != 0)
    return;
  // when validate, projection need to be update
  src.flags |= WORLD_UPDATE | UI_UPDATE;
  // set clear
  check(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
  // cullface to front
  check(glEnable(GL_CULL_FACE));
  check(glCullFace(GL_FRONT));
  // enable depth
  check(glDepthFunc(GL_LESS));
  check(glDepthRangef(0.0f, 1.0f));
  check(glClearDepthf(1.0f));
  // enable blend
  check(glEnable(GL_BLEND));
  check(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  // flat draw
  {
    src.ui.shader = check(glCreateProgram());
    GLuint vi = check(glCreateShader(GL_VERTEX_SHADER));
    const char *vt = "#version 320 es"
                     "\n#ifdef GL_FRAGMENT_PRECISION_HIGH"
                     "\n		precision highp float;"
                     "\n#else"
                     "\n		precision mediump float;"
                     "\n#endif"
                     "\nuniform mat4 u_proj;"
                     "\nlayout(location = 0) in vec4 a_position;"
                     "\nlayout(location = 1) in vec2 a_texCoord;"
                     "\nout vec2 v_texCoord;"
                     "\nvoid main() {"
                     "\n    v_texCoord = a_texCoord;"
                     "\n    gl_Position = u_proj * a_position;"
                     "\n}";
    check(glShaderSource(vi, 1, &vt, 0));
    checkCompileShader(vi);
    check(glAttachShader(src.ui.shader, vi));
    GLuint fi = check(glCreateShader(GL_FRAGMENT_SHADER));
    const char *ft = "#version 320 es"
                     "\n#ifdef GL_FRAGMENT_PRECISION_HIGH"
                     "\n		precision highp float;"
                     "\n#else"
                     "\n		precision mediump float;"
                     "\n#endif"
                     "\nprecision MED float;"
                     "\nuniform sampler2D u_tex;"
                     "\nin vec2 v_texCoord;"
                     "\nlayout(location = 0) out vec4 fragColor;"
                     "\nvoid main() {"
                     "\n    fragColor = texture(u_tex, v_texCoord);"
                     "\n}";
    check(glShaderSource(fi, 1, &ft, 0));
    checkCompileShader(fi);
    check(glAttachShader(src.ui.shader, fi));
    checkLinkProgram(src.ui.shader);
    check(glDeleteShader(vi));
    check(glDeleteShader(fi));
    src.ui.uniform_proj = check(glGetUniformLocation(src.ui.shader, "u_proj"));
    src.ui.uniform_tex = check(glGetUniformLocation(src.ui.shader, "u_tex"));
    check(glGenVertexArrays(1, &src.ui.vao));
    check(glGenBuffers(2, &src.ui.vbo));
    check(glBindVertexArray(src.ui.vao));
    unsigned short indexs[MAX_UI_DRAW * 6];
    for (unsigned short i = 0, j = 0, k = 0; i < MAX_UI_DRAW; i++, j += 6) {
      indexs[j] = k++;
      indexs[j + 1] = indexs[j + 5] = k++;
      indexs[j + 2] = indexs[j + 4] = k++;
      indexs[j + 3] = k++;
    }
    // 0, 1, 2, 3, 2, 1
    check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, src.ui.ibo));
    check(glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_UI_DRAW * 6 * sizeof(unsigned short), (void *)indexs, GL_STATIC_DRAW));
    check(glBindBuffer(GL_ARRAY_BUFFER, src.ui.vbo));
    check(glBufferData(GL_ARRAY_BUFFER, MAX_UI_DRAW * 4 * sizeof(struct flat_vertex), NULL, GL_DYNAMIC_DRAW));
    check(glEnableVertexAttribArray(0));
    check(glEnableVertexAttribArray(1));
    check(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct flat_vertex), (void *)0));
    check(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct flat_vertex), (void *)sizeof(vec2)));
  }
  // world draw
  {
    src.world.shader = check(glCreateProgram());
    GLuint vi = check(glCreateShader(GL_VERTEX_SHADER));
    const char *vt = "#version 320 es"
                     "\n#ifdef GL_FRAGMENT_PRECISION_HIGH"
                     "\n		precision highp float;"
                     "\n#else"
                     "\n		precision mediump float;"
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
    check(glShaderSource(vi, 1, &vt, 0));
    checkCompileShader(vi);
    check(glAttachShader(src.world.shader, vi));
    GLuint fi = check(glCreateShader(GL_FRAGMENT_SHADER));
    const char *ft = "#version 320 es"
                     "\n#ifdef GL_FRAGMENT_PRECISION_HIGH"
                     "\n		precision highp float;"
                     "\n#else"
                     "\n		precision mediump float;"
                     "\n#endif"
                     "\nin vec4 v_color;"
                     "\nlayout(location = 0) out vec4 fragColor;"
                     "\nvoid main() {"
                     "\n    fragColor = v_color;"
                     "\n}";
    check(glShaderSource(fi, 1, &ft, 0));
    checkCompileShader(fi);
    check(glAttachShader(src.world.shader, fi));
    checkLinkProgram(src.world.shader);
    check(glDeleteShader(vi));
    check(glDeleteShader(fi));
    src.world.uniform_proj = check(glGetUniformLocation(src.world.shader, "worldview_proj"));
    src.world.uniform_transProj = check(glGetUniformLocation(src.world.shader, "trans_proj"));
  }
  // texture
  // start from 0 to validate default texture
  for (texture t = 0; t < MAX_RESOURCE; ++t) {
    if (textures[t].size.x == 0)
      continue;
    check(glGenTextures(1, &textures[t].id));
    check(glBindTexture(GL_TEXTURE_2D, textures[t].id));
    check(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    check(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textures[t].size.x, textures[t].size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, textures[t].data));
    check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  }
  check(glBindTexture(GL_TEXTURE_2D, 0));
  // mesh
  for (mesh m = 0; m < MAX_RESOURCE; ++m) {
    if (meshes[m].vertex_len == 0)
      continue;
    check(glGenVertexArrays(1, &meshes[m].vao));
    check(glGenBuffers(2, &meshes[m].vbo));
    check(glBindVertexArray(meshes[m].vao));
    check(glBindBuffer(GL_ARRAY_BUFFER, meshes[m].vbo));
    check(glBufferData(GL_ARRAY_BUFFER, meshes[m].vertex_len * sizeof(struct mesh_vertex), (void *)meshes[m].vertexs, GL_STATIC_DRAW));
    check(glEnableVertexAttribArray(0));
    check(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct mesh_vertex), (void *)0));
    check(glEnableVertexAttribArray(1));
    check(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct mesh_vertex), (void *)sizeof(vec3)));
    check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[m].ibo));
    check(glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshes[m].index_len * sizeof(mesh_index), (void *)meshes[m].indices, GL_STATIC_DRAW));
  }
  check(glBindVertexArray(0));
}
static void android_opengles_preRender() {
  check(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

static void android_opengles_resizeInsets(float x, float y, float z, float w) {
  src.insets.x = x;
  src.insets.y = y;
  src.insets.z = z;
  src.insets.w = w;
  src.screenSize.x = src.viewportSize.x - x - z;
  src.screenSize.y = src.viewportSize.y - y - w;
  src.flags |= UI_UPDATE;
}
static void android_opengles_resizeWindow(float w, float h) {
  src.viewportSize.x = w;
  src.viewportSize.y = h;
  check(glViewport(0, 0, w, h));
  src.screenSize.x = w - src.insets.x - src.insets.z;
  src.screenSize.y = h - src.insets.y - src.insets.w;
  src.flags |= WORLD_UPDATE | UI_UPDATE;
}
static void android_opengles_invalidateResources() {
  if (textures[0].id == 0)
    return;
  // world draw
  check(glDeleteProgram(src.world.shader));
  // flat draw
  check(glDeleteProgram(src.ui.shader));
  check(glDeleteVertexArrays(1, &src.ui.vao));
  check(glDeleteBuffers(2, &src.ui.vbo));
  // mesh
  for (mesh i = 0; i < MAX_RESOURCE; ++i) {
    if (meshes[i].vertex_len == 0)
      continue;
    check(glDeleteVertexArrays(1, &meshes[i].vao));
    check(glDeleteBuffers(2, &meshes[i].vbo));
  }
  // texture
  for (texture i = 0; i < MAX_RESOURCE; ++i) {
    if (textures[i].size.x == 0)
      continue;
    check(glDeleteTextures(1, &textures[i].id));
    textures[i].id = 0;
  }
}

static void android_opengles_init() {
  get_engine()->g.getScreenSize = android_opengles_getScreenSize;
  get_engine()->g.toScreenCoordinate = android_opengles_toScreenCoordinate;
  get_engine()->g.clear = android_opengles_clear;
  get_engine()->g.clearColor = android_opengles_clearColor;
  get_engine()->g.genTexture = android_opengles_genTexture;
  get_engine()->g.bindTexture = android_opengles_bindTexture;
  get_engine()->g.setTextureParam = android_opengles_setTextureParam;
  get_engine()->g.deleteTexture = android_opengles_deleteTexture;
  get_engine()->g.flatRender = android_opengles_flatRender;
  get_engine()->g.genMesh = android_opengles_genMesh;
  get_engine()->g.setMeshTransform = android_opengles_setMeshTransform;
  get_engine()->g.meshRender = android_opengles_meshRender;
  get_engine()->g.deleteMesh = android_opengles_deleteMesh;

  textures = (struct opengles_texture *)calloc(sizeof(struct opengles_texture), MAX_RESOURCE);
  {
    // add default texture
    textures[0].size.x = 1;
    textures[0].size.y = 1;
    textures[0].data = malloc(4);
    memset(textures[0].data, 0xff, 4);
  }
  meshes = (struct opengles_mesh *)calloc(sizeof(struct opengles_mesh), MAX_RESOURCE);
}
static void android_opengles_term() {
  if (textures[0].id != 0) {
    // world draw
    check(glDeleteProgram(src.world.shader));
    // flat draw
    check(glDeleteProgram(src.ui.shader));
    check(glDeleteVertexArrays(1, &src.ui.vao));
    check(glDeleteBuffers(2, &src.ui.vbo));
    // texture
    for (texture i = 0; i < MAX_RESOURCE; ++i) {
      if (textures[i].size.x == 0)
        continue;
      check(glDeleteTextures(1, &textures[i].id));
      free(textures[i].data);
    }
    // mesh
    for (mesh i = 0; i < MAX_RESOURCE; ++i) {
      if (meshes[i].vertex_len == 0)
        continue;
      check(glDeleteVertexArrays(1, &meshes[i].vao));
      check(glDeleteBuffers(2, &meshes[i].vbo));
      free(meshes[i].vertexs);
      free(meshes[i].indices);
    }
  }

  free(textures);
  free(meshes);
  memset(&src, 0, sizeof(struct opengles_data));
}

// private value
static struct android_graphicsManager {
  ANativeWindow *window;
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  EGLConfig eConfig;
  int flags;
} *g = 0;

// private function
enum {
  TERM_EGL_SURFACE = 1,
  TERM_EGL_CONTEXT = 2,
  TERM_EGL_DISPLAY = 4,
};
enum {
  RESIZE_DISPLAY = 2,
  RESIZE_ONLY = 1,
};

static inline void killEGL(const int EGLTermReq) {
  if (!EGLTermReq || !g->display)
    return;
  android_opengles_invalidateResources();
  eglMakeCurrent(g->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  if (g->surface && (EGLTermReq & 5)) {
    // invalidate Framebuffer, RenderBuffer
    eglDestroySurface(g->display, g->surface);
    g->surface = EGL_NO_SURFACE;
  }
  if (g->context && (EGLTermReq & 6)) {
    // invalidating gles
    eglDestroyContext(g->display, g->context);
    g->context = EGL_NO_CONTEXT;
  }
  if (EGLTermReq & 4) {
    eglTerminate(g->display);
    g->display = EGL_NO_DISPLAY;
  }
}
// android purpose
void androidGraphics_init() {
  android_opengles_init();
  g = (struct android_graphicsManager *)calloc(1, sizeof(struct android_graphicsManager));
}
void androidGraphics_onWindowCreate(void *w) {
  g->window = (ANativeWindow *)w;
}
void androidGraphics_onWindowDestroy() {
  killEGL(TERM_EGL_SURFACE);
  g->window = NULL;
}
void androidGraphics_onWindowResizeDisplay() {
  g->flags |= RESIZE_DISPLAY;
}
void androidGraphics_onWindowResize() {
  g->flags |= RESIZE_ONLY;
}
void androidGraphics_resizeInsets(float x, float y, float z, float w) {
  android_opengles_resizeInsets(x, y, z, w);
}
int androidGraphics_preRender() {
  if (!g->window)
    return 0;
  if (!g->display || !g->context || !g->surface) {
    LOGV("Start preRender resources build");
    if (!g->display) {
      g->context = EGL_NO_CONTEXT;
      g->surface = EGL_NO_SURFACE;
      g->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
      if (!g->display) {
        // likely never happen
        LOGV("Cannot get default EGL display?");
        return 0;
      }

      EGLint temp, temp1;
      eglInitialize(g->display, &temp, &temp1);
      if (temp < 1 || temp1 < 3) { // unsupported egl version lower than 1.3
        LOGV("Unsupported EGL version");
        return 0;
      }
      const EGLint configAttr[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_CONFORMANT, EGL_OPENGL_ES2_BIT, EGL_BUFFER_SIZE, 16, EGL_NONE};
      eglChooseConfig(g->display, configAttr, NULL, 0, &temp);
      if (temp <= 0) { // likely never happen
        LOGV("EGL config is to high.");
        return 0;
      }
      EGLConfig *conf = (EGLConfig *)malloc(temp * sizeof(EGLConfig));
      EGLConfig *configs = conf;
      EGLConfig *configs_end = configs + temp;
      eglChooseConfig(g->display, configAttr, configs, temp, &temp);
      g->eConfig = *configs;
      size_t k = 0, l;
      do {
        l = 0;

#define EGL_CHECK_CONFIG(X)                               \
  if (eglGetConfigAttrib(g->display, *configs, X, &temp)) \
  l += temp
        EGL_CHECK_CONFIG(EGL_BUFFER_SIZE);
        EGL_CHECK_CONFIG(EGL_DEPTH_SIZE);
        EGL_CHECK_CONFIG(EGL_STENCIL_SIZE);
        // TODO: and more ....
#undef EGL_CHECK_CONFIG

        if (l > k) {
          k = l;
          g->eConfig = *configs;
        }
      } while (++configs < configs_end);
      free(conf);
    }
    if (!g->context) {
      const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
      g->context = eglCreateContext(g->display, g->eConfig, NULL, ctxAttr);
      if (!g->context) {
        LOGV("failed to create EGL context.");
        return 0;
      }
    }
    if (!g->surface) {
      g->surface = eglCreateWindowSurface(g->display, g->eConfig, g->window, NULL);
      if (!g->surface) {
        LOGV("failed to create EGL surface.");
        return 0;
      }
    }

    eglMakeCurrent(g->display, g->surface, g->surface, g->context);
    android_opengles_validateResources();
    g->flags |= RESIZE_ONLY;
    g->flags &= ~RESIZE_DISPLAY;
  } else if (g->flags & RESIZE_DISPLAY) {
    eglMakeCurrent(g->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglMakeCurrent(g->display, g->surface, g->surface, g->context);
    g->flags |= RESIZE_ONLY;
    g->flags &= ~RESIZE_DISPLAY;
  }
  if (g->flags & RESIZE_ONLY) {
    EGLint w, h;
    eglQuerySurface(g->display, g->surface, EGL_WIDTH, &w);
    eglQuerySurface(g->display, g->surface, EGL_HEIGHT, &h);
    android_opengles_resizeWindow((float)w, (float)h);
    g->flags &= ~RESIZE_ONLY;
  }
  android_opengles_preRender();
  return 1;
}
void androidGraphics_postRender() {
  if (!eglSwapBuffers(g->display, g->surface)) {
    switch (eglGetError()) {
    case EGL_BAD_SURFACE:
    case EGL_BAD_NATIVE_WINDOW:
    case EGL_BAD_CURRENT_SURFACE:
      killEGL(TERM_EGL_SURFACE);
      break;
    case EGL_BAD_CONTEXT:
    case EGL_CONTEXT_LOST:
      killEGL(TERM_EGL_CONTEXT);
      break;
    case EGL_NOT_INITIALIZED:
    case EGL_BAD_DISPLAY:
      killEGL(TERM_EGL_DISPLAY);
      break;
    default:
      break;
    }
  }
}
void androidGraphics_term() {
  eglSwapBuffers(g->display, g->surface);
  android_opengles_term();
  if (g->display) {
    eglMakeCurrent(g->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (g->surface) {
      eglDestroySurface(g->display, g->surface);
      g->surface = EGL_NO_SURFACE;
    }
    if (g->context) {
      eglDestroyContext(g->display, g->context);
      g->context = EGL_NO_CONTEXT;
    }
    eglTerminate(g->display);
    g->display = EGL_NO_DISPLAY;
  }
  memset(&get_engine()->g, 0, sizeof(struct engine_graphics));
  free(g);
}
