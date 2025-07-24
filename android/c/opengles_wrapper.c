#include <android/native_window.h>

#include "engine.h"
#include "glad.h"
#include "glad_egl.h"
#include "log.h"
#include "util.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define MAX_MSG 512
static GLint success;
static GLchar msg[MAX_MSG];

static void getErrorGL(const char *X) {
  static GLenum error;
  while ((error = glGetError()))
    LOGE("Err %s 0x%x\n", X, error);
}
static void checkLinkProgram(GLint X) {
  glLinkProgram(X);
  glGetProgramiv(X, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(X, MAX_MSG, NULL, msg);
    LOGE("Shader link: %s", msg);
  }
}
static void checkCompileShader(GLint X) {
  glCompileShader(X);
  glGetShaderiv(X, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(X, MAX_MSG, NULL, msg);
    LOGE("Shader compile: %s", msg);
  }
}

#define check(X) \
  X;             \
  getErrorGL(#X)
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
  WORLD_UPDATE = 1 << 1,
  RESIZE_DISPLAY = 1 << 2,
  RESIZE_ONLY = 1 << 3,
};
// private function
enum {
  TERM_EGL_SURFACE = 1,
  TERM_EGL_CONTEXT = 2,
  TERM_EGL_DISPLAY = 4,
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
static struct androidGraphics {
  ANativeWindow *window;
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  EGLConfig eConfig;
  int flags;

  struct {
    GLint shader, uniform_proj, uniform_tex;
    GLuint vao, vbo, ibo;
  } ui;
  struct {
    GLint shader, uniform_proj, uniform_transProj;
  } world;

  vec2 viewportSize; //
  vec2 screenSize;   //
  vec4 insets;

  void *egllib, *gleslib;

} *src = NULL;

// core implementation
static vec2 opengles_getScreenSize() { return src->screenSize; }
static void opengles_toScreenCoordinate(vec2 *v) {
  v->x -= src->insets.x;
  v->y = src->viewportSize.y - v->y - src->insets.w;
}
static void opengles_clear(const int m) {
  check(glClear(
    (((m & GRAPHICS_CLEAR_COLOR) == GRAPHICS_CLEAR_COLOR) * GL_COLOR_BUFFER_BIT) |
    (((m & GRAPHICS_CLEAR_DEPTH) == GRAPHICS_CLEAR_DEPTH) * GL_DEPTH_BUFFER_BIT) |
    (((m & GRAPHICS_CLEAR_STENCIL) == GRAPHICS_CLEAR_STENCIL) * GL_STENCIL_BUFFER_BIT)));
}
static void opengles_clearColor(const fcolor c) {
  check(glClearColor(c.r, c.g, c.b, c.a));
}
static texture opengles_genTexture(const uivec2 size, void *data) {
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
static void opengles_bindTexture(const texture t) {
  check(glBindTexture(GL_TEXTURE_2D, textures[t].id));
}
static void opengles_setTextureParam(const int param, const int val) {
  check(glTexParameteri(GL_TEXTURE_2D, param, val));
}
static void opengles_deleteTexture(const texture t) {
  check(glDeleteTextures(1, &textures[t].id));
  free(textures[t].data);
  memset((void *)(textures + t), 0, sizeof(struct opengles_texture));
}
static void opengles_flatRender(const texture t, struct flat_vertex *v, const size_t l) {
  check(glDisable(GL_DEPTH_TEST));
  check(glUseProgram(src->ui.shader));
  if (src->flags & UI_UPDATE) {
    static float mat[16];
    matrix4_idt(mat);
    mat[0] = 2.f / src->viewportSize.x;
    mat[5] = 2.f / src->viewportSize.y;
    mat[12] = (2.0f * src->insets.x / src->viewportSize.x) - 1.0f;
    mat[13] = (2.0f * src->insets.w / src->viewportSize.y) - 1.0f;
    check(glUniformMatrix4fv(src->ui.uniform_proj, 1, GL_FALSE, mat));
    src->flags &= ~UI_UPDATE;
  }
  check(glActiveTexture(GL_TEXTURE0));
  check(glBindTexture(GL_TEXTURE_2D, textures[t].id));
  check(glUniform1i(src->ui.uniform_tex, 0));
  check(glBindVertexArray(src->ui.vao));
  check(glBindBuffer(GL_ARRAY_BUFFER, src->ui.vbo));
  check(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * l * sizeof(struct flat_vertex), (void *)v));
  check(glDrawElements(GL_TRIANGLES, 6 * l, GL_UNSIGNED_SHORT, NULL));
  check(glBindVertexArray(0));
  check(glBindTexture(GL_TEXTURE_2D, 0));
  check(glUseProgram(0));
}
static mesh opengles_genMesh(struct mesh_vertex *v, const size_t vl, mesh_index *i, const size_t il) {
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
static void opengles_setMeshTransform(const mesh ms, float *mat) {
  memcpy((void *)meshes[ms].trans, mat, 16 * sizeof(float));
}
static void opengles_meshRender(mesh *ms, const size_t l) {
  check(glEnable(GL_DEPTH_TEST));
  check(glUseProgram(src->world.shader));
  if (src->flags & WORLD_UPDATE) {
    static float mat[16];
    matrix4_idt(mat);
    mat[0] = 2.f / src->viewportSize.x;
    mat[5] = 2.f / src->viewportSize.y;
    check(glUniformMatrix4fv(src->world.uniform_proj, 1, GL_FALSE, mat));
    src->flags &= ~WORLD_UPDATE;
  }
  for (size_t i = 0; i < l; i++) {
    struct opengles_mesh m = meshes[ms[i]];
    check(glUniformMatrix4fv(src->world.uniform_transProj, 1, GL_FALSE, m.trans));
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
static void opengles_deleteMesh(mesh m) {
  check(glDeleteVertexArrays(1, &meshes[m].vao));
  check(glDeleteBuffers(2, &meshes[m].vbo));
  free(meshes[m].vertexs);
  free(meshes[m].indices);
  memset(meshes + m, 0, sizeof(struct opengles_mesh));
}

static void killEGL(const int EGLTermReq) {
  if (!EGLTermReq || !src->display)
    return;
  if (textures[0].id) {
    // world draw
    check(glDeleteProgram(src->world.shader));
    // flat draw
    check(glDeleteProgram(src->ui.shader));
    check(glDeleteVertexArrays(1, &src->ui.vao));
    check(glDeleteBuffers(2, &src->ui.vbo));
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
  eglMakeCurrent(src->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  if (src->surface && (EGLTermReq & 5)) {
    // invalidate Framebuffer, RenderBuffer
    eglDestroySurface(src->display, src->surface);
    src->surface = EGL_NO_SURFACE;
  }
  if (src->context && (EGLTermReq & 6)) {
    // invalidating gles
    eglDestroyContext(src->display, src->context);
    src->context = EGL_NO_CONTEXT;
  }
  if (EGLTermReq & 4) {
    eglTerminate(src->display);
    src->display = EGL_NO_DISPLAY;
  }
}
// android purpose
#include <dlfcn.h>
static void *lpegl(const char *name) {
  return dlsym(src->egllib, name);
}
static void *lpgles(const char *name) {
  return dlsym(src->gleslib, name);
}

void androidGraphics_init() {
  src = (struct androidGraphics *)calloc(1, sizeof(struct androidGraphics));
  src->egllib = dlopen("libEGL.so", RTLD_NOW | RTLD_LOCAL);
  if (!src->egllib) {
    LOGE("dll error %s", dlerror());
    return;
  }
  src->gleslib = dlopen("libGLESv3.so", RTLD_NOW | RTLD_LOCAL);
  if (!src->gleslib) {
    LOGE("dll error %s", dlerror());
    return;
  }
  gladLoadEGL(lpegl);
  gladLoadGLES(lpgles);

  global_engine.g.getScreenSize = opengles_getScreenSize;
  global_engine.g.toScreenCoordinate = opengles_toScreenCoordinate;
  global_engine.g.clear = opengles_clear;
  global_engine.g.clearColor = opengles_clearColor;
  global_engine.g.genTexture = opengles_genTexture;
  global_engine.g.bindTexture = opengles_bindTexture;
  global_engine.g.setTextureParam = opengles_setTextureParam;
  global_engine.g.deleteTexture = opengles_deleteTexture;
  global_engine.g.flatRender = opengles_flatRender;
  global_engine.g.genMesh = opengles_genMesh;
  global_engine.g.setMeshTransform = opengles_setMeshTransform;
  global_engine.g.meshRender = opengles_meshRender;
  global_engine.g.deleteMesh = opengles_deleteMesh;

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
void androidGraphics_onWindowCreate(void *w) {
  src->window = (ANativeWindow *)w;
}
void androidGraphics_onWindowDestroy() {
  killEGL(TERM_EGL_SURFACE);
  src->window = NULL;
}
void androidGraphics_onWindowResizeDisplay() {
  src->flags |= RESIZE_DISPLAY;
}
void androidGraphics_onWindowResize() {
  src->flags |= RESIZE_ONLY;
}
void androidGraphics_resizeInsets(float x, float y, float z, float w) {
  src->insets.x = x;
  src->insets.y = y;
  src->insets.z = z;
  src->insets.w = w;
  src->screenSize.x = src->viewportSize.x - x - z;
  src->screenSize.y = src->viewportSize.y - y - w;
  src->flags |= UI_UPDATE;
}
int androidGraphics_preRender() {
  if (!src->window)
    return 0;
  if (!src->display || !src->context || !src->surface) {
    if (!src->display) {
      src->context = EGL_NO_CONTEXT;
      src->surface = EGL_NO_SURFACE;
      src->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
      if (!src->display) {
        LOGW("Failed to get EGLDisplay");
        return 0;
      }
      EGLint temp, temp1;
      eglInitialize(src->display, &temp, &temp1);
      if (temp < 1 || temp1 < 3) { // unsupported egl version lower than 1.3
        LOGW("EGL version is below 1.3");
        return 0;
      }
      const EGLint configAttr[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
        EGL_BUFFER_SIZE, 16,
        EGL_NONE};
      eglChooseConfig(src->display, configAttr, NULL, 0, &temp);
      if (temp <= 0) {
        LOGW("There is no fit config for minimum EGLConfig attribute");
        return 0;
      }
      EGLConfig *configs = (EGLConfig *)malloc(temp * sizeof(EGLConfig));
      eglChooseConfig(src->display, configAttr, configs, temp, &temp);
      size_t i = 0, j = temp, k = 0, l;
      do {
        l = 1;
#define EGL_CONFIG_EVA(X)                                     \
  if (eglGetConfigAttrib(src->display, configs[i], X, &temp)) \
  l += temp
        EGL_CONFIG_EVA(EGL_BUFFER_SIZE);
        EGL_CONFIG_EVA(EGL_DEPTH_SIZE);
        EGL_CONFIG_EVA(EGL_STENCIL_SIZE);
        // TODO: and more attributes
#undef EGL_CONFIG_EVA
        if (l > k) {
          k = l;
          src->eConfig = configs[i];
        }
      } while ((++i) < j);
      free(configs);
    }
    if (!src->context) {
      const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
      src->context = eglCreateContext(src->display, src->eConfig, NULL, ctxAttr);
      if (!src->context) {
        LOGW("Failed to create EGLContext");
        return 0;
      }
    }
    if (!src->surface) {
      src->surface = eglCreateWindowSurface(src->display, src->eConfig, src->window, NULL);
      if (!src->surface) {
        LOGW("Failed to create EGLSurface");
        return 0;
      }
    }
    eglMakeCurrent(src->display, src->surface, src->surface, src->context);
    if (textures[0].id == 0) {
      // when validate, projection need to be update
      src->flags |= WORLD_UPDATE | UI_UPDATE;
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
        src->ui.shader = check(glCreateProgram());
        GLuint vi = check(glCreateShader(GL_VERTEX_SHADER));
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
        check(glShaderSource(vi, 1, &vt, 0));
        checkCompileShader(vi);
        check(glAttachShader(src->ui.shader, vi));
        GLuint fi = check(glCreateShader(GL_FRAGMENT_SHADER));
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
        check(glShaderSource(fi, 1, &ft, 0));
        checkCompileShader(fi);
        check(glAttachShader(src->ui.shader, fi));
        checkLinkProgram(src->ui.shader);
        check(glDeleteShader(vi));
        check(glDeleteShader(fi));
        src->ui.uniform_proj = check(glGetUniformLocation(src->ui.shader, "u_proj"));
        src->ui.uniform_tex = check(glGetUniformLocation(src->ui.shader, "u_tex"));
        check(glGenVertexArrays(1, &src->ui.vao));
        check(glGenBuffers(2, &src->ui.vbo));
        check(glBindVertexArray(src->ui.vao));
        unsigned short indexs[MAX_UI_DRAW * 6];
        for (unsigned short i = 0, j = 0, k = 0; i < MAX_UI_DRAW; i++, j += 6) {
          indexs[j] = k++;
          indexs[j + 1] = indexs[j + 5] = k++;
          indexs[j + 2] = indexs[j + 4] = k++;
          indexs[j + 3] = k++;
        }
        // 0, 1, 2, 3, 2, 1
        check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, src->ui.ibo));
        check(glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_UI_DRAW * 6 * sizeof(unsigned short), (void *)indexs, GL_STATIC_DRAW));
        check(glBindBuffer(GL_ARRAY_BUFFER, src->ui.vbo));
        check(glBufferData(GL_ARRAY_BUFFER, MAX_UI_DRAW * 4 * sizeof(struct flat_vertex), NULL, GL_DYNAMIC_DRAW));
        check(glEnableVertexAttribArray(0));
        check(glEnableVertexAttribArray(1));
        check(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct flat_vertex), (void *)0));
        check(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct flat_vertex), (void *)sizeof(vec2)));
      }
      // world draw
      {
        src->world.shader = check(glCreateProgram());
        GLuint vi = check(glCreateShader(GL_VERTEX_SHADER));
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
        check(glShaderSource(vi, 1, &vt, 0));
        checkCompileShader(vi);
        check(glAttachShader(src->world.shader, vi));
        GLuint fi = check(glCreateShader(GL_FRAGMENT_SHADER));
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
        check(glShaderSource(fi, 1, &ft, 0));
        checkCompileShader(fi);
        check(glAttachShader(src->world.shader, fi));
        checkLinkProgram(src->world.shader);
        check(glDeleteShader(vi));
        check(glDeleteShader(fi));
        src->world.uniform_proj = check(glGetUniformLocation(src->world.shader, "worldview_proj"));
        src->world.uniform_transProj = check(glGetUniformLocation(src->world.shader, "trans_proj"));
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
    src->flags |= RESIZE_ONLY;
    src->flags &= ~RESIZE_DISPLAY;
  } else if (src->flags & RESIZE_DISPLAY) {
    eglMakeCurrent(src->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglMakeCurrent(src->display, src->surface, src->surface, src->context);
    src->flags |= RESIZE_ONLY;
    src->flags &= ~RESIZE_DISPLAY;
  }
  if (src->flags & RESIZE_ONLY) {
    EGLint w, h;
    eglQuerySurface(src->display, src->surface, EGL_WIDTH, &w);
    eglQuerySurface(src->display, src->surface, EGL_HEIGHT, &h);
    check(glViewport(0, 0, w, h));
    src->viewportSize.x = (float)w;
    src->viewportSize.y = (float)h;
    src->screenSize.x = src->viewportSize.x - src->insets.x - src->insets.z;
    src->screenSize.y = src->viewportSize.y - src->insets.y - src->insets.w;
    src->flags |= WORLD_UPDATE | UI_UPDATE;
    src->flags &= ~RESIZE_ONLY;
  }
  check(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
  return 1;
}
void androidGraphics_postRender() {
  int EGLTermReq = 0;
  if (!eglSwapBuffers(src->display, src->surface)) {
    switch (eglGetError()) {
    case EGL_BAD_SURFACE:
    case EGL_BAD_NATIVE_WINDOW:
    case EGL_BAD_CURRENT_SURFACE:
      EGLTermReq |= TERM_EGL_SURFACE;
      break;
    case EGL_BAD_CONTEXT:
    case EGL_CONTEXT_LOST:
      EGLTermReq |= TERM_EGL_CONTEXT;
      break;
    case EGL_NOT_INITIALIZED:
    case EGL_BAD_DISPLAY:
      EGLTermReq |= TERM_EGL_DISPLAY;
      break;
    default:
      break;
    }
  }
  killEGL(EGLTermReq);
}
void androidGraphics_term() {
  if (textures[0].id) {
    // world draw
    check(glDeleteProgram(src->world.shader));
    // flat draw
    check(glDeleteProgram(src->ui.shader));
    check(glDeleteVertexArrays(1, &src->ui.vao));
    check(glDeleteBuffers(2, &src->ui.vbo));
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

  eglSwapBuffers(src->display, src->surface);
  if (src->display) {
    eglMakeCurrent(src->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (src->surface) {
      eglDestroySurface(src->display, src->surface);
      src->surface = EGL_NO_SURFACE;
    }
    if (src->context) {
      eglDestroyContext(src->display, src->context);
      src->context = EGL_NO_CONTEXT;
    }
    eglTerminate(src->display);
    src->display = EGL_NO_DISPLAY;
  }
  dlclose(src->egllib);
  dlclose(src->gleslib);
  free(src);
}
