#include "engine.h"
#include "log.h"
#include "util.h"

#include <EGL/egl.h>
#include <android/native_window.h>
#include <string.h>

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

extern void android_opengles_init();
extern void android_opengles_validateResources();
extern void android_opengles_preRender();
extern void android_opengles_resizeInsets(float, float, float, float);
extern void android_opengles_resizeWindow(float, float);
extern void android_opengles_invalidateResources();
extern void android_opengles_term();
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
    if (!g->display) {
      g->context = EGL_NO_CONTEXT;
      g->surface = EGL_NO_SURFACE;
      g->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
      if (!g->display)
        return 0;

      EGLint temp, temp1;
      eglInitialize(g->display, &temp, &temp1);
      if (temp < 1 || temp1 < 3) // unsupported egl version lower than 1.3
        return 0;
      const EGLint configAttr[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_CONFORMANT, EGL_OPENGL_ES2_BIT, EGL_BUFFER_SIZE, 16, EGL_NONE};
      eglChooseConfig(g->display, configAttr, NULL, 0, &temp);
      if (temp <= 0)
        return 0;
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
      if (!g->context)
        return 0;
    }
    if (!g->surface) {
      g->surface = eglCreateWindowSurface(g->display, g->eConfig, g->window, NULL);
      if (!g->surface)
        return 0;
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
  LOGE("Reach preRender");
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
