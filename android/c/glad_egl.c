#include "glad_egl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// EGL Core Functions
EGLBoolean (*eglChooseConfig)(EGLDisplay, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) = NULL;
EGLBoolean (*eglCopyBuffers)(EGLDisplay, EGLSurface, EGLNativePixmapType target) = NULL;
EGLContext (*eglCreateContext)(EGLDisplay, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) = NULL;
EGLSurface (*eglCreatePbufferSurface)(EGLDisplay, EGLConfig config, const EGLint *attrib_list) = NULL;
EGLSurface (*eglCreatePixmapSurface)(EGLDisplay, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list) = NULL;
EGLSurface (*eglCreateWindowSurface)(EGLDisplay, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list) = NULL;
EGLBoolean (*eglDestroyContext)(EGLDisplay, EGLContext) = NULL;
EGLBoolean (*eglDestroySurface)(EGLDisplay, EGLSurface) = NULL;
EGLBoolean (*eglGetConfigAttrib)(EGLDisplay, EGLConfig config, EGLint attribute, EGLint *value) = NULL;
EGLBoolean (*eglGetConfigs)(EGLDisplay, EGLConfig *configs, EGLint config_size, EGLint *num_config) = NULL;
EGLDisplay (*eglGetCurrentDisplay)(void) = NULL;
EGLSurface (*eglGetCurrentSurface)(EGLint readdraw) = NULL;
EGLDisplay (*eglGetDisplay)(EGLNativeDisplayType display_id) = NULL;
EGLint (*eglGetError)(void) = NULL;
EGLBoolean (*eglInitialize)(EGLDisplay, EGLint *major, EGLint *minor) = NULL;
EGLBoolean (*eglMakeCurrent)(EGLDisplay, EGLSurface draw, EGLSurface read, EGLContext) = NULL;
EGLBoolean (*eglQueryContext)(EGLDisplay, EGLContext, EGLint attribute, EGLint *value) = NULL;
const char *(*eglQueryString)(EGLDisplay, EGLint name) = NULL;
EGLBoolean (*eglQuerySurface)(EGLDisplay, EGLSurface, EGLint attribute, EGLint *value) = NULL;
EGLBoolean (*eglSwapBuffers)(EGLDisplay, EGLSurface) = NULL;
EGLBoolean (*eglTerminate)(EGLDisplay) = NULL;
EGLBoolean (*eglWaitGL)(void) = NULL;
EGLBoolean (*eglWaitNative)(EGLint engine) = NULL;

// EGL 1.1 Functions
EGLBoolean (*eglBindTexImage)(EGLDisplay, EGLSurface, EGLint buffer) = NULL;
EGLBoolean (*eglReleaseTexImage)(EGLDisplay, EGLSurface, EGLint buffer) = NULL;
EGLBoolean (*eglSurfaceAttrib)(EGLDisplay, EGLSurface, EGLint attribute, EGLint value) = NULL;
EGLBoolean (*eglSwapInterval)(EGLDisplay, EGLint interval) = NULL;

// EGL 1.2 Functions
EGLBoolean (*eglBindAPI)(EGLenum api) = NULL;
EGLenum (*eglQueryAPI)(void) = NULL;
EGLSurface (*eglCreatePbufferFromClientBuffer)(EGLDisplay, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list) = NULL;
EGLBoolean (*eglReleaseThread)(void) = NULL;
EGLBoolean (*eglWaitClient)(void) = NULL;

// EGL 1.4 Functions
EGLContext (*eglGetCurrentContext)(void) = NULL;

void gladLoadEGL(void *(*load)(const char *)) {

  // EGL Core Functions
  eglChooseConfig = (EGLBoolean(*)(EGLDisplay, const EGLint *, EGLConfig *, EGLint, EGLint *))load("eglChooseConfig");
  eglCopyBuffers = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLNativePixmapType))load("eglCopyBuffers");
  eglCreateContext = (EGLContext(*)(EGLDisplay, EGLConfig, EGLContext, const EGLint *))load("eglCreateContext");
  eglCreatePbufferSurface = (EGLSurface(*)(EGLDisplay, EGLConfig, const EGLint *))load("eglCreatePbufferSurface");
  eglCreatePixmapSurface = (EGLSurface(*)(EGLDisplay, EGLConfig, EGLNativePixmapType, const EGLint *))load("eglCreatePixmapSurface");
  eglCreateWindowSurface = (EGLSurface(*)(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint *))load("eglCreateWindowSurface");
  eglDestroyContext = (EGLBoolean(*)(EGLDisplay, EGLContext))load("eglDestroyContext");
  eglDestroySurface = (EGLBoolean(*)(EGLDisplay, EGLSurface))load("eglDestroySurface");
  eglGetConfigAttrib = (EGLBoolean(*)(EGLDisplay, EGLConfig, EGLint, EGLint *))load("eglGetConfigAttrib");
  eglGetConfigs = (EGLBoolean(*)(EGLDisplay, EGLConfig *, EGLint, EGLint *))load("eglGetConfigs");
  eglGetCurrentDisplay = (EGLDisplay(*)(void))load("eglGetCurrentDisplay");
  eglGetCurrentSurface = (EGLSurface(*)(EGLint))load("eglGetCurrentSurface");
  eglGetDisplay = (EGLDisplay(*)(EGLNativeDisplayType))load("eglGetDisplay");
  eglGetError = (EGLint(*)(void))load("eglGetError");
  eglInitialize = (EGLBoolean(*)(EGLDisplay, EGLint *, EGLint *))load("eglInitialize");
  eglMakeCurrent = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLSurface, EGLContext))load("eglMakeCurrent");
  eglQueryContext = (EGLBoolean(*)(EGLDisplay, EGLContext, EGLint, EGLint *))load("eglQueryContext");
  eglQueryString = (const char *(*)(EGLDisplay, EGLint))load("eglQueryString");
  eglQuerySurface = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLint, EGLint *))load("eglQuerySurface");
  eglSwapBuffers = (EGLBoolean(*)(EGLDisplay, EGLSurface))load("eglSwapBuffers");
  eglTerminate = (EGLBoolean(*)(EGLDisplay))load("eglTerminate");
  eglWaitGL = (EGLBoolean(*)(void))load("eglWaitGL");
  eglWaitNative = (EGLBoolean(*)(EGLint))load("eglWaitNative");

  // EGL 1.1 Functions
  eglBindTexImage = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLint))load("eglBindTexImage");
  eglReleaseTexImage = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLint))load("eglReleaseTexImage");
  eglSurfaceAttrib = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLint, EGLint))load("eglSurfaceAttrib");
  eglSwapInterval = (EGLBoolean(*)(EGLDisplay, EGLint))load("eglSwapInterval");

  // EGL 1.2 Functions
  eglBindAPI = (EGLBoolean(*)(EGLenum))load("eglBindAPI");
  eglQueryAPI = (EGLenum(*)(void))load("eglQueryAPI");
  eglCreatePbufferFromClientBuffer = (EGLSurface(*)(EGLDisplay, EGLenum, EGLClientBuffer, EGLConfig, const EGLint *))load("eglCreatePbufferFromClientBuffer");
  eglReleaseThread = (EGLBoolean(*)(void))load("eglReleaseThread");
  eglWaitClient = (EGLBoolean(*)(void))load("eglWaitClient");

  // EGL 1.4 Functions
  eglGetCurrentContext = (EGLContext(*)(void))load("eglGetCurrentContext");
}
