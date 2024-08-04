#include <cstddef>
#include <unordered_set>
// make opengles lastest possible version
#include <EGL/egl.h>
#include <GLES3/gl32.h> //API 24

#include "../android_engine.hpp"
#include "../engine.hpp"
#define TERM_EGL_SURFACE 1
#define TERM_EGL_CONTEXT 2
#define TERM_EGL_DISPLAY 4

namespace graphics_opengles {

struct opengles_texture : public engine::texture_core {
  GLuint id;
  unsigned int w, h;
  unsigned char *d;
  opengles_texture (GLint i, unsigned int _w, unsigned int _h, void *dt) : id (i), w (_w), h (_h) {
    d = new unsigned char[w * h * sizeof (unsigned char)];
    memcpy (d, dt, w * h * sizeof (unsigned char));
  }
  unsigned int width () override {
    return w;
  }
  unsigned int height () override {
    return h;
  }
  ~opengles_texture () {
    delete[] d;
  }
};

struct gl_data {
  bool dirty_uiProj;
  bool dirty_worldProj;
  unsigned char resize_state = 0;
  GLint ui_shader;
  GLint u_uiProj;
  GLint u_uiTex;
  GLint world_shader;
  GLint u_worldProj;
  GLint u_worldTransProj;
  GLuint ui_vao, ui_vbo, ui_ibo;
  GLuint nullTextureId; // this is used for null texture needed
  EGLDisplay display = EGL_NO_DISPLAY;
  EGLSurface surface = EGL_NO_SURFACE;
  EGLContext context = EGL_NO_CONTEXT;
  EGLConfig eConfig;
  EGLint wWidth, wHeight;        // platform full display
  float game_width, game_height; // display after safe insets
  //
  float uiProj[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
  float worldProj[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.00001f, 0, 0, 0, 0, 1};
  std::unordered_set<opengles_texture *> managedTexture;
  std::unordered_set<engine::mesh_core *> managedMesh;
} *mgl_data = nullptr;

static ANativeWindow *window = nullptr;

// self
static void killEGL (const unsigned int EGLTermReq) {
  if (!EGLTermReq || !mgl_data->display) return;
  eglMakeCurrent (mgl_data->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  if (mgl_data->surface && (EGLTermReq & 5)) {
    // invalidate Framebuffer, RenderBuffer
    eglDestroySurface (mgl_data->display, mgl_data->surface);
    mgl_data->surface = EGL_NO_SURFACE;
  }
  if (mgl_data->context && (EGLTermReq & 6)) {
    // invalidating gles resources
    // world draw
    glDeleteProgram (mgl_data->world_shader);
    // flat draw
    glDeleteProgram (mgl_data->ui_shader);
    glDeleteVertexArrays (1, &mgl_data->ui_vao);
    glDeleteBuffers (2, &mgl_data->ui_vbo);
    // mesh
    for (engine::mesh_core *i : mgl_data->managedMesh) {
      glDeleteVertexArrays (1, &i->vao);
      glDeleteBuffers (2, &i->vbo);
    }
    // texture
    for (opengles_texture *i : mgl_data->managedTexture) {
      glDeleteTextures (1, &i->id);
    }
    // reset null texture
    glDeleteTextures (1, &mgl_data->nullTextureId);

    eglDestroyContext (mgl_data->display, mgl_data->context);
    mgl_data->context = EGL_NO_CONTEXT;
  }
  if (EGLTermReq & 4) {
    eglTerminate (mgl_data->display);
    mgl_data->display = EGL_NO_DISPLAY;
  }
}
static inline void update_layout () {
  const float fixedWidth = mgl_data->wWidth;
  const float fixedHeight = mgl_data->wHeight;

  mgl_data->uiProj[0] = mgl_data->worldProj[0] = 2.f / fixedWidth;
  mgl_data->uiProj[5] = mgl_data->worldProj[5] = 2.f / fixedHeight;
  // ui safe insets update
  mgl_data->uiProj[12] = (2.0f * android_graphics::cur_safe_insets[0] / fixedWidth) - 1.0f;
  mgl_data->uiProj[13] = (2.0f * android_graphics::cur_safe_insets[3] / fixedHeight) - 1.0f;
  mgl_data->game_width = fixedWidth - android_graphics::cur_safe_insets[0] - android_graphics::cur_safe_insets[2];
  mgl_data->game_height = fixedHeight - android_graphics::cur_safe_insets[1] - android_graphics::cur_safe_insets[3];
  mgl_data->dirty_uiProj = true;
  mgl_data->dirty_worldProj = true;
}

// android
static void onWindowChange (ANativeWindow *w) {
  if (window)
    killEGL (TERM_EGL_SURFACE);
  window = w;
}
static void onWindowResize (unsigned char par) {
  mgl_data->resize_state |= par;
}
static bool preRender () {
  if (window == nullptr) return false;
  if (!mgl_data->display || !mgl_data->context || !mgl_data->surface) {
    while (!mgl_data->display) {
      // proof
      mgl_data->context = EGL_NO_CONTEXT;
      mgl_data->surface = EGL_NO_SURFACE;
      mgl_data->display = eglGetDisplay (EGL_DEFAULT_DISPLAY);
      eglInitialize (mgl_data->display, nullptr, nullptr);
      mgl_data->eConfig = nullptr;
    }
    while (!mgl_data->eConfig) {
      const EGLint configAttr[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_CONFORMANT, EGL_OPENGL_ES2_BIT, EGL_ALPHA_SIZE, 0, EGL_NONE};
      EGLint temp;
      eglChooseConfig (mgl_data->display, configAttr, nullptr, 0, &temp);
      EGLConfig *configs = (EGLConfig *)alloca (temp * sizeof (EGLConfig));
      eglChooseConfig (mgl_data->display, configAttr, configs, temp, &temp);
      mgl_data->eConfig = configs[0];
      for (size_t i = 0, j = temp, k = 0, l; i < j; i++) {
        EGLConfig &cfg = configs[i];
        eglGetConfigAttrib (mgl_data->display, cfg, EGL_BUFFER_SIZE, &temp);
        l = temp;
        eglGetConfigAttrib (mgl_data->display, cfg, EGL_DEPTH_SIZE, &temp);
        l += temp;
        eglGetConfigAttrib (mgl_data->display, cfg, EGL_STENCIL_SIZE, &temp);
        l += temp;
        if (l > k) {
          k = l;
          mgl_data->eConfig = cfg;
        }
      }
    }
    bool newCntx = false;
    while (!mgl_data->context) {
      const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
      mgl_data->context = eglCreateContext (mgl_data->display, mgl_data->eConfig, nullptr, ctxAttr);
      newCntx = true;
    }
    while (!mgl_data->surface)
      mgl_data->surface = eglCreateWindowSurface (mgl_data->display, mgl_data->eConfig, window, nullptr);

    eglMakeCurrent (mgl_data->display, mgl_data->surface, mgl_data->surface, mgl_data->context);
    eglQuerySurface (mgl_data->display, mgl_data->surface, EGL_WIDTH, &mgl_data->wWidth);
    eglQuerySurface (mgl_data->display, mgl_data->surface, EGL_HEIGHT, &mgl_data->wHeight);
    if (newCntx) {
      // made root for null texture test
      {
        glGenTextures (1, &mgl_data->nullTextureId);
        unsigned char data[4]{0xff, 0xff, 0xff, 0xff};
        glBindTexture (GL_TEXTURE_2D, mgl_data->nullTextureId);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)data);
        glBindTexture (GL_TEXTURE_2D, 0);
      }
      // validating gles resources
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
      GLuint vi, fi;
      // flat draw
      {
        mgl_data->ui_shader = glCreateProgram ();
        vi = glCreateShader (GL_VERTEX_SHADER);
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
        glAttachShader (mgl_data->ui_shader, vi);
        fi = glCreateShader (GL_FRAGMENT_SHADER);
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
        glAttachShader (mgl_data->ui_shader, fi);
        glLinkProgram (mgl_data->ui_shader);
        glDeleteShader (vi);
        glDeleteShader (fi);
        mgl_data->u_uiProj = glGetUniformLocation (mgl_data->ui_shader, "u_proj");
        mgl_data->u_uiTex = glGetUniformLocation (mgl_data->ui_shader, "u_tex");
        glGenVertexArrays (1, &mgl_data->ui_vao);
        glGenBuffers (2, &mgl_data->ui_vbo);
        glBindVertexArray (mgl_data->ui_vao);
        unsigned short indexs[MAX_UI_DRAW * 6];
        for (unsigned short i = 0, j = 0, k = 0; i < MAX_UI_DRAW; i++, j += 6) {
          indexs[j] = k++;
          indexs[j + 1] = indexs[j + 5] = k++;
          indexs[j + 2] = indexs[j + 4] = k++;
          indexs[j + 3] = k++;
        }
        // 0, 1, 2, 3, 2, 1
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, mgl_data->ui_ibo);
        glBufferData (GL_ELEMENT_ARRAY_BUFFER, MAX_UI_DRAW * 6 * sizeof (unsigned short), (void *)indexs, GL_STATIC_DRAW);
        glBindBuffer (GL_ARRAY_BUFFER, mgl_data->ui_vbo);
        glBufferData (GL_ARRAY_BUFFER, MAX_UI_DRAW * 4 * sizeof (engine::flat_vertex), NULL, GL_DYNAMIC_DRAW);
        glVertexAttribPointer (0, 2, GL_FLOAT, false, sizeof (engine::flat_vertex), (void *)offsetof (engine::flat_vertex, x));
        glVertexAttribPointer (1, 4, GL_UNSIGNED_BYTE, true, sizeof (engine::flat_vertex), (void *)offsetof (engine::flat_vertex, color));
        glVertexAttribPointer (2, 2, GL_FLOAT, false, sizeof (engine::flat_vertex), (void *)offsetof (engine::flat_vertex, u));
        glEnableVertexAttribArray (0);
        glEnableVertexAttribArray (1);
        glEnableVertexAttribArray (2);
      }
      // world draw
      {
        mgl_data->world_shader = glCreateProgram ();
        vi = glCreateShader (GL_VERTEX_SHADER);
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
        glAttachShader (mgl_data->world_shader, vi);
        fi = glCreateShader (GL_FRAGMENT_SHADER);
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
        glAttachShader (mgl_data->world_shader, fi);
        glLinkProgram (mgl_data->world_shader);
        glDeleteShader (vi);
        glDeleteShader (fi);
        mgl_data->u_worldProj = glGetUniformLocation (mgl_data->world_shader, "worldview_proj");
        mgl_data->u_worldTransProj = glGetUniformLocation (mgl_data->world_shader, "trans_proj");
      }
      // mesh
      for (engine::mesh_core *i : mgl_data->managedMesh) {
        glGenVertexArrays (1, &i->vao);
        glGenBuffers (2, &i->vbo);
        glBindVertexArray (i->vao);
        glBindBuffer (GL_ARRAY_BUFFER, i->vbo);
        glBufferData (GL_ARRAY_BUFFER, i->vertex_len * sizeof (engine::mesh_core::data), (void *)i->vertex, GL_STATIC_DRAW);
        glEnableVertexAttribArray (0);
        glVertexAttribPointer (0, 3, GL_FLOAT, false, sizeof (engine::mesh_core::data), (void *)offsetof (engine::mesh_core::data, pos));
        glEnableVertexAttribArray (1);
        glVertexAttribPointer (1, 4, GL_UNSIGNED_BYTE, true, sizeof (engine::mesh_core::data), (void *)offsetof (engine::mesh_core::data, color));
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, i->ibo);
        glBufferData (GL_ELEMENT_ARRAY_BUFFER, i->index_len * sizeof (unsigned short), (void *)i->index, GL_STATIC_DRAW);
      }
      glBindVertexArray (0);
      // texture
      for (opengles_texture *i : mgl_data->managedTexture) {
        glGenTextures (1, &i->id);
        glBindTexture (GL_TEXTURE_2D, i->id);
        glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, i->w, i->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)i->d);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      }
      glBindTexture (GL_TEXTURE_2D, 0);
    }
    glViewport (0, 0, mgl_data->wWidth, mgl_data->wHeight);
    update_layout ();
  } else if (mgl_data->resize_state) {
    if (mgl_data->resize_state & 2) {
      eglMakeCurrent (mgl_data->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
      eglMakeCurrent (mgl_data->display, mgl_data->surface, mgl_data->surface, mgl_data->context);
      eglQuerySurface (mgl_data->display, mgl_data->surface, EGL_WIDTH, &mgl_data->wWidth);
      eglQuerySurface (mgl_data->display, mgl_data->surface, EGL_HEIGHT, &mgl_data->wHeight);
    }
    update_layout ();
    mgl_data->resize_state = 0;
  }
  return true;
}
static void postRender (bool isDestroy) {
  unsigned int EGLTermReq = (isDestroy) ? TERM_EGL_DISPLAY : 0;
  if (!eglSwapBuffers (mgl_data->display, mgl_data->surface)) {
    switch (eglGetError ()) {
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
  killEGL (EGLTermReq);
}
// core
static float getWidth () { return mgl_data->game_width; }
static float getHeight () { return mgl_data->game_height; }
static void clear (const unsigned int &m) {
  GLuint c = 0;
  if (m & 1)
    c |= GL_COLOR_BUFFER_BIT;
  if (m & 2)
    c |= GL_DEPTH_BUFFER_BIT;
  if (m & 4)
    c |= GL_STENCIL_BUFFER_BIT;
  glClear (c);
}
static void clearcolor (const float &r, const float &g, const float &b, const float &a) {
  glClearColor (r, g, b, a);
}
static engine::texture_core *gen_texture (const int &tw, const int &th, unsigned char *data) {
  opengles_texture *t = new opengles_texture (0, tw, th, data);
  glGenTextures (1, &t->id);
  glBindTexture (GL_TEXTURE_2D, t->id);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)data);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture (GL_TEXTURE_2D, 0);
  mgl_data->managedTexture.insert (t);
  return t;
}
static void bind_texture (engine::texture_core *t) {
  glBindTexture (GL_TEXTURE_2D, t ? static_cast<opengles_texture *> (t)->id : 0);
}
static void set_texture_param (const int &param, const int &val) {
  glTexParameteri (GL_TEXTURE_2D, param, val);
}
static void delete_texture (engine::texture_core *t) {
  opengles_texture *i = static_cast<opengles_texture *> (t);
  auto it = mgl_data->managedTexture.find (i);
  if (it == mgl_data->managedTexture.end ()) return;
  mgl_data->managedTexture.erase (it);
  glDeleteTextures (1, &i->id);
  delete i;
}
static void flat_render (engine::texture_core *tex, engine::flat_vertex *v, const unsigned int len) {
  glDisable (GL_DEPTH_TEST);
  glUseProgram (mgl_data->ui_shader);
  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_2D, tex ? ((opengles_texture *)tex)->id : mgl_data->nullTextureId);
  glUniform1i (mgl_data->u_uiTex, 0);
  if (mgl_data->dirty_uiProj) {
    glUniformMatrix4fv (mgl_data->u_uiProj, 1, false, mgl_data->uiProj);
    mgl_data->dirty_uiProj = false;
  }
  glBindVertexArray (mgl_data->ui_vao);
  glBindBuffer (GL_ARRAY_BUFFER, mgl_data->ui_vbo);
  glBufferSubData (GL_ARRAY_BUFFER, 0, 4 * len * sizeof (engine::flat_vertex), (void *)v);
  glDrawElements (GL_TRIANGLES, 6 * len, GL_UNSIGNED_SHORT, NULL);
  glBindVertexArray (0);
  glBindTexture (GL_TEXTURE_2D, 0);
  glUseProgram (0);
}
static engine::mesh_core *gen_mesh (engine::mesh_core::data *v, unsigned int v_len, unsigned short *i, unsigned int i_len) {
  engine::mesh_core *r = new engine::mesh_core;
  r->vertex_len = v_len;
  r->vertex = new engine::mesh_core::data[v_len];
  memcpy (r->vertex, v, v_len * sizeof (engine::mesh_core::data));
  r->index_len = i_len;
  r->index = new unsigned short[i_len];
  memcpy (r->index, i, i_len * sizeof (unsigned short));
  glGenVertexArrays (1, &r->vao);
  glGenBuffers (2, &r->vbo);
  glBindVertexArray (r->vao);
  glBindBuffer (GL_ARRAY_BUFFER, r->vbo);
  glBufferData (GL_ARRAY_BUFFER, r->vertex_len * sizeof (engine::mesh_core::data), (void *)r->vertex, GL_STATIC_DRAW);
  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, false, sizeof (engine::mesh_core::data), (void *)offsetof (engine::mesh_core::data, pos));
  glEnableVertexAttribArray (1);
  glVertexAttribPointer (1, 4, GL_UNSIGNED_BYTE, true, sizeof (engine::mesh_core::data), (void *)offsetof (engine::mesh_core::data, color));
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, r->ibo);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, r->index_len * sizeof (unsigned short), (void *)r->index, GL_STATIC_DRAW);
  glBindVertexArray (0);
  mgl_data->managedMesh.insert (r);
  return r;
}
static void mesh_render (engine::mesh_core **meshes, const unsigned int &count) {
  glEnable (GL_DEPTH_TEST);
  glUseProgram (mgl_data->world_shader);
  if (mgl_data->dirty_worldProj) {
    glUniformMatrix4fv (mgl_data->u_worldProj, 1, false, mgl_data->worldProj);
    mgl_data->dirty_worldProj = false;
  }
  for (size_t i = 0; i < count; i++) {
    engine::mesh_core *m = *(meshes + i);
    glUniformMatrix4fv (mgl_data->u_worldTransProj, 1, false, m->trans);
    glBindVertexArray (m->vao);
    if (m->dirty_vertex) {
      glBindBuffer (GL_ARRAY_BUFFER, m->vbo);
      glBufferSubData (GL_ARRAY_BUFFER, 0, m->vertex_len * sizeof (engine::mesh_core::data), (void *)m->vertex);
      m->dirty_vertex = false;
    }
    if (m->dirty_index) {
      glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m->ibo);
      glBufferSubData (GL_ELEMENT_ARRAY_BUFFER, 0, m->index_len * sizeof (unsigned short), (void *)m->index);
      m->dirty_index = false;
    }
    glDrawElements (GL_TRIANGLES, m->index_len, GL_UNSIGNED_SHORT, NULL);
  }
  glBindVertexArray (0);
  glUseProgram (0);
}
static void delete_mesh (engine::mesh_core *m) {
  auto it = mgl_data->managedMesh.find (m);
  if (it == mgl_data->managedMesh.end ()) return;
  mgl_data->managedMesh.erase (it);
  glDeleteVertexArrays (1, &m->vao);
  glDeleteBuffers (2, &m->vbo);
  delete[] m->vertex;
  delete[] m->index;
  delete m;
}
static void to_flat_coordinate (float &x, float &y) {
  x -= android_graphics::cur_safe_insets[0];
  y = mgl_data->wHeight - y - android_graphics::cur_safe_insets[3];
}

void init () {
  mgl_data = new gl_data{};

  engine::graphics::getWidth = getWidth;
  engine::graphics::getHeight = getHeight;
  engine::graphics::clear = clear;
  engine::graphics::clearcolor = clearcolor;
  engine::graphics::gen_texture = gen_texture;
  engine::graphics::bind_texture = bind_texture;
  engine::graphics::set_texture_param = set_texture_param;
  engine::graphics::delete_texture = delete_texture;
  engine::graphics::flat_render = flat_render;
  engine::graphics::gen_mesh = gen_mesh;
  engine::graphics::mesh_render = mesh_render;
  engine::graphics::delete_mesh = delete_mesh;
  engine::graphics::to_flat_coordinate = to_flat_coordinate;

  android_graphics::onWindowChange = onWindowChange;
  android_graphics::onWindowResize = onWindowResize;
  android_graphics::preRender = preRender;
  android_graphics::postRender = postRender;
}

void term () {
  mgl_data->managedTexture.clear ();
  mgl_data->managedMesh.clear ();
  delete mgl_data;
  mgl_data = nullptr;

  // Set the function pointers to nullptr
  engine::graphics::getWidth = nullptr;
  engine::graphics::getHeight = nullptr;
  engine::graphics::clear = nullptr;
  engine::graphics::clearcolor = nullptr;
  engine::graphics::gen_texture = nullptr;
  engine::graphics::bind_texture = nullptr;
  engine::graphics::set_texture_param = nullptr;
  engine::graphics::delete_texture = nullptr;
  engine::graphics::flat_render = nullptr;
  engine::graphics::gen_mesh = nullptr;
  engine::graphics::mesh_render = nullptr;
  engine::graphics::delete_mesh = nullptr;
  engine::graphics::to_flat_coordinate = nullptr;

  android_graphics::onWindowChange = nullptr;
  android_graphics::onWindowResize = nullptr;
  android_graphics::preRender = nullptr;
  android_graphics::postRender = nullptr;
}
} // namespace graphics_opengles