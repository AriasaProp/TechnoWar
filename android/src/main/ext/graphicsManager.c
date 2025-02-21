#include <EGL/egl.h>

#include "engine.h"
#include "manager.h"
#include "util.h"

// private value
static struct android_graphicsManager {
  ANativeWindow *window;
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  EGLConfig eConfig;
  int flags;
} g = {0};

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

extern void android_opengles_init ();
extern void android_opengles_validateResources ();
extern void android_opengles_resizeInsets (float, float, float, float);
extern void android_opengles_resizeWindow (float, float);
extern void android_opengles_invalidateResources ();
extern void android_opengles_term ();
static inline void killEGL (const int EGLTermReq) {
  if (!EGLTermReq || !g.display) return;
  eglMakeCurrent (g.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  if (g.surface && (EGLTermReq & 5)) {
    // invalidate Framebuffer, RenderBuffer
    eglDestroySurface (g.display, g.surface);
    g.surface = EGL_NO_SURFACE;
  }
  if (g.context && (EGLTermReq & 6)) {
    // invalidating gles
    android_opengles_invalidateResources ();
    eglDestroyContext (g.display, g.context);
    g.context = EGL_NO_CONTEXT;
  }
  if (EGLTermReq & 4) {
    eglTerminate (g.display);
    g.display = EGL_NO_DISPLAY;
  }
}
// android purpose
void android_graphicsManager_init () {
  android_opengles_init ();
}
void android_graphicsManager_onWindowCreate (ANativeWindow *w) {
  g.window = w;
}
void android_graphicsManager_onWindowDestroy () {
  killEGL (TERM_EGL_SURFACE);
  g.window = NULL;
}
void android_graphicsManager_onWindowResizeDisplay () {
  g.flags |= RESIZE_DISPLAY;
}
void android_graphicsManager_onWindowResize () {
  g.flags |= RESIZE_ONLY;
}
void android_graphicsManager_resizeInsets (float x, float y, float z, float w) {
  android_opengles_resizeInsets (x, y, z, w);
}
int android_graphicsManager_preRender () {
  if (!g.window) return 0;
  if (!g.display || !g.context || !g.surface) {
    while (!g.display) {
      g.context = EGL_NO_CONTEXT;
      g.surface = EGL_NO_SURFACE;
      g.display = eglGetDisplay (EGL_DEFAULT_DISPLAY);

      EGLint temp, temp1;
      eglInitialize (g.display, &temp, &temp1);
      if (temp < 1 || temp1 < 3) // unsupported egl version lower than 1.3
        return 0;
      const EGLint configAttr[] = {
          EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_CONFORMANT, EGL_OPENGL_ES2_BIT, EGL_BUFFER_SIZE, 16, EGL_NONE};
      eglChooseConfig (g.display, configAttr, NULL, 0, &temp);
      EGLConfig *conf = (EGLConfig *)new_mem (temp * sizeof (EGLConfig));
      EGLConfig *configs = conf;
      EGLConfig *configs_end = configs + temp;
      eglChooseConfig (g.display, configAttr, configs, temp, &temp);
      g.eConfig = *configs;
      size_t k = 0, l;
      do {
        eglGetConfigAttrib (g.display, *configs, EGL_BUFFER_SIZE, &temp);
        l = temp;
        eglGetConfigAttrib (g.display, *configs, EGL_DEPTH_SIZE, &temp);
        l += temp;
        eglGetConfigAttrib (g.display, *configs, EGL_STENCIL_SIZE, &temp);
        l += temp;
        if (l > k) {
          k = l;
          g.eConfig = *configs;
        }
      } while (++configs < configs_end);
      free_mem (conf);
    }
    while (!g.context) {
      const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
      g.context = eglCreateContext (g.display, g.eConfig, NULL, ctxAttr);

      android_opengles_validateResources ();
    }
    while (!g.surface)
      g.surface = eglCreateWindowSurface (g.display, g.eConfig, g.window, NULL);

    eglMakeCurrent (g.display, g.surface, g.surface, g.context);
    g.flags |= RESIZE_ONLY;
    g.flags &= ~RESIZE_DISPLAY;
  } else if (g.flags & RESIZE_DISPLAY) {
    eglMakeCurrent (g.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglMakeCurrent (g.display, g.surface, g.surface, g.context);
    g.flags |= RESIZE_ONLY;
    g.flags &= ~RESIZE_DISPLAY;
  }
  if (g.flags & RESIZE_ONLY) {
    EGLint w, h;
    eglQuerySurface (g.display, g.surface, EGL_WIDTH, &w);
    eglQuerySurface (g.display, g.surface, EGL_HEIGHT, &h);
    android_opengles_resizeWindow ((float)w, (float)h);
    g.flags &= ~RESIZE_ONLY;
  }
  return 1;
}
void android_graphicsManager_postRender () {
  int EGLTermReq = 0;
  if (!eglSwapBuffers (g.display, g.surface)) {
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
void android_graphicsManager_term () {
  eglSwapBuffers (g.display, g.surface);
  android_opengles_term ();
  if (g.display) {
    eglMakeCurrent (g.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (g.surface) {
      eglDestroySurface (g.display, g.surface);
      g.surface = EGL_NO_SURFACE;
    }
    if (g.context) {
      eglDestroyContext (g.display, g.context);
      g.context = EGL_NO_CONTEXT;
    }
    eglTerminate (g.display);
    g.display = EGL_NO_DISPLAY;
  }
  memset (&get_engine ()->g, 0, sizeof (struct engine_graphics));
  memset (&g, 0, sizeof (struct android_graphicsManager));
}
