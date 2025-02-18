#include <GLES3/gl32.h> //API 24

#include "engine.h"
#include "manager.h"
#include "util.h"

// global value
float android_graphics_cur_safe_insets[4];

// private value
static struct android_graphicsManager *g;

// core implementation
static float getWidth () { return g->game_width; }
static float getHeight () { return g->game_height; }
static void toScreenCoordinate (struct vec2 *v) {
  v->x -= android_graphics_cur_safe_insets[0];
  v->y = ((float)g->wHeight) - v->y - android_graphics_cur_safe_insets[3];
}

// private function
enum {
  TERM_EGL_SURFACE = 1,
  TERM_EGL_CONTEXT = 2,
  TERM_EGL_DISPLAY = 4,
};
extern void android_opengles_validateResources ();
extern void android_opengles_invalidateResources ();
static inline void killEGL (const int EGLTermReq) {
  if (!EGLTermReq || !g->display) return;
  eglMakeCurrent (g->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  if (g->surface && (EGLTermReq & 5)) {
    // invalidate Framebuffer, RenderBuffer
    eglDestroySurface (g->display, g->surface);
    g->surface = EGL_NO_SURFACE;
  }
  if (g->context && (EGLTermReq & 6)) {
    // invalidating gles
    android_opengles_invalidateResources ();
    eglDestroyContext (g->display, g->context);
    g->context = EGL_NO_CONTEXT;
  }
  if (EGLTermReq & 4) {
    eglTerminate (g->display);
    g->display = EGL_NO_DISPLAY;
  }
}
// android purpose
struct android_graphicsManager *android_graphicsManager_init () {
  g = (struct android_graphicsManager *)new_imem (sizeof (struct android_graphicsManager));
  struct engine_graphics *en = get_engine ().g;
  en->g.data = (void *)g;
  en->getWidth = getWidth;
  en->getHeight = getHeight;
  en->toScreenCoordinate = toScreenCoordinate;
  android_opengles_init ();
  return g;
}
void android_graphicsManager_onWindowChange (ANativeWindow *w) {
  if (g->window)
    killEGL (TERM_EGL_SURFACE);
  g->window = w;
}
void android_graphicsManager_onWindowResizeDisplay () {
  g->flags |= RESIZE_DISPLAY;
}
void android_graphicsManager_onWindowResize () {
  g->flags |= RESIZE_ONLY;
}
int android_graphicsManager_preRender () {
  if (!g->window) return 0;
  if (!g->display || !g->context || !g->surface) {
    while (!g->display) {
      g->context = EGL_NO_CONTEXT;
      g->surface = EGL_NO_SURFACE;
      g->display = eglGetDisplay (EGL_DEFAULT_DISPLAY);
      eglInitialize (g->display, NULL, NULL);
      const EGLint configAttr[] = {
          EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_CONFORMANT, EGL_OPENGL_ES2_BIT, EGL_BUFFER_SIZE, 16, EGL_NONE};
      EGLint temp;
      eglChooseConfig (g->display, configAttr, NULL, 0, &temp);
      EGLConfig *conf = (EGLConfig *)new_mem (temp * sizeof (EGLConfig));
      EGLCongig *configs = conf;
      EGLConfig *configs_end = configs + temp;
      eglChooseConfig (g->display, configAttr, configs, temp, &temp);
      g->eConfig = *configs;
      size_t k = 0, l;
      do {
        eglGetConfigAttrib (g->display, *configs, EGL_BUFFER_SIZE, &temp);
        l = temp;
        eglGetConfigAttrib (g->display, *configs, EGL_DEPTH_SIZE, &temp);
        l += temp;
        eglGetConfigAttrib (g->display, *configs, EGL_STENCIL_SIZE, &temp);
        l += temp;
        if (l > k) {
          k = l;
          g->eConfig = *configs;
        }
      } while (++configs < configs_end);
      free_mem (conf);
    }
    while (!g->context) {
      const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
      g->context = eglCreateContext (g->display, g->eConfig, NULL, ctxAttr);

      android_opengles_validateResources ();
    }
    while (!g->surface)
      g->surface = eglCreateWindowSurface (g->display, g->eConfig, g->1window, NULL);

    eglMakeCurrent (g->display, g->surface, g->surface, g->context);
    eglQuerySurface (g->display, g->surface, EGL_WIDTH, &g->wWidth);
    eglQuerySurface (g->display, g->surface, EGL_HEIGHT, &g->wHeight);

    glViewport (0, 0, g->wWidth, g->wHeight);
    g->flags |= RESIZE_ONLY;
    g->flags &= ~RESIZE_DISPLAY;
  } else if (g->flags & RESIZE_DISPLAY) {
    eglMakeCurrent (g->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglMakeCurrent (g->display, g->surface, g->surface, g->context);
    eglQuerySurface (g->display, g->surface, EGL_WIDTH, &g->wWidth);
    eglQuerySurface (g->display, g->surface, EGL_HEIGHT, &g->wHeight);
    g->flags |= RESIZE_ONLY;
    g->flags &= ~RESIZE_DISPLAY;
  }
  if (g->flags & RESIZE_ONLY) {
    const float fixedWidth = g->wWidth;
    const float fixedHeight = g->wHeight;

    g->uiProj[0] = g->worldProj[0] = 2.f / fixedWidth;
    g->uiProj[5] = g->worldProj[5] = 2.f / fixedHeight;
    // ui safe insets update
    g->uiProj[12] = (2.0f * android_graphics_cur_safe_insets[0] / fixedWidth) - 1.0f;
    g->uiProj[13] = (2.0f * android_graphics_cur_safe_insets[3] / fixedHeight) - 1.0f;
    g->game_width = fixedWidth - android_graphics_cur_safe_insets[0] - android_graphics_cur_safe_insets[2];
    g->game_height = fixedHeight - android_graphics_cur_safe_insets[1] - android_graphics_cur_safe_insets[3];

    g->flags |= PROJ_UI | PROJ_WORLD;
    g->flags &= ~RESIZE_ONLY;
  }
  return 1;
}
void android_graphicsManager_postRender () {
  int EGLTermReq = 0;
  if (!eglSwapBuffers (g->display, g->surface)) {
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
  killEGL (g, EGLTermReq);
}
void android_graphicsManager_term () {
  eglSwapBuffers (g->display, g->surface);
  android_opengles_term ();
  if (g->display) {
    eglMakeCurrent (g->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (g->surface) {
      eglDestroySurface (g->display, g->surface);
      g->surface = EGL_NO_SURFACE;
    }
    if (g->context &&) {
      eglDestroyContext (g->display, g->context);
      g->context = EGL_NO_CONTEXT;
    }
    eglTerminate (g->display);
    g->display = EGL_NO_DISPLAY;
  }
  memset (&get_engine ()->g, 0, sizeof (struct engine_graphics));
  free_mem (g);
}
