#include <cstddef>
#include <unordered_set>


#include "android_engine.hpp"
#include "engine.hpp"
#include "utils/value.hpp"
#include "utils/math.hpp"

#define MAX_MANAGED_SOURCE 512

#define PROJ_UI 1
#define PROJ_WORLD 2
#define RESIZE_DISPLAY 4
#define RESIZE_ONLY 8


struct opengles_texture{
  GLuint id;
  void *data;
};

struct gl_data {
	int flags;
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
  engine_texture_core managedTexture[MAX_MANAGED_SOURCE];
  engine_mesh_core managedMesh[MAX_MANAGED_SOURCE];
} *mgl_data = nullptr;


// core
static float getWidth () { return mgl_data->game_width; }
static float getHeight () { return mgl_data->game_height; }
static void clear (const int &m) {
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
static engine_texture_core *gen_texture (const int &tw, const int &th, unsigned char *data) {
  opengles_texture *t = (opengles_texture *)allocate_memory(sizeof(opengles_texture));
  engine_texture_core *r= (engine_texture_core *)allocate_memory(sizeof(engine_texture_core));
  r->width = tw;
  r->height = th;
  t->data = (void*)data;
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
  r->id = t;
  return r;
}
static void bind_texture (engine_texture_core *t) {
  glBindTexture (GL_TEXTURE_2D, (t != NULL) * ((opengles_texture *)t->id)->id);
}
static void set_texture_param (const int &param, const int &val) {
  glTexParameteri (GL_TEXTURE_2D, param, val);
}
static void delete_texture (engine_texture_core *t) {
  opengles_texture *i = (opengles_texture *)t->id;
  auto it = mgl_data->managedTexture.find (i);
  if (it == mgl_data->managedTexture.end ()) return;
  mgl_data->managedTexture.erase (it);
  glDeleteTextures (1, &i->id);
  delete i;
}
static void flat_render (engine_texture_core *tex, engine_flat_vertex *v, const unsigned int len) {
  glDisable (GL_DEPTH_TEST);
  glUseProgram (mgl_data->ui_shader);
  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_2D, tex ? ((opengles_texture *)tex)->id : mgl_data->nullTextureId);
  glUniform1i (mgl_data->u_uiTex, 0);
  if (mgl_data->flags & PROJ_UI) {
    glUniformMatrix4fv (mgl_data->u_uiProj, 1, false, mgl_data->uiProj);
    mgl_data->flags &= ~PROJ_UI;
  }
  glBindVertexArray (mgl_data->ui_vao);
  glBindBuffer (GL_ARRAY_BUFFER, mgl_data->ui_vbo);
  glBufferSubData (GL_ARRAY_BUFFER, 0, 4 * len * sizeof (engine_flat_vertex), (void *)v);
  glDrawElements (GL_TRIANGLES, 6 * len, GL_UNSIGNED_SHORT, NULL);
  glBindVertexArray (0);
  glBindTexture (GL_TEXTURE_2D, 0);
  glUseProgram (0);
}
static engine_mesh_core *gen_mesh (engine_mesh_core_data *v, unsigned int v_len, unsigned short *i, unsigned int i_len) {
  engine_mesh_core *r = (engine_mesh_core*)allocate_memory(sizeof(engine_mesh_core));
  matrix4_idt(r->trans);
  r->vertex_len = v_len;
  r->vertex = new engine_mesh_core_data[v_len];
  memcpy (r->vertex, v, v_len * sizeof (engine_mesh_core_data));
  r->index_len = i_len;
  r->index = new unsigned short[i_len];
  memcpy (r->index, i, i_len * sizeof (unsigned short));
  glGenVertexArrays (1, &r->vao);
  glGenBuffers (2, &r->vbo);
  glBindVertexArray (r->vao);
  glBindBuffer (GL_ARRAY_BUFFER, r->vbo);
  glBufferData (GL_ARRAY_BUFFER, r->vertex_len * sizeof (engine_mesh_core_data), (void *)r->vertex, GL_STATIC_DRAW);
  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, false, sizeof (engine_mesh_core_data), (void *)offsetof (engine_mesh_core_data, pos));
  glEnableVertexAttribArray (1);
  glVertexAttribPointer (1, 4, GL_UNSIGNED_BYTE, true, sizeof (engine_mesh_core_data), (void *)offsetof (engine_mesh_core_data, color));
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, r->ibo);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, r->index_len * sizeof (unsigned short), (void *)r->index, GL_STATIC_DRAW);
  glBindVertexArray (0);
  mgl_data->managedMesh.insert (r);
  return r;
}
static void mesh_render (engine_mesh_core **meshes, const unsigned int &count) {
  glEnable (GL_DEPTH_TEST);
  glUseProgram (mgl_data->world_shader);
  if (mgl_data->flags & PROJ_WORLD) {
    glUniformMatrix4fv (mgl_data->u_worldProj, 1, false, mgl_data->worldProj);
    mgl_data->flags &= ~PROJ_WORLD;
  }
  for (size_t i = 0; i < count; i++) {
    engine_mesh_core *m = *(meshes + i);
    glUniformMatrix4fv (mgl_data->u_worldTransProj, 1, false, m->trans);
    glBindVertexArray (m->vao);
    if (m->dirty_vertex) {
      glBindBuffer (GL_ARRAY_BUFFER, m->vbo);
      glBufferSubData (GL_ARRAY_BUFFER, 0, m->vertex_len * sizeof (engine_mesh_core_data), (void *)m->vertex);
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
static void delete_mesh (engine_mesh_core *m) {
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
  x -= android_graphics_cur_safe_insets[0];
  y = mgl_data->wHeight - y - android_graphics_cur_safe_insets[3];
}

void init () {
  mgl_data = new gl_data{};

  engine_graphics_getWidth = getWidth;
  engine_graphics_getHeight = getHeight;
  engine_graphics_clear = clear;
  engine_graphics_clearcolor = clearcolor;
  engine_graphics_gen_texture = gen_texture;
  engine_graphics_bind_texture = bind_texture;
  engine_graphics_set_texture_param = set_texture_param;
  engine_graphics_delete_texture = delete_texture;
  engine_graphics_flat_render = flat_render;
  engine_graphics_gen_mesh = gen_mesh;
  engine_graphics_mesh_render = mesh_render;
  engine_graphics_delete_mesh = delete_mesh;
  engine_graphics_to_flat_coordinate = to_flat_coordinate;

  android_graphics_onWindowChange = onWindowChange;
  android_graphics_onWindowResizeDisplay = onWindowResizeDisplay;
  android_graphics_onWindowResize = onWindowResize;
  android_graphics_preRender = preRender;
  android_graphics_postRender = postRender;
}

void term () {
  for (opengles_texture *i : mgl_data->managedTexture) {
    delete i;
  }
  mgl_data->managedTexture.clear ();
  for (engine_mesh_core *i : mgl_data->managedMesh) {
    delete i;
  }
  mgl_data->managedMesh = 0:
  free_memory(mgl_data);
  mgl_data = nullptr;

  // Set the function pointers to nullptr
  engine_graphics_getWidth = nullptr;
  engine_graphics_getHeight = nullptr;
  engine_graphics_clear = nullptr;
  engine_graphics_clearcolor = nullptr;
  engine_graphics_gen_texture = nullptr;
  engine_graphics_bind_texture = nullptr;
  engine_graphics_set_texture_param = nullptr;
  engine_graphics_delete_texture = nullptr;
  engine_graphics_flat_render = nullptr;
  engine_graphics_gen_mesh = nullptr;
  engine_graphics_mesh_render = nullptr;
  engine_graphics_delete_mesh = nullptr;
  engine_graphics_to_flat_coordinate = nullptr;

  android_graphics_onWindowChange = nullptr;
  android_graphics_onWindowResizeDisplay = nullptr;
  android_graphics_onWindowResize = nullptr;
  android_graphics_preRender = nullptr;
  android_graphics_postRender = nullptr;
}