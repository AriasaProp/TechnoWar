#include <android/native_window.h>
#include <dlfcn.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "engine.h"
#include "log.h"
#include "manager.h"

typedef int32_t khronos_int32_t;
typedef uint32_t khronos_uint32_t;
typedef int64_t khronos_int64_t;
typedef uint64_t khronos_uint64_t;
#define KHRONOS_SUPPORT_INT64 1
#define KHRONOS_SUPPORT_FLOAT 1

#if defined(__SIZEOF_LONG__) && defined(__SIZEOF_POINTER__)
#if __SIZEOF_POINTER__ > __SIZEOF_LONG__
#define KHRONOS_USE_INTPTR_T
#endif
#endif

typedef signed char khronos_int8_t;
typedef unsigned char khronos_uint8_t;
typedef signed short int khronos_int16_t;
typedef unsigned short int khronos_uint16_t;

/*
 * Types that differ between LLP64 and LP64 architectures - in LLP64,
 * pointers are 64 bits, but 'long' is still 32 bits. Win64 appears
 * to be the only LLP64 architecture in current use.
 */
#ifdef KHRONOS_USE_INTPTR_T
typedef intptr_t khronos_intptr_t;
typedef uintptr_t khronos_uintptr_t;
#else
typedef signed long int khronos_intptr_t;
typedef unsigned long int khronos_uintptr_t;
#endif

typedef signed long int khronos_ssize_t;
typedef unsigned long int khronos_usize_t;

#if KHRONOS_SUPPORT_FLOAT
typedef float khronos_float_t;
#endif

#if KHRONOS_SUPPORT_INT64
typedef khronos_uint64_t khronos_utime_nanoseconds_t;
typedef khronos_int64_t khronos_stime_nanoseconds_t;
#endif

typedef enum {
  KHRONOS_FALSE = 0,
  KHRONOS_TRUE = 1,
  KHRONOS_BOOLEAN_ENUM_FORCE_SIZE = 0x7FFFFFFF
} khronos_boolean_enum_t;

// IMPLEMENTS EGL
struct egl_native_pixmap_t;

typedef void *EGLNativeDisplayType;
typedef struct egl_native_pixmap_t *EGLNativePixmapType;
typedef ANativeWindow *EGLNativeWindowType;

/* EGL 1.2 types, renamed for consistency in EGL 1.3 */
typedef EGLNativeDisplayType NativeDisplayType;
typedef EGLNativePixmapType NativePixmapType;
typedef EGLNativeWindowType NativeWindowType;

typedef khronos_int32_t EGLint;

struct AHardwareBuffer;
struct wl_buffer;
struct wl_display;
struct wl_resource;
typedef unsigned int EGLBoolean;
typedef unsigned int EGLenum;
typedef intptr_t EGLAttribKHR;
typedef intptr_t EGLAttrib;
typedef void *EGLClientBuffer;
typedef void *EGLConfig;
typedef void *EGLContext;
typedef void *EGLDeviceEXT;
typedef void *EGLDisplay;
typedef void *EGLImage;
typedef void *EGLImageKHR;
typedef void *EGLLabelKHR;
typedef void *EGLObjectKHR;
typedef void *EGLOutputLayerEXT;
typedef void *EGLOutputPortEXT;
typedef void *EGLStreamKHR;
typedef void *EGLSurface;
typedef void *EGLSync;
typedef void *EGLSyncKHR;
typedef void *EGLSyncNV;
typedef void (*__eglMustCastToProperFunctionPointerType)(void);
typedef khronos_utime_nanoseconds_t EGLTimeKHR;
typedef khronos_utime_nanoseconds_t EGLTime;
typedef khronos_utime_nanoseconds_t EGLTimeNV;
typedef khronos_utime_nanoseconds_t EGLuint64NV;
typedef khronos_uint64_t EGLuint64KHR;
typedef khronos_stime_nanoseconds_t EGLnsecsANDROID;
typedef int EGLNativeFileDescriptorKHR;
#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1060)
#if defined(__khrplatform_h_)
typedef khronos_ssize_t EGLsizeiANDROID;
#else
typedef long EGLsizeiANDROID;
#endif
#else
#if defined(__khrplatform_h_)
typedef khronos_ssize_t EGLsizeiANDROID;
#else
typedef ptrdiff_t EGLsizeiANDROID;
#endif
#endif
typedef void (*EGLSetBlobFuncANDROID)(const void *key, EGLsizeiANDROID keySize, const void *value, EGLsizeiANDROID valueSize);
typedef EGLsizeiANDROID (*EGLGetBlobFuncANDROID)(const void *key, EGLsizeiANDROID keySize, void *value, EGLsizeiANDROID valueSize);
struct EGLClientPixmapHI {
  void *pData;
  EGLint iWidth;
  EGLint iHeight;
  EGLint iStride;
};
#define EGL_ALPHA_SIZE                  0x3021
#define EGL_BAD_ACCESS                  0x3002
#define EGL_BAD_ALLOC                   0x3003
#define EGL_BAD_ATTRIBUTE               0x3004
#define EGL_BAD_CONFIG                  0x3005
#define EGL_BAD_CONTEXT                 0x3006
#define EGL_BAD_CURRENT_SURFACE         0x3007
#define EGL_BAD_DISPLAY                 0x3008
#define EGL_BAD_MATCH                   0x3009
#define EGL_BAD_NATIVE_PIXMAP           0x300A
#define EGL_BAD_NATIVE_WINDOW           0x300B
#define EGL_BAD_PARAMETER               0x300C
#define EGL_BAD_SURFACE                 0x300D
#define EGL_BLUE_SIZE                   0x3022
#define EGL_BUFFER_SIZE                 0x3020
#define EGL_CONFIG_CAVEAT               0x3027
#define EGL_CONFIG_ID                   0x3028
#define EGL_CORE_NATIVE_ENGINE          0x305B
#define EGL_DEPTH_SIZE                  0x3025
#define EGL_DONT_CARE                   ((EGLint) - 1)
#define EGL_DRAW                        0x3059
#define EGL_EXTENSIONS                  0x3055
#define EGL_FALSE                       0
#define EGL_GREEN_SIZE                  0x3023
#define EGL_HEIGHT                      0x3056
#define EGL_LARGEST_PBUFFER             0x3058
#define EGL_LEVEL                       0x3029
#define EGL_MAX_PBUFFER_HEIGHT          0x302A
#define EGL_MAX_PBUFFER_PIXELS          0x302B
#define EGL_MAX_PBUFFER_WIDTH           0x302C
#define EGL_NATIVE_RENDERABLE           0x302D
#define EGL_NATIVE_VISUAL_ID            0x302E
#define EGL_NATIVE_VISUAL_TYPE          0x302F
#define EGL_NONE                        0x3038
#define EGL_NON_CONFORMANT_CONFIG       0x3051
#define EGL_NOT_INITIALIZED             0x3001
#define EGL_NO_CONTEXT                  ((EGLContext)0)
#define EGL_NO_DISPLAY                  ((EGLDisplay)0)
#define EGL_NO_SURFACE                  ((EGLSurface)0)
#define EGL_PBUFFER_BIT                 0x0001
#define EGL_PIXMAP_BIT                  0x0002
#define EGL_READ                        0x305A
#define EGL_RED_SIZE                    0x3024
#define EGL_SAMPLES                     0x3031
#define EGL_SAMPLE_BUFFERS              0x3032
#define EGL_SLOW_CONFIG                 0x3050
#define EGL_STENCIL_SIZE                0x3026
#define EGL_SUCCESS                     0x3000
#define EGL_SURFACE_TYPE                0x3033
#define EGL_TRANSPARENT_BLUE_VALUE      0x3035
#define EGL_TRANSPARENT_GREEN_VALUE     0x3036
#define EGL_TRANSPARENT_RED_VALUE       0x3037
#define EGL_TRANSPARENT_RGB             0x3052
#define EGL_TRANSPARENT_TYPE            0x3034
#define EGL_TRUE                        1
#define EGL_VENDOR                      0x3053
#define EGL_VERSION                     0x3054
#define EGL_WIDTH                       0x3057
#define EGL_WINDOW_BIT                  0x0004
#define EGL_BACK_BUFFER                 0x3084
#define EGL_BIND_TO_TEXTURE_RGB         0x3039
#define EGL_BIND_TO_TEXTURE_RGBA        0x303A
#define EGL_CONTEXT_LOST                0x300E
#define EGL_MIN_SWAP_INTERVAL           0x303B
#define EGL_MAX_SWAP_INTERVAL           0x303C
#define EGL_MIPMAP_TEXTURE              0x3082
#define EGL_MIPMAP_LEVEL                0x3083
#define EGL_NO_TEXTURE                  0x305C
#define EGL_TEXTURE_2D                  0x305F
#define EGL_TEXTURE_FORMAT              0x3080
#define EGL_TEXTURE_RGB                 0x305D
#define EGL_TEXTURE_RGBA                0x305E
#define EGL_TEXTURE_TARGET              0x3081
#define EGL_ALPHA_FORMAT                0x3088
#define EGL_ALPHA_FORMAT_NONPRE         0x308B
#define EGL_ALPHA_FORMAT_PRE            0x308C
#define EGL_ALPHA_MASK_SIZE             0x303E
#define EGL_BUFFER_PRESERVED            0x3094
#define EGL_BUFFER_DESTROYED            0x3095
#define EGL_CLIENT_APIS                 0x308D
#define EGL_COLORSPACE                  0x3087
#define EGL_COLORSPACE_sRGB             0x3089
#define EGL_COLORSPACE_LINEAR           0x308A
#define EGL_COLOR_BUFFER_TYPE           0x303F
#define EGL_CONTEXT_CLIENT_TYPE         0x3097
#define EGL_DISPLAY_SCALING             10000
#define EGL_HORIZONTAL_RESOLUTION       0x3090
#define EGL_LUMINANCE_BUFFER            0x308F
#define EGL_LUMINANCE_SIZE              0x303D
#define EGL_OPENGL_ES_BIT               0x0001
#define EGL_OPENVG_BIT                  0x0002
#define EGL_OPENGL_ES_API               0x30A0
#define EGL_OPENVG_API                  0x30A1
#define EGL_OPENVG_IMAGE                0x3096
#define EGL_PIXEL_ASPECT_RATIO          0x3092
#define EGL_RENDERABLE_TYPE             0x3040
#define EGL_RENDER_BUFFER               0x3086
#define EGL_RGB_BUFFER                  0x308E
#define EGL_SINGLE_BUFFER               0x3085
#define EGL_SWAP_BEHAVIOR               0x3093
#define EGL_UNKNOWN                     ((EGLint) - 1)
#define EGL_VERTICAL_RESOLUTION         0x3091
#define EGL_CONFORMANT                  0x3042
#define EGL_CONTEXT_CLIENT_VERSION      0x3098
#define EGL_MATCH_NATIVE_PIXMAP         0x3041
#define EGL_OPENGL_ES2_BIT              0x0004
#define EGL_VG_ALPHA_FORMAT             0x3088
#define EGL_VG_ALPHA_FORMAT_NONPRE      0x308B
#define EGL_VG_ALPHA_FORMAT_PRE         0x308C
#define EGL_VG_ALPHA_FORMAT_PRE_BIT     0x0040
#define EGL_VG_COLORSPACE               0x3087
#define EGL_VG_COLORSPACE_sRGB          0x3089
#define EGL_VG_COLORSPACE_LINEAR        0x308A
#define EGL_VG_COLORSPACE_LINEAR_BIT    0x0020
#define EGL_DEFAULT_DISPLAY             ((EGLNativeDisplayType)0)
#define EGL_MULTISAMPLE_RESOLVE_BOX_BIT 0x0200
#define EGL_MULTISAMPLE_RESOLVE         0x3099
#define EGL_MULTISAMPLE_RESOLVE_DEFAULT 0x309A
#define EGL_MULTISAMPLE_RESOLVE_BOX     0x309B
#define EGL_OPENGL_API                  0x30A2
#define EGL_OPENGL_BIT                  0x0008
#define EGL_SWAP_BEHAVIOR_PRESERVED_BIT 0x0400

// EGL Core Functions
static EGLBoolean (*eglChooseConfig)(EGLDisplay, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) = NULL;
static EGLBoolean (*eglCopyBuffers)(EGLDisplay, EGLSurface, EGLNativePixmapType target) = NULL;
static EGLContext (*eglCreateContext)(EGLDisplay, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) = NULL;
static EGLSurface (*eglCreatePbufferSurface)(EGLDisplay, EGLConfig config, const EGLint *attrib_list) = NULL;
static EGLSurface (*eglCreatePixmapSurface)(EGLDisplay, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list) = NULL;
static EGLSurface (*eglCreateWindowSurface)(EGLDisplay, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list) = NULL;
static EGLBoolean (*eglDestroyContext)(EGLDisplay, EGLContext) = NULL;
static EGLBoolean (*eglDestroySurface)(EGLDisplay, EGLSurface) = NULL;
static EGLBoolean (*eglGetConfigAttrib)(EGLDisplay, EGLConfig config, EGLint attribute, EGLint *value) = NULL;
static EGLBoolean (*eglGetConfigs)(EGLDisplay, EGLConfig *configs, EGLint config_size, EGLint *num_config) = NULL;
static EGLDisplay (*eglGetCurrentDisplay)(void) = NULL;
static EGLSurface (*eglGetCurrentSurface)(EGLint readdraw) = NULL;
static EGLDisplay (*eglGetDisplay)(EGLNativeDisplayType display_id) = NULL;
static EGLint (*eglGetError)(void) = NULL;
static EGLBoolean (*eglInitialize)(EGLDisplay, EGLint *major, EGLint *minor) = NULL;
static EGLBoolean (*eglMakeCurrent)(EGLDisplay, EGLSurface draw, EGLSurface read, EGLContext) = NULL;
static EGLBoolean (*eglQueryContext)(EGLDisplay, EGLContext, EGLint attribute, EGLint *value) = NULL;
static const char *(*eglQueryString)(EGLDisplay, EGLint name) = NULL;
static EGLBoolean (*eglQuerySurface)(EGLDisplay, EGLSurface, EGLint attribute, EGLint *value) = NULL;
static EGLBoolean (*eglSwapBuffers)(EGLDisplay, EGLSurface) = NULL;
static EGLBoolean (*eglTerminate)(EGLDisplay) = NULL;
static EGLBoolean (*eglWaitGL)(void) = NULL;
static EGLBoolean (*eglWaitNative)(EGLint engine) = NULL;
// EGL 1.1 Functions
static EGLBoolean (*eglBindTexImage)(EGLDisplay, EGLSurface, EGLint buffer) = NULL;
static EGLBoolean (*eglReleaseTexImage)(EGLDisplay, EGLSurface, EGLint buffer) = NULL;
static EGLBoolean (*eglSurfaceAttrib)(EGLDisplay, EGLSurface, EGLint attribute, EGLint value) = NULL;
static EGLBoolean (*eglSwapInterval)(EGLDisplay, EGLint interval) = NULL;
// EGL 1.2 Functions
static EGLBoolean (*eglBindAPI)(EGLenum api) = NULL;
static EGLenum (*eglQueryAPI)(void) = NULL;
static EGLSurface (*eglCreatePbufferFromClientBuffer)(EGLDisplay, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list) = NULL;
static EGLBoolean (*eglReleaseThread)(void) = NULL;
static EGLBoolean (*eglWaitClient)(void) = NULL;

static void *loadEGL(void) {
  void *v = dlopen("libEGL.so", RTLD_NOW | RTLD_LOCAL);
  if (!v)
    goto load_egl_err_e;

  // EGL Core Functions
  eglChooseConfig = (EGLBoolean(*)(EGLDisplay, const EGLint *, EGLConfig *, EGLint, EGLint *))dlsym(v, "eglChooseConfig");
  eglCopyBuffers = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLNativePixmapType))dlsym(v, "eglCopyBuffers");
  eglCreateContext = (EGLContext(*)(EGLDisplay, EGLConfig, EGLContext, const EGLint *))dlsym(v, "eglCreateContext");
  eglCreatePbufferSurface = (EGLSurface(*)(EGLDisplay, EGLConfig, const EGLint *))dlsym(v, "eglCreatePbufferSurface");
  eglCreatePixmapSurface = (EGLSurface(*)(EGLDisplay, EGLConfig, EGLNativePixmapType, const EGLint *))dlsym(v, "eglCreatePixmapSurface");
  eglCreateWindowSurface = (EGLSurface(*)(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint *))dlsym(v, "eglCreateWindowSurface");
  eglDestroyContext = (EGLBoolean(*)(EGLDisplay, EGLContext))dlsym(v, "eglDestroyContext");
  eglDestroySurface = (EGLBoolean(*)(EGLDisplay, EGLSurface))dlsym(v, "eglDestroySurface");
  eglGetConfigAttrib = (EGLBoolean(*)(EGLDisplay, EGLConfig, EGLint, EGLint *))dlsym(v, "eglGetConfigAttrib");
  eglGetConfigs = (EGLBoolean(*)(EGLDisplay, EGLConfig *, EGLint, EGLint *))dlsym(v, "eglGetConfigs");
  eglGetCurrentDisplay = (EGLDisplay(*)(void))dlsym(v, "eglGetCurrentDisplay");
  eglGetCurrentSurface = (EGLSurface(*)(EGLint))dlsym(v, "eglGetCurrentSurface");
  eglGetDisplay = (EGLDisplay(*)(EGLNativeDisplayType))dlsym(v, "eglGetDisplay");
  eglGetError = (EGLint(*)(void))dlsym(v, "eglGetError");
  eglInitialize = (EGLBoolean(*)(EGLDisplay, EGLint *, EGLint *))dlsym(v, "eglInitialize");
  eglMakeCurrent = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLSurface, EGLContext))dlsym(v, "eglMakeCurrent");
  eglQueryContext = (EGLBoolean(*)(EGLDisplay, EGLContext, EGLint, EGLint *))dlsym(v, "eglQueryContext");
  eglQueryString = (const char *(*)(EGLDisplay, EGLint))dlsym(v, "eglQueryString");
  eglQuerySurface = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLint, EGLint *))dlsym(v, "eglQuerySurface");
  eglSwapBuffers = (EGLBoolean(*)(EGLDisplay, EGLSurface))dlsym(v, "eglSwapBuffers");
  eglTerminate = (EGLBoolean(*)(EGLDisplay))dlsym(v, "eglTerminate");
  eglWaitGL = (EGLBoolean(*)(void))dlsym(v, "eglWaitGL");
  eglWaitNative = (EGLBoolean(*)(EGLint))dlsym(v, "eglWaitNative");

  // EGL 1.1 Functions
  eglBindTexImage = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLint))dlsym(v, "eglBindTexImage");
  eglReleaseTexImage = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLint))dlsym(v, "eglReleaseTexImage");
  eglSurfaceAttrib = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLint, EGLint))dlsym(v, "eglSurfaceAttrib");
  eglSwapInterval = (EGLBoolean(*)(EGLDisplay, EGLint))dlsym(v, "eglSwapInterval");

  // EGL 1.2 Functions
  eglBindAPI = (EGLBoolean(*)(EGLenum))dlsym(v, "eglBindAPI");
  eglQueryAPI = (EGLenum(*)(void))dlsym(v, "eglQueryAPI");
  eglCreatePbufferFromClientBuffer = (EGLSurface(*)(EGLDisplay, EGLenum, EGLClientBuffer, EGLConfig, const EGLint *))dlsym(v, "eglCreatePbufferFromClientBuffer");
  eglReleaseThread = (EGLBoolean(*)(void))dlsym(v, "eglReleaseThread");
  eglWaitClient = (EGLBoolean(*)(void))dlsym(v, "eglWaitClient");
  return v;
load_egl_err:
  dlclose(v);
load_egl_err_e:
  return NULL;
}
// END IMPLEMENTS EGL

// IMPLEMENTS OPENGL ES
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef khronos_int8_t GLbyte;
typedef khronos_uint8_t GLubyte;
typedef khronos_int16_t GLshort;
typedef khronos_uint16_t GLushort;
typedef int GLint;
typedef unsigned int GLuint;
typedef khronos_int32_t GLclampx;
typedef int GLsizei;
typedef khronos_float_t GLfloat;
typedef khronos_float_t GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void *GLeglClientBufferEXT;
typedef void *GLeglImageOES;
typedef char GLchar;
typedef char GLcharARB;
typedef unsigned int GLhandleARB;
typedef khronos_uint16_t GLhalf;
typedef khronos_uint16_t GLhalfARB;
typedef khronos_int32_t GLfixed;
#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1060)
#if defined(__khrplatform_h_)
typedef khronos_intptr_t GLintptr;
#else
typedef long GLintptr;
#endif
#else
#if defined(__khrplatform_h_)
typedef khronos_intptr_t GLintptr;
#else
typedef ptrdiff_t GLintptr;
#endif
#endif
#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1060)
#if defined(__khrplatform_h_)
typedef khronos_intptr_t GLintptrARB;
#else
typedef long GLintptrARB;
#endif
#else
#if defined(__khrplatform_h_)
typedef khronos_intptr_t GLintptrARB;
#else
typedef ptrdiff_t GLintptrARB;
#endif
#endif
#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1060)
#if defined(__khrplatform_h_)
typedef khronos_ssize_t GLsizeiptr;
#else
typedef long GLsizeiptr;
#endif
#else
#if defined(__khrplatform_h_)
typedef khronos_ssize_t GLsizeiptr;
#else
typedef ptrdiff_t GLsizeiptr;
#endif
#endif
#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1060)
#if defined(__khrplatform_h_)
typedef khronos_ssize_t GLsizeiptrARB;
#else
typedef long GLsizeiptrARB;
#endif
#else
#if defined(__khrplatform_h_)
typedef khronos_ssize_t GLsizeiptrARB;
#else
typedef ptrdiff_t GLsizeiptrARB;
#endif
#endif
typedef khronos_int64_t GLint64;
typedef khronos_int64_t GLint64EXT;
typedef khronos_uint64_t GLuint64;
typedef khronos_uint64_t GLuint64EXT;
typedef struct __GLsync *GLsync;

#define GL_DEPTH_BUFFER_BIT                              0x00000100
#define GL_STENCIL_BUFFER_BIT                            0x00000400
#define GL_COLOR_BUFFER_BIT                              0x00004000
#define GL_FALSE                                         0
#define GL_TRUE                                          1
#define GL_POINTS                                        0x0000
#define GL_LINES                                         0x0001
#define GL_LINE_LOOP                                     0x0002
#define GL_LINE_STRIP                                    0x0003
#define GL_TRIANGLES                                     0x0004
#define GL_TRIANGLE_STRIP                                0x0005
#define GL_TRIANGLE_FAN                                  0x0006
#define GL_ZERO                                          0
#define GL_ONE                                           1
#define GL_SRC_COLOR                                     0x0300
#define GL_ONE_MINUS_SRC_COLOR                           0x0301
#define GL_SRC_ALPHA                                     0x0302
#define GL_ONE_MINUS_SRC_ALPHA                           0x0303
#define GL_DST_ALPHA                                     0x0304
#define GL_ONE_MINUS_DST_ALPHA                           0x0305
#define GL_DST_COLOR                                     0x0306
#define GL_ONE_MINUS_DST_COLOR                           0x0307
#define GL_SRC_ALPHA_SATURATE                            0x0308
#define GL_FUNC_ADD                                      0x8006
#define GL_BLEND_EQUATION                                0x8009
#define GL_BLEND_EQUATION_RGB                            0x8009
#define GL_BLEND_EQUATION_ALPHA                          0x883D
#define GL_FUNC_SUBTRACT                                 0x800A
#define GL_FUNC_REVERSE_SUBTRACT                         0x800B
#define GL_BLEND_DST_RGB                                 0x80C8
#define GL_BLEND_SRC_RGB                                 0x80C9
#define GL_BLEND_DST_ALPHA                               0x80CA
#define GL_BLEND_SRC_ALPHA                               0x80CB
#define GL_CONSTANT_COLOR                                0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR                      0x8002
#define GL_CONSTANT_ALPHA                                0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA                      0x8004
#define GL_BLEND_COLOR                                   0x8005
#define GL_ARRAY_BUFFER                                  0x8892
#define GL_ELEMENT_ARRAY_BUFFER                          0x8893
#define GL_ARRAY_BUFFER_BINDING                          0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING                  0x8895
#define GL_STREAM_DRAW                                   0x88E0
#define GL_STATIC_DRAW                                   0x88E4
#define GL_DYNAMIC_DRAW                                  0x88E8
#define GL_BUFFER_SIZE                                   0x8764
#define GL_BUFFER_USAGE                                  0x8765
#define GL_CURRENT_VERTEX_ATTRIB                         0x8626
#define GL_FRONT                                         0x0404
#define GL_BACK                                          0x0405
#define GL_FRONT_AND_BACK                                0x0408
#define GL_TEXTURE_2D                                    0x0DE1
#define GL_CULL_FACE                                     0x0B44
#define GL_BLEND                                         0x0BE2
#define GL_DITHER                                        0x0BD0
#define GL_STENCIL_TEST                                  0x0B90
#define GL_DEPTH_TEST                                    0x0B71
#define GL_SCISSOR_TEST                                  0x0C11
#define GL_POLYGON_OFFSET_FILL                           0x8037
#define GL_SAMPLE_ALPHA_TO_COVERAGE                      0x809E
#define GL_SAMPLE_COVERAGE                               0x80A0
#define GL_NO_ERROR                                      0
#define GL_INVALID_ENUM                                  0x0500
#define GL_INVALID_VALUE                                 0x0501
#define GL_INVALID_OPERATION                             0x0502
#define GL_OUT_OF_MEMORY                                 0x0505
#define GL_CW                                            0x0900
#define GL_CCW                                           0x0901
#define GL_LINE_WIDTH                                    0x0B21
#define GL_ALIASED_POINT_SIZE_RANGE                      0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE                      0x846E
#define GL_CULL_FACE_MODE                                0x0B45
#define GL_FRONT_FACE                                    0x0B46
#define GL_DEPTH_RANGE                                   0x0B70
#define GL_DEPTH_WRITEMASK                               0x0B72
#define GL_DEPTH_CLEAR_VALUE                             0x0B73
#define GL_DEPTH_FUNC                                    0x0B74
#define GL_STENCIL_CLEAR_VALUE                           0x0B91
#define GL_STENCIL_FUNC                                  0x0B92
#define GL_STENCIL_FAIL                                  0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL                       0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS                       0x0B96
#define GL_STENCIL_REF                                   0x0B97
#define GL_STENCIL_VALUE_MASK                            0x0B93
#define GL_STENCIL_WRITEMASK                             0x0B98
#define GL_STENCIL_BACK_FUNC                             0x8800
#define GL_STENCIL_BACK_FAIL                             0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL                  0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS                  0x8803
#define GL_STENCIL_BACK_REF                              0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK                       0x8CA4
#define GL_STENCIL_BACK_WRITEMASK                        0x8CA5
#define GL_VIEWPORT                                      0x0BA2
#define GL_SCISSOR_BOX                                   0x0C10
#define GL_COLOR_CLEAR_VALUE                             0x0C22
#define GL_COLOR_WRITEMASK                               0x0C23
#define GL_UNPACK_ALIGNMENT                              0x0CF5
#define GL_PACK_ALIGNMENT                                0x0D05
#define GL_MAX_TEXTURE_SIZE                              0x0D33
#define GL_MAX_VIEWPORT_DIMS                             0x0D3A
#define GL_SUBPIXEL_BITS                                 0x0D50
#define GL_RED_BITS                                      0x0D52
#define GL_GREEN_BITS                                    0x0D53
#define GL_BLUE_BITS                                     0x0D54
#define GL_ALPHA_BITS                                    0x0D55
#define GL_DEPTH_BITS                                    0x0D56
#define GL_STENCIL_BITS                                  0x0D57
#define GL_POLYGON_OFFSET_UNITS                          0x2A00
#define GL_POLYGON_OFFSET_FACTOR                         0x8038
#define GL_TEXTURE_BINDING_2D                            0x8069
#define GL_SAMPLE_BUFFERS                                0x80A8
#define GL_SAMPLES                                       0x80A9
#define GL_SAMPLE_COVERAGE_VALUE                         0x80AA
#define GL_SAMPLE_COVERAGE_INVERT                        0x80AB
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS                0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS                    0x86A3
#define GL_DONT_CARE                                     0x1100
#define GL_FASTEST                                       0x1101
#define GL_NICEST                                        0x1102
#define GL_GENERATE_MIPMAP_HINT                          0x8192
#define GL_BYTE                                          0x1400
#define GL_UNSIGNED_BYTE                                 0x1401
#define GL_SHORT                                         0x1402
#define GL_UNSIGNED_SHORT                                0x1403
#define GL_INT                                           0x1404
#define GL_UNSIGNED_INT                                  0x1405
#define GL_FLOAT                                         0x1406
#define GL_FIXED                                         0x140C
#define GL_DEPTH_COMPONENT                               0x1902
#define GL_ALPHA                                         0x1906
#define GL_RGB                                           0x1907
#define GL_RGBA                                          0x1908
#define GL_LUMINANCE                                     0x1909
#define GL_LUMINANCE_ALPHA                               0x190A
#define GL_UNSIGNED_SHORT_4_4_4_4                        0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1                        0x8034
#define GL_UNSIGNED_SHORT_5_6_5                          0x8363
#define GL_FRAGMENT_SHADER                               0x8B30
#define GL_VERTEX_SHADER                                 0x8B31
#define GL_MAX_VERTEX_ATTRIBS                            0x8869
#define GL_MAX_VERTEX_UNIFORM_VECTORS                    0x8DFB
#define GL_MAX_VARYING_VECTORS                           0x8DFC
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS              0x8B4D
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS                0x8B4C
#define GL_MAX_TEXTURE_IMAGE_UNITS                       0x8872
#define GL_MAX_FRAGMENT_UNIFORM_VECTORS                  0x8DFD
#define GL_SHADER_TYPE                                   0x8B4F
#define GL_DELETE_STATUS                                 0x8B80
#define GL_LINK_STATUS                                   0x8B82
#define GL_VALIDATE_STATUS                               0x8B83
#define GL_ATTACHED_SHADERS                              0x8B85
#define GL_ACTIVE_UNIFORMS                               0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH                     0x8B87
#define GL_ACTIVE_ATTRIBUTES                             0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH                   0x8B8A
#define GL_SHADING_LANGUAGE_VERSION                      0x8B8C
#define GL_CURRENT_PROGRAM                               0x8B8D
#define GL_NEVER                                         0x0200
#define GL_LESS                                          0x0201
#define GL_EQUAL                                         0x0202
#define GL_LEQUAL                                        0x0203
#define GL_GREATER                                       0x0204
#define GL_NOTEQUAL                                      0x0205
#define GL_GEQUAL                                        0x0206
#define GL_ALWAYS                                        0x0207
#define GL_KEEP                                          0x1E00
#define GL_REPLACE                                       0x1E01
#define GL_INCR                                          0x1E02
#define GL_DECR                                          0x1E03
#define GL_INVERT                                        0x150A
#define GL_INCR_WRAP                                     0x8507
#define GL_DECR_WRAP                                     0x8508
#define GL_VENDOR                                        0x1F00
#define GL_RENDERER                                      0x1F01
#define GL_VERSION                                       0x1F02
#define GL_EXTENSIONS                                    0x1F03
#define GL_NEAREST                                       0x2600
#define GL_LINEAR                                        0x2601
#define GL_NEAREST_MIPMAP_NEAREST                        0x2700
#define GL_LINEAR_MIPMAP_NEAREST                         0x2701
#define GL_NEAREST_MIPMAP_LINEAR                         0x2702
#define GL_LINEAR_MIPMAP_LINEAR                          0x2703
#define GL_TEXTURE_MAG_FILTER                            0x2800
#define GL_TEXTURE_MIN_FILTER                            0x2801
#define GL_TEXTURE_WRAP_S                                0x2802
#define GL_TEXTURE_WRAP_T                                0x2803
#define GL_TEXTURE                                       0x1702
#define GL_TEXTURE_CUBE_MAP                              0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP                      0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X                   0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X                   0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y                   0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y                   0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z                   0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z                   0x851A
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE                     0x851C
#define GL_TEXTURE0                                      0x84C0
#define GL_TEXTURE1                                      0x84C1
#define GL_TEXTURE2                                      0x84C2
#define GL_TEXTURE3                                      0x84C3
#define GL_TEXTURE4                                      0x84C4
#define GL_TEXTURE5                                      0x84C5
#define GL_TEXTURE6                                      0x84C6
#define GL_TEXTURE7                                      0x84C7
#define GL_TEXTURE8                                      0x84C8
#define GL_TEXTURE9                                      0x84C9
#define GL_TEXTURE10                                     0x84CA
#define GL_TEXTURE11                                     0x84CB
#define GL_TEXTURE12                                     0x84CC
#define GL_TEXTURE13                                     0x84CD
#define GL_TEXTURE14                                     0x84CE
#define GL_TEXTURE15                                     0x84CF
#define GL_TEXTURE16                                     0x84D0
#define GL_TEXTURE17                                     0x84D1
#define GL_TEXTURE18                                     0x84D2
#define GL_TEXTURE19                                     0x84D3
#define GL_TEXTURE20                                     0x84D4
#define GL_TEXTURE21                                     0x84D5
#define GL_TEXTURE22                                     0x84D6
#define GL_TEXTURE23                                     0x84D7
#define GL_TEXTURE24                                     0x84D8
#define GL_TEXTURE25                                     0x84D9
#define GL_TEXTURE26                                     0x84DA
#define GL_TEXTURE27                                     0x84DB
#define GL_TEXTURE28                                     0x84DC
#define GL_TEXTURE29                                     0x84DD
#define GL_TEXTURE30                                     0x84DE
#define GL_TEXTURE31                                     0x84DF
#define GL_ACTIVE_TEXTURE                                0x84E0
#define GL_REPEAT                                        0x2901
#define GL_CLAMP_TO_EDGE                                 0x812F
#define GL_MIRRORED_REPEAT                               0x8370
#define GL_FLOAT_VEC2                                    0x8B50
#define GL_FLOAT_VEC3                                    0x8B51
#define GL_FLOAT_VEC4                                    0x8B52
#define GL_INT_VEC2                                      0x8B53
#define GL_INT_VEC3                                      0x8B54
#define GL_INT_VEC4                                      0x8B55
#define GL_BOOL                                          0x8B56
#define GL_BOOL_VEC2                                     0x8B57
#define GL_BOOL_VEC3                                     0x8B58
#define GL_BOOL_VEC4                                     0x8B59
#define GL_FLOAT_MAT2                                    0x8B5A
#define GL_FLOAT_MAT3                                    0x8B5B
#define GL_FLOAT_MAT4                                    0x8B5C
#define GL_SAMPLER_2D                                    0x8B5E
#define GL_SAMPLER_CUBE                                  0x8B60
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED                   0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE                      0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE                    0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE                      0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED                0x886A
#define GL_VERTEX_ATTRIB_ARRAY_POINTER                   0x8645
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING            0x889F
#define GL_IMPLEMENTATION_COLOR_READ_TYPE                0x8B9A
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT              0x8B9B
#define GL_COMPILE_STATUS                                0x8B81
#define GL_INFO_LOG_LENGTH                               0x8B84
#define GL_SHADER_SOURCE_LENGTH                          0x8B88
#define GL_SHADER_COMPILER                               0x8DFA
#define GL_SHADER_BINARY_FORMATS                         0x8DF8
#define GL_NUM_SHADER_BINARY_FORMATS                     0x8DF9
#define GL_LOW_FLOAT                                     0x8DF0
#define GL_MEDIUM_FLOAT                                  0x8DF1
#define GL_HIGH_FLOAT                                    0x8DF2
#define GL_LOW_INT                                       0x8DF3
#define GL_MEDIUM_INT                                    0x8DF4
#define GL_HIGH_INT                                      0x8DF5
#define GL_FRAMEBUFFER                                   0x8D40
#define GL_RENDERBUFFER                                  0x8D41
#define GL_RGBA4                                         0x8056
#define GL_RGB5_A1                                       0x8057
#define GL_RGB565                                        0x8D62
#define GL_DEPTH_COMPONENT16                             0x81A5
#define GL_STENCIL_INDEX8                                0x8D48
#define GL_RENDERBUFFER_WIDTH                            0x8D42
#define GL_RENDERBUFFER_HEIGHT                           0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT                  0x8D44
#define GL_RENDERBUFFER_RED_SIZE                         0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE                       0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE                        0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE                       0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE                       0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE                     0x8D55
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE            0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME            0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL          0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE  0x8CD3
#define GL_COLOR_ATTACHMENT0                             0x8CE0
#define GL_DEPTH_ATTACHMENT                              0x8D00
#define GL_STENCIL_ATTACHMENT                            0x8D20
#define GL_NONE                                          0
#define GL_FRAMEBUFFER_COMPLETE                          0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT             0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT     0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS             0x8CD9
#define GL_FRAMEBUFFER_UNSUPPORTED                       0x8CDD
#define GL_FRAMEBUFFER_BINDING                           0x8CA6
#define GL_RENDERBUFFER_BINDING                          0x8CA7
#define GL_MAX_RENDERBUFFER_SIZE                         0x84E8
#define GL_INVALID_FRAMEBUFFER_OPERATION                 0x0506
#define GL_READ_BUFFER                                   0x0C02
#define GL_UNPACK_ROW_LENGTH                             0x0CF2
#define GL_UNPACK_SKIP_ROWS                              0x0CF3
#define GL_UNPACK_SKIP_PIXELS                            0x0CF4
#define GL_PACK_ROW_LENGTH                               0x0D02
#define GL_PACK_SKIP_ROWS                                0x0D03
#define GL_PACK_SKIP_PIXELS                              0x0D04
#define GL_COLOR                                         0x1800
#define GL_DEPTH                                         0x1801
#define GL_STENCIL                                       0x1802
#define GL_RED                                           0x1903
#define GL_RGB8                                          0x8051
#define GL_RGBA8                                         0x8058
#define GL_RGB10_A2                                      0x8059
#define GL_TEXTURE_BINDING_3D                            0x806A
#define GL_UNPACK_SKIP_IMAGES                            0x806D
#define GL_UNPACK_IMAGE_HEIGHT                           0x806E
#define GL_TEXTURE_3D                                    0x806F
#define GL_TEXTURE_WRAP_R                                0x8072
#define GL_MAX_3D_TEXTURE_SIZE                           0x8073
#define GL_UNSIGNED_INT_2_10_10_10_REV                   0x8368
#define GL_MAX_ELEMENTS_VERTICES                         0x80E8
#define GL_MAX_ELEMENTS_INDICES                          0x80E9
#define GL_TEXTURE_MIN_LOD                               0x813A
#define GL_TEXTURE_MAX_LOD                               0x813B
#define GL_TEXTURE_BASE_LEVEL                            0x813C
#define GL_TEXTURE_MAX_LEVEL                             0x813D
#define GL_MIN                                           0x8007
#define GL_MAX                                           0x8008
#define GL_DEPTH_COMPONENT24                             0x81A6
#define GL_MAX_TEXTURE_LOD_BIAS                          0x84FD
#define GL_TEXTURE_COMPARE_MODE                          0x884C
#define GL_TEXTURE_COMPARE_FUNC                          0x884D
#define GL_CURRENT_QUERY                                 0x8865
#define GL_QUERY_RESULT                                  0x8866
#define GL_QUERY_RESULT_AVAILABLE                        0x8867
#define GL_BUFFER_MAPPED                                 0x88BC
#define GL_BUFFER_MAP_POINTER                            0x88BD
#define GL_STREAM_READ                                   0x88E1
#define GL_STREAM_COPY                                   0x88E2
#define GL_STATIC_READ                                   0x88E5
#define GL_STATIC_COPY                                   0x88E6
#define GL_DYNAMIC_READ                                  0x88E9
#define GL_DYNAMIC_COPY                                  0x88EA
#define GL_MAX_DRAW_BUFFERS                              0x8824
#define GL_DRAW_BUFFER0                                  0x8825
#define GL_DRAW_BUFFER1                                  0x8826
#define GL_DRAW_BUFFER2                                  0x8827
#define GL_DRAW_BUFFER3                                  0x8828
#define GL_DRAW_BUFFER4                                  0x8829
#define GL_DRAW_BUFFER5                                  0x882A
#define GL_DRAW_BUFFER6                                  0x882B
#define GL_DRAW_BUFFER7                                  0x882C
#define GL_DRAW_BUFFER8                                  0x882D
#define GL_DRAW_BUFFER9                                  0x882E
#define GL_DRAW_BUFFER10                                 0x882F
#define GL_DRAW_BUFFER11                                 0x8830
#define GL_DRAW_BUFFER12                                 0x8831
#define GL_DRAW_BUFFER13                                 0x8832
#define GL_DRAW_BUFFER14                                 0x8833
#define GL_DRAW_BUFFER15                                 0x8834
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS               0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS                 0x8B4A
#define GL_SAMPLER_3D                                    0x8B5F
#define GL_SAMPLER_2D_SHADOW                             0x8B62
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT               0x8B8B
#define GL_PIXEL_PACK_BUFFER                             0x88EB
#define GL_PIXEL_UNPACK_BUFFER                           0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING                     0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING                   0x88EF
#define GL_FLOAT_MAT2x3                                  0x8B65
#define GL_FLOAT_MAT2x4                                  0x8B66
#define GL_FLOAT_MAT3x2                                  0x8B67
#define GL_FLOAT_MAT3x4                                  0x8B68
#define GL_FLOAT_MAT4x2                                  0x8B69
#define GL_FLOAT_MAT4x3                                  0x8B6A
#define GL_SRGB                                          0x8C40
#define GL_SRGB8                                         0x8C41
#define GL_SRGB8_ALPHA8                                  0x8C43
#define GL_COMPARE_REF_TO_TEXTURE                        0x884E
#define GL_MAJOR_VERSION                                 0x821B
#define GL_MINOR_VERSION                                 0x821C
#define GL_NUM_EXTENSIONS                                0x821D
#define GL_RGBA32F                                       0x8814
#define GL_RGB32F                                        0x8815
#define GL_RGBA16F                                       0x881A
#define GL_RGB16F                                        0x881B
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER                   0x88FD
#define GL_MAX_ARRAY_TEXTURE_LAYERS                      0x88FF
#define GL_MIN_PROGRAM_TEXEL_OFFSET                      0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET                      0x8905
#define GL_MAX_VARYING_COMPONENTS                        0x8B4B
#define GL_TEXTURE_2D_ARRAY                              0x8C1A
#define GL_TEXTURE_BINDING_2D_ARRAY                      0x8C1D
#define GL_R11F_G11F_B10F                                0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV                  0x8C3B
#define GL_RGB9_E5                                       0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV                      0x8C3E
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH         0x8C76
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE                0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS    0x8C80
#define GL_TRANSFORM_FEEDBACK_VARYINGS                   0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START               0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE                0x8C85
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN         0x8C88
#define GL_RASTERIZER_DISCARD                            0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS       0x8C8B
#define GL_INTERLEAVED_ATTRIBS                           0x8C8C
#define GL_SEPARATE_ATTRIBS                              0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER                     0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING             0x8C8F
#define GL_RGBA32UI                                      0x8D70
#define GL_RGB32UI                                       0x8D71
#define GL_RGBA16UI                                      0x8D76
#define GL_RGB16UI                                       0x8D77
#define GL_RGBA8UI                                       0x8D7C
#define GL_RGB8UI                                        0x8D7D
#define GL_RGBA32I                                       0x8D82
#define GL_RGB32I                                        0x8D83
#define GL_RGBA16I                                       0x8D88
#define GL_RGB16I                                        0x8D89
#define GL_RGBA8I                                        0x8D8E
#define GL_RGB8I                                         0x8D8F
#define GL_RED_INTEGER                                   0x8D94
#define GL_RGB_INTEGER                                   0x8D98
#define GL_RGBA_INTEGER                                  0x8D99
#define GL_SAMPLER_2D_ARRAY                              0x8DC1
#define GL_SAMPLER_2D_ARRAY_SHADOW                       0x8DC4
#define GL_SAMPLER_CUBE_SHADOW                           0x8DC5
#define GL_UNSIGNED_INT_VEC2                             0x8DC6
#define GL_UNSIGNED_INT_VEC3                             0x8DC7
#define GL_UNSIGNED_INT_VEC4                             0x8DC8
#define GL_INT_SAMPLER_2D                                0x8DCA
#define GL_INT_SAMPLER_3D                                0x8DCB
#define GL_INT_SAMPLER_CUBE                              0x8DCC
#define GL_INT_SAMPLER_2D_ARRAY                          0x8DCF
#define GL_UNSIGNED_INT_SAMPLER_2D                       0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D                       0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE                     0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY                 0x8DD7
#define GL_BUFFER_ACCESS_FLAGS                           0x911F
#define GL_BUFFER_MAP_LENGTH                             0x9120
#define GL_BUFFER_MAP_OFFSET                             0x9121
#define GL_DEPTH_COMPONENT32F                            0x8CAC
#define GL_DEPTH32F_STENCIL8                             0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV                0x8DAD
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING         0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE         0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE               0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE             0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE              0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE             0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE             0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE           0x8217
#define GL_FRAMEBUFFER_DEFAULT                           0x8218
#define GL_FRAMEBUFFER_UNDEFINED                         0x8219
#define GL_DEPTH_STENCIL_ATTACHMENT                      0x821A
#define GL_DEPTH_STENCIL                                 0x84F9
#define GL_UNSIGNED_INT_24_8                             0x84FA
#define GL_DEPTH24_STENCIL8                              0x88F0
#define GL_UNSIGNED_NORMALIZED                           0x8C17
#define GL_DRAW_FRAMEBUFFER_BINDING                      0x8CA6
#define GL_READ_FRAMEBUFFER                              0x8CA8
#define GL_DRAW_FRAMEBUFFER                              0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING                      0x8CAA
#define GL_RENDERBUFFER_SAMPLES                          0x8CAB
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER          0x8CD4
#define GL_MAX_COLOR_ATTACHMENTS                         0x8CDF
#define GL_COLOR_ATTACHMENT1                             0x8CE1
#define GL_COLOR_ATTACHMENT2                             0x8CE2
#define GL_COLOR_ATTACHMENT3                             0x8CE3
#define GL_COLOR_ATTACHMENT4                             0x8CE4
#define GL_COLOR_ATTACHMENT5                             0x8CE5
#define GL_COLOR_ATTACHMENT6                             0x8CE6
#define GL_COLOR_ATTACHMENT7                             0x8CE7
#define GL_COLOR_ATTACHMENT8                             0x8CE8
#define GL_COLOR_ATTACHMENT9                             0x8CE9
#define GL_COLOR_ATTACHMENT10                            0x8CEA
#define GL_COLOR_ATTACHMENT11                            0x8CEB
#define GL_COLOR_ATTACHMENT12                            0x8CEC
#define GL_COLOR_ATTACHMENT13                            0x8CED
#define GL_COLOR_ATTACHMENT14                            0x8CEE
#define GL_COLOR_ATTACHMENT15                            0x8CEF
#define GL_COLOR_ATTACHMENT16                            0x8CF0
#define GL_COLOR_ATTACHMENT17                            0x8CF1
#define GL_COLOR_ATTACHMENT18                            0x8CF2
#define GL_COLOR_ATTACHMENT19                            0x8CF3
#define GL_COLOR_ATTACHMENT20                            0x8CF4
#define GL_COLOR_ATTACHMENT21                            0x8CF5
#define GL_COLOR_ATTACHMENT22                            0x8CF6
#define GL_COLOR_ATTACHMENT23                            0x8CF7
#define GL_COLOR_ATTACHMENT24                            0x8CF8
#define GL_COLOR_ATTACHMENT25                            0x8CF9
#define GL_COLOR_ATTACHMENT26                            0x8CFA
#define GL_COLOR_ATTACHMENT27                            0x8CFB
#define GL_COLOR_ATTACHMENT28                            0x8CFC
#define GL_COLOR_ATTACHMENT29                            0x8CFD
#define GL_COLOR_ATTACHMENT30                            0x8CFE
#define GL_COLOR_ATTACHMENT31                            0x8CFF
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE            0x8D56
#define GL_MAX_SAMPLES                                   0x8D57
#define GL_HALF_FLOAT                                    0x140B
#define GL_MAP_READ_BIT                                  0x0001
#define GL_MAP_WRITE_BIT                                 0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT                      0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT                     0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT                        0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT                        0x0020
#define GL_RG                                            0x8227
#define GL_RG_INTEGER                                    0x8228
#define GL_R8                                            0x8229
#define GL_RG8                                           0x822B
#define GL_R16F                                          0x822D
#define GL_R32F                                          0x822E
#define GL_RG16F                                         0x822F
#define GL_RG32F                                         0x8230
#define GL_R8I                                           0x8231
#define GL_R8UI                                          0x8232
#define GL_R16I                                          0x8233
#define GL_R16UI                                         0x8234
#define GL_R32I                                          0x8235
#define GL_R32UI                                         0x8236
#define GL_RG8I                                          0x8237
#define GL_RG8UI                                         0x8238
#define GL_RG16I                                         0x8239
#define GL_RG16UI                                        0x823A
#define GL_RG32I                                         0x823B
#define GL_RG32UI                                        0x823C
#define GL_VERTEX_ARRAY_BINDING                          0x85B5
#define GL_R8_SNORM                                      0x8F94
#define GL_RG8_SNORM                                     0x8F95
#define GL_RGB8_SNORM                                    0x8F96
#define GL_RGBA8_SNORM                                   0x8F97
#define GL_SIGNED_NORMALIZED                             0x8F9C
#define GL_PRIMITIVE_RESTART_FIXED_INDEX                 0x8D69
#define GL_COPY_READ_BUFFER                              0x8F36
#define GL_COPY_WRITE_BUFFER                             0x8F37
#define GL_COPY_READ_BUFFER_BINDING                      0x8F36
#define GL_COPY_WRITE_BUFFER_BINDING                     0x8F37
#define GL_UNIFORM_BUFFER                                0x8A11
#define GL_UNIFORM_BUFFER_BINDING                        0x8A28
#define GL_UNIFORM_BUFFER_START                          0x8A29
#define GL_UNIFORM_BUFFER_SIZE                           0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS                     0x8A2B
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS                   0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS                   0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS                   0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE                        0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS        0x8A31
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS      0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT               0x8A34
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH          0x8A35
#define GL_ACTIVE_UNIFORM_BLOCKS                         0x8A36
#define GL_UNIFORM_TYPE                                  0x8A37
#define GL_UNIFORM_SIZE                                  0x8A38
#define GL_UNIFORM_NAME_LENGTH                           0x8A39
#define GL_UNIFORM_BLOCK_INDEX                           0x8A3A
#define GL_UNIFORM_OFFSET                                0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE                          0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE                         0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR                          0x8A3E
#define GL_UNIFORM_BLOCK_BINDING                         0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE                       0x8A40
#define GL_UNIFORM_BLOCK_NAME_LENGTH                     0x8A41
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS                 0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES          0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER     0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER   0x8A46
#define GL_INVALID_INDEX                                 0xFFFFFFFF
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS                  0x9122
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS                 0x9125
#define GL_MAX_SERVER_WAIT_TIMEOUT                       0x9111
#define GL_OBJECT_TYPE                                   0x9112
#define GL_SYNC_CONDITION                                0x9113
#define GL_SYNC_STATUS                                   0x9114
#define GL_SYNC_FLAGS                                    0x9115
#define GL_SYNC_FENCE                                    0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE                    0x9117
#define GL_UNSIGNALED                                    0x9118
#define GL_SIGNALED                                      0x9119
#define GL_ALREADY_SIGNALED                              0x911A
#define GL_TIMEOUT_EXPIRED                               0x911B
#define GL_CONDITION_SATISFIED                           0x911C
#define GL_WAIT_FAILED                                   0x911D
#define GL_SYNC_FLUSH_COMMANDS_BIT                       0x00000001
#define GL_TIMEOUT_IGNORED                               0xFFFFFFFFFFFFFFFF
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR                   0x88FE
#define GL_ANY_SAMPLES_PASSED                            0x8C2F
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE               0x8D6A
#define GL_SAMPLER_BINDING                               0x8919
#define GL_RGB10_A2UI                                    0x906F
#define GL_TEXTURE_SWIZZLE_R                             0x8E42
#define GL_TEXTURE_SWIZZLE_G                             0x8E43
#define GL_TEXTURE_SWIZZLE_B                             0x8E44
#define GL_TEXTURE_SWIZZLE_A                             0x8E45
#define GL_GREEN                                         0x1904
#define GL_BLUE                                          0x1905
#define GL_INT_2_10_10_10_REV                            0x8D9F
#define GL_TRANSFORM_FEEDBACK                            0x8E22
#define GL_TRANSFORM_FEEDBACK_PAUSED                     0x8E23
#define GL_TRANSFORM_FEEDBACK_ACTIVE                     0x8E24
#define GL_TRANSFORM_FEEDBACK_BINDING                    0x8E25
#define GL_PROGRAM_BINARY_RETRIEVABLE_HINT               0x8257
#define GL_PROGRAM_BINARY_LENGTH                         0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS                    0x87FE
#define GL_PROGRAM_BINARY_FORMATS                        0x87FF
#define GL_COMPRESSED_R11_EAC                            0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC                     0x9271
#define GL_COMPRESSED_RG11_EAC                           0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC                    0x9273
#define GL_COMPRESSED_RGB8_ETC2                          0x9274
#define GL_COMPRESSED_SRGB8_ETC2                         0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2      0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2     0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC                     0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC              0x9279
#define GL_TEXTURE_IMMUTABLE_FORMAT                      0x912F
#define GL_MAX_ELEMENT_INDEX                             0x8D6B
#define GL_NUM_SAMPLE_COUNTS                             0x9380
#define GL_TEXTURE_IMMUTABLE_LEVELS                      0x82DF
#define GL_COMPUTE_SHADER                                0x91B9
#define GL_MAX_COMPUTE_UNIFORM_BLOCKS                    0x91BB
#define GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS               0x91BC
#define GL_MAX_COMPUTE_IMAGE_UNIFORMS                    0x91BD
#define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE                0x8262
#define GL_MAX_COMPUTE_UNIFORM_COMPONENTS                0x8263
#define GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS            0x8264
#define GL_MAX_COMPUTE_ATOMIC_COUNTERS                   0x8265
#define GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS       0x8266
#define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS            0x90EB
#define GL_MAX_COMPUTE_WORK_GROUP_COUNT                  0x91BE
#define GL_MAX_COMPUTE_WORK_GROUP_SIZE                   0x91BF
#define GL_COMPUTE_WORK_GROUP_SIZE                       0x8267
#define GL_DISPATCH_INDIRECT_BUFFER                      0x90EE
#define GL_DISPATCH_INDIRECT_BUFFER_BINDING              0x90EF
#define GL_COMPUTE_SHADER_BIT                            0x00000020
#define GL_DRAW_INDIRECT_BUFFER                          0x8F3F
#define GL_DRAW_INDIRECT_BUFFER_BINDING                  0x8F43
#define GL_MAX_UNIFORM_LOCATIONS                         0x826E
#define GL_FRAMEBUFFER_DEFAULT_WIDTH                     0x9310
#define GL_FRAMEBUFFER_DEFAULT_HEIGHT                    0x9311
#define GL_FRAMEBUFFER_DEFAULT_SAMPLES                   0x9313
#define GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS    0x9314
#define GL_MAX_FRAMEBUFFER_WIDTH                         0x9315
#define GL_MAX_FRAMEBUFFER_HEIGHT                        0x9316
#define GL_MAX_FRAMEBUFFER_SAMPLES                       0x9318
#define GL_UNIFORM                                       0x92E1
#define GL_UNIFORM_BLOCK                                 0x92E2
#define GL_PROGRAM_INPUT                                 0x92E3
#define GL_PROGRAM_OUTPUT                                0x92E4
#define GL_BUFFER_VARIABLE                               0x92E5
#define GL_SHADER_STORAGE_BLOCK                          0x92E6
#define GL_ATOMIC_COUNTER_BUFFER                         0x92C0
#define GL_TRANSFORM_FEEDBACK_VARYING                    0x92F4
#define GL_ACTIVE_RESOURCES                              0x92F5
#define GL_MAX_NAME_LENGTH                               0x92F6
#define GL_MAX_NUM_ACTIVE_VARIABLES                      0x92F7
#define GL_NAME_LENGTH                                   0x92F9
#define GL_TYPE                                          0x92FA
#define GL_ARRAY_SIZE                                    0x92FB
#define GL_OFFSET                                        0x92FC
#define GL_BLOCK_INDEX                                   0x92FD
#define GL_ARRAY_STRIDE                                  0x92FE
#define GL_MATRIX_STRIDE                                 0x92FF
#define GL_IS_ROW_MAJOR                                  0x9300
#define GL_ATOMIC_COUNTER_BUFFER_INDEX                   0x9301
#define GL_BUFFER_BINDING                                0x9302
#define GL_BUFFER_DATA_SIZE                              0x9303
#define GL_NUM_ACTIVE_VARIABLES                          0x9304
#define GL_ACTIVE_VARIABLES                              0x9305
#define GL_REFERENCED_BY_VERTEX_SHADER                   0x9306
#define GL_REFERENCED_BY_FRAGMENT_SHADER                 0x930A
#define GL_REFERENCED_BY_COMPUTE_SHADER                  0x930B
#define GL_TOP_LEVEL_ARRAY_SIZE                          0x930C
#define GL_TOP_LEVEL_ARRAY_STRIDE                        0x930D
#define GL_LOCATION                                      0x930E
#define GL_VERTEX_SHADER_BIT                             0x00000001
#define GL_FRAGMENT_SHADER_BIT                           0x00000002
#define GL_ALL_SHADER_BITS                               0xFFFFFFFF
#define GL_PROGRAM_SEPARABLE                             0x8258
#define GL_ACTIVE_PROGRAM                                0x8259
#define GL_PROGRAM_PIPELINE_BINDING                      0x825A
#define GL_ATOMIC_COUNTER_BUFFER_BINDING                 0x92C1
#define GL_ATOMIC_COUNTER_BUFFER_START                   0x92C2
#define GL_ATOMIC_COUNTER_BUFFER_SIZE                    0x92C3
#define GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS             0x92CC
#define GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS           0x92D0
#define GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS           0x92D1
#define GL_MAX_VERTEX_ATOMIC_COUNTERS                    0x92D2
#define GL_MAX_FRAGMENT_ATOMIC_COUNTERS                  0x92D6
#define GL_MAX_COMBINED_ATOMIC_COUNTERS                  0x92D7
#define GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE                0x92D8
#define GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS            0x92DC
#define GL_ACTIVE_ATOMIC_COUNTER_BUFFERS                 0x92D9
#define GL_UNSIGNED_INT_ATOMIC_COUNTER                   0x92DB
#define GL_MAX_IMAGE_UNITS                               0x8F38
#define GL_MAX_VERTEX_IMAGE_UNIFORMS                     0x90CA
#define GL_MAX_FRAGMENT_IMAGE_UNIFORMS                   0x90CE
#define GL_MAX_COMBINED_IMAGE_UNIFORMS                   0x90CF
#define GL_IMAGE_BINDING_NAME                            0x8F3A
#define GL_IMAGE_BINDING_LEVEL                           0x8F3B
#define GL_IMAGE_BINDING_LAYERED                         0x8F3C
#define GL_IMAGE_BINDING_LAYER                           0x8F3D
#define GL_IMAGE_BINDING_ACCESS                          0x8F3E
#define GL_IMAGE_BINDING_FORMAT                          0x906E
#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT               0x00000001
#define GL_ELEMENT_ARRAY_BARRIER_BIT                     0x00000002
#define GL_UNIFORM_BARRIER_BIT                           0x00000004
#define GL_TEXTURE_FETCH_BARRIER_BIT                     0x00000008
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT               0x00000020
#define GL_COMMAND_BARRIER_BIT                           0x00000040
#define GL_PIXEL_BUFFER_BARRIER_BIT                      0x00000080
#define GL_TEXTURE_UPDATE_BARRIER_BIT                    0x00000100
#define GL_BUFFER_UPDATE_BARRIER_BIT                     0x00000200
#define GL_FRAMEBUFFER_BARRIER_BIT                       0x00000400
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT                0x00000800
#define GL_ATOMIC_COUNTER_BARRIER_BIT                    0x00001000
#define GL_ALL_BARRIER_BITS                              0xFFFFFFFF
#define GL_IMAGE_2D                                      0x904D
#define GL_IMAGE_3D                                      0x904E
#define GL_IMAGE_CUBE                                    0x9050
#define GL_IMAGE_2D_ARRAY                                0x9053
#define GL_INT_IMAGE_2D                                  0x9058
#define GL_INT_IMAGE_3D                                  0x9059
#define GL_INT_IMAGE_CUBE                                0x905B
#define GL_INT_IMAGE_2D_ARRAY                            0x905E
#define GL_UNSIGNED_INT_IMAGE_2D                         0x9063
#define GL_UNSIGNED_INT_IMAGE_3D                         0x9064
#define GL_UNSIGNED_INT_IMAGE_CUBE                       0x9066
#define GL_UNSIGNED_INT_IMAGE_2D_ARRAY                   0x9069
#define GL_IMAGE_FORMAT_COMPATIBILITY_TYPE               0x90C7
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE            0x90C8
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS           0x90C9
#define GL_READ_ONLY                                     0x88B8
#define GL_WRITE_ONLY                                    0x88B9
#define GL_READ_WRITE                                    0x88BA
#define GL_SHADER_STORAGE_BUFFER                         0x90D2
#define GL_SHADER_STORAGE_BUFFER_BINDING                 0x90D3
#define GL_SHADER_STORAGE_BUFFER_START                   0x90D4
#define GL_SHADER_STORAGE_BUFFER_SIZE                    0x90D5
#define GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS              0x90D6
#define GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS            0x90DA
#define GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS             0x90DB
#define GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS            0x90DC
#define GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS            0x90DD
#define GL_MAX_SHADER_STORAGE_BLOCK_SIZE                 0x90DE
#define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT        0x90DF
#define GL_SHADER_STORAGE_BARRIER_BIT                    0x00002000
#define GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES          0x8F39
#define GL_DEPTH_STENCIL_TEXTURE_MODE                    0x90EA
#define GL_STENCIL_INDEX                                 0x1901
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET             0x8E5E
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET             0x8E5F
#define GL_SAMPLE_POSITION                               0x8E50
#define GL_SAMPLE_MASK                                   0x8E51
#define GL_SAMPLE_MASK_VALUE                             0x8E52
#define GL_TEXTURE_2D_MULTISAMPLE                        0x9100
#define GL_MAX_SAMPLE_MASK_WORDS                         0x8E59
#define GL_MAX_COLOR_TEXTURE_SAMPLES                     0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES                     0x910F
#define GL_MAX_INTEGER_SAMPLES                           0x9110
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE                0x9104
#define GL_TEXTURE_SAMPLES                               0x9106
#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS                0x9107
#define GL_TEXTURE_WIDTH                                 0x1000
#define GL_TEXTURE_HEIGHT                                0x1001
#define GL_TEXTURE_DEPTH                                 0x8071
#define GL_TEXTURE_INTERNAL_FORMAT                       0x1003
#define GL_TEXTURE_RED_SIZE                              0x805C
#define GL_TEXTURE_GREEN_SIZE                            0x805D
#define GL_TEXTURE_BLUE_SIZE                             0x805E
#define GL_TEXTURE_ALPHA_SIZE                            0x805F
#define GL_TEXTURE_DEPTH_SIZE                            0x884A
#define GL_TEXTURE_STENCIL_SIZE                          0x88F1
#define GL_TEXTURE_SHARED_SIZE                           0x8C3F
#define GL_TEXTURE_RED_TYPE                              0x8C10
#define GL_TEXTURE_GREEN_TYPE                            0x8C11
#define GL_TEXTURE_BLUE_TYPE                             0x8C12
#define GL_TEXTURE_ALPHA_TYPE                            0x8C13
#define GL_TEXTURE_DEPTH_TYPE                            0x8C16
#define GL_TEXTURE_COMPRESSED                            0x86A1
#define GL_SAMPLER_2D_MULTISAMPLE                        0x9108
#define GL_INT_SAMPLER_2D_MULTISAMPLE                    0x9109
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE           0x910A
#define GL_VERTEX_ATTRIB_BINDING                         0x82D4
#define GL_VERTEX_ATTRIB_RELATIVE_OFFSET                 0x82D5
#define GL_VERTEX_BINDING_DIVISOR                        0x82D6
#define GL_VERTEX_BINDING_OFFSET                         0x82D7
#define GL_VERTEX_BINDING_STRIDE                         0x82D8
#define GL_VERTEX_BINDING_BUFFER                         0x8F4F
#define GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET             0x82D9
#define GL_MAX_VERTEX_ATTRIB_BINDINGS                    0x82DA
#define GL_MAX_VERTEX_ATTRIB_STRIDE                      0x82E5

// --- GL ES 2.0 Core Functions ---
static void (*glActiveTexture)(GLenum texture) = NULL;
static void (*glAttachShader)(GLuint program, GLuint shader) = NULL;
static void (*glBindAttribLocation)(GLuint program, GLuint index, const GLchar *name) = NULL;
static void (*glBindBuffer)(GLenum target, GLuint buffer) = NULL;
static void (*glBindFramebuffer)(GLenum target, GLuint framebuffer) = NULL;
static void (*glBindRenderbuffer)(GLenum target, GLuint renderbuffer) = NULL;
static void (*glBindTexture)(GLenum target, GLuint texture) = NULL;
static void (*glBlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) = NULL;
static void (*glBlendEquation)(GLenum mode) = NULL;
static void (*glBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha) = NULL;
static void (*glBlendFunc)(GLenum sfactor, GLenum dfactor) = NULL;
static void (*glBlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) = NULL;
static void (*glClear)(GLbitfield mask) = NULL;
static void (*glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) = NULL;
static void (*glClearDepthf)(GLfloat d) = NULL;
static void (*glClearStencil)(GLint s) = NULL;
static void (*glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) = NULL;
static void (*glCompileShader)(GLuint shader) = NULL;
static void (*glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data) = NULL;
static void (*glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data) = NULL;
static void (*glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) = NULL;
static void (*glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
static GLuint (*glCreateProgram)(void) = NULL;
static GLuint (*glCreateShader)(GLenum type) = NULL;
static void (*glCullFace)(GLenum mode) = NULL;
static void (*glDeleteBuffers)(GLsizei n, const GLuint *buffers) = NULL;
static void (*glDeleteFramebuffers)(GLsizei n, const GLuint *framebuffers) = NULL;
static void (*glDeleteProgram)(GLuint program) = NULL;
static void (*glDeleteRenderbuffers)(GLsizei n, const GLuint *renderbuffers) = NULL;
static void (*glDeleteShader)(GLuint shader) = NULL;
static void (*glDeleteTextures)(GLsizei n, const GLuint *textures) = NULL;
static void (*glDepthFunc)(GLenum func) = NULL;
static void (*glDepthMask)(GLboolean flag) = NULL;
static void (*glDepthRangef)(GLfloat n, GLfloat f) = NULL;
static void (*glDetachShader)(GLuint program, GLuint shader) = NULL;
static void (*glDisable)(GLenum cap) = NULL;
static void (*glDisableVertexAttribArray)(GLuint index) = NULL;
static void (*glDrawArrays)(GLenum mode, GLint first, GLsizei count) = NULL;
static void (*glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void *indices) = NULL;
static void (*glEnable)(GLenum cap) = NULL;
static void (*glEnableVertexAttribArray)(GLuint index) = NULL;
static void (*glFinish)(void) = NULL;
static void (*glFlush)(void) = NULL;
static void (*glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) = NULL;
static void (*glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) = NULL;
static void (*glFrontFace)(GLenum mode) = NULL;
static void (*glGenBuffers)(GLsizei n, GLuint *buffers) = NULL;
static void (*glGenerateMipmap)(GLenum target) = NULL;
static void (*glGenFramebuffers)(GLsizei n, GLuint *framebuffers) = NULL;
static void (*glGenRenderbuffers)(GLsizei n, GLuint *renderbuffers) = NULL;
static void (*glGenTextures)(GLsizei n, GLuint *textures) = NULL;
static void (*glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) = NULL;
static void (*glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) = NULL;
static void (*glGetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders) = NULL;
static GLint (*glGetAttribLocation)(GLuint program, const GLchar *name) = NULL;
static void (*glGetBooleanv)(GLenum pname, GLboolean *data) = NULL;
static void (*glGetBufferParameteriv)(GLenum target, GLenum pname, GLint *params) = NULL;
static GLenum (*glGetError)(void) = NULL;
static void (*glGetFloatv)(GLenum pname, GLfloat *data) = NULL;
static void (*glGetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint *params) = NULL;
static void (*glGetIntegerv)(GLenum pname, GLint *data) = NULL;
static void (*glGetProgramiv)(GLuint program, GLenum pname, GLint *params) = NULL;
static void (*glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) = NULL;
static void (*glGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint *params) = NULL;
static void (*glGetShaderiv)(GLuint shader, GLenum pname, GLint *params) = NULL;
static void (*glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) = NULL;
static void (*glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision) = NULL;
static void (*glGetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) = NULL;
static const GLubyte *(*glGetString)(GLenum name) = NULL;
static void (*glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params) = NULL;
static void (*glGetTexParameteriv)(GLenum target, GLenum pname, GLint *params) = NULL;
static GLint (*glGetUniformLocation)(GLuint program, const GLchar *name) = NULL;
static void (*glGetUniformfv)(GLuint program, GLint location, GLfloat *params) = NULL;
static void (*glGetUniformiv)(GLuint program, GLint location, GLint *params) = NULL;
static void (*glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat *params) = NULL;
static void (*glGetVertexAttribiv)(GLuint index, GLenum pname, GLint *params) = NULL;
static void (*glGetVertexAttribPointerv)(GLuint index, GLenum pname, void **pointer) = NULL;
static void (*glHint)(GLenum target, GLenum mode) = NULL;
static GLboolean (*glIsBuffer)(GLuint buffer) = NULL;
static GLboolean (*glIsEnabled)(GLenum cap) = NULL;
static GLboolean (*glIsFramebuffer)(GLuint framebuffer) = NULL;
static GLboolean (*glIsProgram)(GLuint program) = NULL;
static GLboolean (*glIsRenderbuffer)(GLuint renderbuffer) = NULL;
static GLboolean (*glIsShader)(GLuint shader) = NULL;
static GLboolean (*glIsTexture)(GLuint texture) = NULL;
static void (*glLineWidth)(GLfloat width) = NULL;
static void (*glLinkProgram)(GLuint program) = NULL;
static void (*glPixelStorei)(GLenum pname, GLint param) = NULL;
static void (*glPolygonOffset)(GLfloat factor, GLfloat units) = NULL;
static void (*glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) = NULL;
static void (*glReleaseShaderCompiler)(void) = NULL;
static void (*glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) = NULL;
static void (*glSampleCoverage)(GLfloat value, GLboolean invert) = NULL;
static void (*glScissor)(GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
static void (*glShaderBinary)(GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length) = NULL;
static void (*glShaderSource)(GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length) = NULL;
static void (*glStencilFunc)(GLenum func, GLint ref, GLuint mask) = NULL;
static void (*glStencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask) = NULL;
static void (*glStencilMask)(GLuint mask) = NULL;
static void (*glStencilMaskSeparate)(GLenum face, GLuint mask) = NULL;
static void (*glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass) = NULL;
static void (*glStencilOpSeparate)(GLenum face, GLenum fail, GLenum zfail, GLenum zpass) = NULL;
static void (*glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) = NULL;
static void (*glTexParameterf)(GLenum target, GLenum pname, GLfloat param) = NULL;
static void (*glTexParameterfv)(GLenum target, GLenum pname, const GLfloat *params) = NULL;
static void (*glTexParameteri)(GLenum target, GLenum pname, GLint param) = NULL;
static void (*glTexParameteriv)(GLenum target, GLenum pname, const GLint *params) = NULL;
static void (*glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) = NULL;
static void (*glUniform1f)(GLint location, GLfloat v0) = NULL;
static void (*glUniform1fv)(GLint location, GLsizei count, const GLfloat *value) = NULL;
static void (*glUniform1i)(GLint location, GLint v0) = NULL;
static void (*glUniform1iv)(GLint location, GLsizei count, const GLint *value) = NULL;
static void (*glUniform2f)(GLint location, GLfloat v0, GLfloat v1) = NULL;
static void (*glUniform2fv)(GLint location, GLsizei count, const GLfloat *value) = NULL;
static void (*glUniform2i)(GLint location, GLint v0, GLint v1) = NULL;
static void (*glUniform2iv)(GLint location, GLsizei count, const GLint *value) = NULL;
static void (*glUniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) = NULL;
static void (*glUniform3fv)(GLint location, GLsizei count, const GLfloat *value) = NULL;
static void (*glUniform3i)(GLint location, GLint v0, GLint v1, GLint v2) = NULL;
static void (*glUniform3iv)(GLint location, GLsizei count, const GLint *value) = NULL;
static void (*glUniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) = NULL;
static void (*glUniform4fv)(GLint location, GLsizei count, const GLfloat *value) = NULL;
static void (*glUniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) = NULL;
static void (*glUniform4iv)(GLint location, GLsizei count, const GLint *value) = NULL;
static void (*glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = NULL;
static void (*glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = NULL;
static void (*glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = NULL;
static void (*glUseProgram)(GLuint program) = NULL;
static void (*glValidateProgram)(GLuint program) = NULL;
static void (*glVertexAttrib1f)(GLuint index, GLfloat x) = NULL;
static void (*glVertexAttrib1fv)(GLuint index, const GLfloat *v) = NULL;
static void (*glVertexAttrib2f)(GLuint index, GLfloat x, GLfloat y) = NULL;
static void (*glVertexAttrib2fv)(GLuint index, const GLfloat *v) = NULL;
static void (*glVertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z) = NULL;
static void (*glVertexAttrib3fv)(GLuint index, const GLfloat *v) = NULL;
static void (*glVertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) = NULL;
static void (*glVertexAttrib4fv)(GLuint index, const GLfloat *v) = NULL;
static void (*glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) = NULL;
static void (*glViewport)(GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
static void (*glBufferData)(GLenum, GLsizeiptr, const void *, GLenum) = NULL;
static void (*glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const void *) = NULL;

// --- GL ES 3.0 Core Functions ---
static void (*glReadBuffer)(GLenum src) = NULL;
static void (*glDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices) = NULL;
static void (*glTexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels) = NULL;
static void (*glTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels) = NULL;
static void (*glCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
static void (*glCompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data) = NULL;
static void (*glCompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data) = NULL;
static void (*glGenQueries)(GLsizei n, GLuint *ids) = NULL;
static void (*glDeleteQueries)(GLsizei n, const GLuint *ids) = NULL;
static GLboolean (*glIsQuery)(GLuint id) = NULL;
static void (*glBeginQuery)(GLenum target, GLuint id) = NULL;
static void (*glEndQuery)(GLenum target) = NULL;
static void (*glGetQueryiv)(GLenum target, GLenum pname, GLint *params) = NULL;
static void (*glGetQueryObjectuiv)(GLuint id, GLenum pname, GLuint *params) = NULL;
static void (*glUnmapBuffer)(GLenum target) = NULL;
static const void *(*glMapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) = NULL;
static void (*glFlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr) = NULL;
static void (*glGenSamplers)(GLsizei count, GLuint *samplers) = NULL;
static void (*glDeleteSamplers)(GLsizei count, const GLuint *samplers) = NULL;
static GLboolean (*glIsSampler)(GLuint sampler) = NULL;
static void (*glBindSampler)(GLuint unit, GLuint sampler) = NULL;
static void (*glSamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param) = NULL;
static void (*glSamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat *params) = NULL;
static void (*glSamplerParameteri)(GLuint sampler, GLenum pname, GLint param) = NULL;
static void (*glSamplerParameteriv)(GLuint sampler, GLenum pname, const GLint *params) = NULL;
static void (*glGetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat *params) = NULL;
static void (*glGetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint *params) = NULL;
static void (*glVertexAttribDivisor)(GLuint index, GLuint divisor) = NULL;
static void (*glBindTransformFeedback)(GLenum target, GLuint id) = NULL;
static void (*glDeleteTransformFeedbacks)(GLsizei n, const GLuint *ids) = NULL;
static GLboolean (*glIsTransformFeedback)(GLuint id) = NULL;
static void (*glGenTransformFeedbacks)(GLsizei n, GLuint *ids) = NULL;
static void (*glPauseTransformFeedback)(void) = NULL;
static void (*glResumeTransformFeedback)(void) = NULL;
static void (*glBeginTransformFeedback)(GLenum primitiveMode) = NULL;
static void (*glEndTransformFeedback)(void) = NULL;
static void (*glTransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar *const *varyings, GLenum bufferMode) = NULL;
static void (*glGetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name) = NULL;
static void (*glBindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) = NULL;
static void (*glBindBufferBase)(GLenum target, GLuint index, GLuint buffer) = NULL;
static void (*glGetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName) = NULL;
static void (*glGetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params) = NULL;
static void (*glGetUniformBlockIndex)(GLuint program, const GLchar *uniformBlockName) = NULL;
static void (*glUniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) = NULL;
static void (*glGetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar *const *uniformNames, GLuint *uniformIndices) = NULL;
static void (*glGetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params) = NULL;
static GLsync (*glFenceSync)(GLenum condition, GLbitfield flags) = NULL;
static GLboolean (*glIsSync)(GLsync sync) = NULL;
static void (*glDeleteSync)(GLsync sync) = NULL;
static GLenum (*glClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout) = NULL;
static void (*glWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout) = NULL;
static void (*glGetInteger64v)(GLenum pname, GLint64 *data) = NULL;
static void (*glGetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values) = NULL;
static void (*glCopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) = NULL;
static void (*glGetBufferParameteri64v)(GLenum target, GLenum pname, GLint64 *params) = NULL;
static void (*glBlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) = NULL;
static GLenum (*glCheckFramebufferStatus)(GLenum target) = NULL;
static void (*glDrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) = NULL;
static void (*glDrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount) = NULL;
static void (*glFramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) = NULL;
static void (*glGetIntegeri_v)(GLenum target, GLuint index, GLint *data) = NULL;
static void (*glGetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary) = NULL;
static const GLubyte *(*glGetStringi)(GLenum name, GLuint index) = NULL;
static void (*glProgramBinary)(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length) = NULL;
static void (*glProgramParameteri)(GLuint program, GLenum pname, GLint value) = NULL;
static void (*glTexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) = NULL;
static void (*glTexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) = NULL;
static void (*glUniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = NULL;
static void (*glUniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = NULL;
static void (*glUniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = NULL;
static void (*glUniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = NULL;
static void (*glUniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = NULL;
static void (*glUniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = NULL;
static void (*glVertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w) = NULL;
static void (*glVertexAttribI4iv)(GLuint index, const GLint *v) = NULL;
static void (*glVertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) = NULL;
static void (*glVertexAttribI4uiv)(GLuint index, const GLuint *v) = NULL;
static void (*glVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer) = NULL;
static void (*glGetVertexAttribIiv)(GLuint index, GLenum pname, GLint *params) = NULL;
static void (*glGetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint *params) = NULL;
static GLint (*glGetFragDataLocation)(GLuint program, const GLchar *name) = NULL;
static void (*glBindVertexArray)(GLuint) = NULL;
static void (*glGenVertexArrays)(GLsizei, GLuint *) = NULL;
static void (*glDeleteVertexArrays)(GLsizei, const GLuint *) = NULL;
// --- GL ES 3.1 Core Functions ---
/*
static void (*glDispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) = NULL;
static void (*glDispatchComputeIndirect)(GLintptr indirect) = NULL;
static void (*glDrawArraysIndirect)(GLenum mode, const void *indirect) = NULL;
static void (*glDrawElementsIndirect)(GLenum mode, GLenum type, const void *indirect) = NULL;
static void (*glFramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level) = NULL;
static void (*glGetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar *name) = NULL;
static void (*glGetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name) = NULL;
static void (*glGetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params) = NULL;
static GLint (*glGetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar *name) = NULL;
static void (*glShaderStorageBlockBinding)(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding) = NULL;
static void (*glTexBuffer)(GLenum target, GLenum internalformat, GLuint buffer) = NULL;
static void (*glTexBufferRange)(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) = NULL;
static void (*glTexImage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) = NULL;
*/
static void *loadGLES(void) {
  void *v = dlopen("libGLESv3.so", RTLD_NOW | RTLD_LOCAL);
  if (!v)
    return NULL;
  // --- GL ES 2.0 Core Functions ---
  glActiveTexture = (void (*)(GLenum))dlsym(v, "glActiveTexture");
  glAttachShader = (void (*)(GLuint, GLuint))dlsym(v, "glAttachShader");
  glBindAttribLocation = (void (*)(GLuint, GLuint, const GLchar *))dlsym(v, "glBindAttribLocation");
  glBindBuffer = (void (*)(GLenum, GLuint))dlsym(v, "glBindBuffer");
  glBindFramebuffer = (void (*)(GLenum, GLuint))dlsym(v, "glBindFramebuffer");
  glBindRenderbuffer = (void (*)(GLenum, GLuint))dlsym(v, "glBindRenderbuffer");
  glBindTexture = (void (*)(GLenum, GLuint))dlsym(v, "glBindTexture");
  glBlendColor = (void (*)(GLfloat, GLfloat, GLfloat, GLfloat))dlsym(v, "glBlendColor");
  glBlendEquation = (void (*)(GLenum))dlsym(v, "glBlendEquation");
  glBlendEquationSeparate = (void (*)(GLenum, GLenum))dlsym(v, "glBlendEquationSeparate");
  glBlendFunc = (void (*)(GLenum, GLenum))dlsym(v, "glBlendFunc");
  glBlendFuncSeparate = (void (*)(GLenum, GLenum, GLenum, GLenum))dlsym(v, "glBlendFuncSeparate");
  glClear = (void (*)(GLbitfield))dlsym(v, "glClear");
  glClearColor = (void (*)(GLfloat, GLfloat, GLfloat, GLfloat))dlsym(v, "glClearColor");
  glClearDepthf = (void (*)(GLfloat))dlsym(v, "glClearDepthf");
  glClearStencil = (void (*)(GLint))dlsym(v, "glClearStencil");
  glColorMask = (void (*)(GLboolean, GLboolean, GLboolean, GLboolean))dlsym(v, "glColorMask");
  glCompileShader = (void (*)(GLuint))dlsym(v, "glCompileShader");
  glCompressedTexImage2D = (void (*)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void *))dlsym(v, "glCompressedTexImage2D");
  glCompressedTexSubImage2D = (void (*)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void *))dlsym(v, "glCompressedTexSubImage2D");
  glCopyTexImage2D = (void (*)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint))dlsym(v, "glCopyTexImage2D");
  glCopyTexSubImage2D = (void (*)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei))dlsym(v, "glCopyTexSubImage2D");
  glCreateProgram = (GLuint(*)(void))dlsym(v, "glCreateProgram");
  glCreateShader = (GLuint(*)(GLenum))dlsym(v, "glCreateShader");
  glCullFace = (void (*)(GLenum))dlsym(v, "glCullFace");
  glDeleteBuffers = (void (*)(GLsizei, const GLuint *))dlsym(v, "glDeleteBuffers");
  glDeleteFramebuffers = (void (*)(GLsizei, const GLuint *))dlsym(v, "glDeleteFramebuffers");
  glDeleteProgram = (void (*)(GLuint))dlsym(v, "glDeleteProgram");
  glDeleteRenderbuffers = (void (*)(GLsizei, const GLuint *))dlsym(v, "glDeleteRenderbuffers");
  glDeleteShader = (void (*)(GLuint))dlsym(v, "glDeleteShader");
  glDeleteTextures = (void (*)(GLsizei, const GLuint *))dlsym(v, "glDeleteTextures");
  glDepthFunc = (void (*)(GLenum))dlsym(v, "glDepthFunc");
  glDepthMask = (void (*)(GLboolean))dlsym(v, "glDepthMask");
  glDepthRangef = (void (*)(GLfloat, GLfloat))dlsym(v, "glDepthRangef");
  glDetachShader = (void (*)(GLuint, GLuint))dlsym(v, "glDetachShader");
  glDisable = (void (*)(GLenum))dlsym(v, "glDisable");
  glDisableVertexAttribArray = (void (*)(GLuint))dlsym(v, "glDisableVertexAttribArray");
  glDrawArrays = (void (*)(GLenum, GLint, GLsizei))dlsym(v, "glDrawArrays");
  glDrawElements = (void (*)(GLenum, GLsizei, GLenum, const void *))dlsym(v, "glDrawElements");
  glEnable = (void (*)(GLenum))dlsym(v, "glEnable");
  glEnableVertexAttribArray = (void (*)(GLuint))dlsym(v, "glEnableVertexAttribArray");
  glFinish = (void (*)(void))dlsym(v, "glFinish");
  glFlush = (void (*)(void))dlsym(v, "glFlush");
  glFramebufferRenderbuffer = (void (*)(GLenum, GLenum, GLenum, GLuint))dlsym(v, "glFramebufferRenderbuffer");
  glFramebufferTexture2D = (void (*)(GLenum, GLenum, GLenum, GLuint, GLint))dlsym(v, "glFramebufferTexture2D");
  glFrontFace = (void (*)(GLenum))dlsym(v, "glFrontFace");
  glGenBuffers = (void (*)(GLsizei, GLuint *))dlsym(v, "glGenBuffers");
  glGenerateMipmap = (void (*)(GLenum))dlsym(v, "glGenerateMipmap");
  glGenFramebuffers = (void (*)(GLsizei, GLuint *))dlsym(v, "glGenFramebuffers");
  glGenRenderbuffers = (void (*)(GLsizei, GLuint *))dlsym(v, "glGenRenderbuffers");
  glGenTextures = (void (*)(GLsizei, GLuint *))dlsym(v, "glGenTextures");
  glGetActiveAttrib = (void (*)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *))dlsym(v, "glGetActiveAttrib");
  glGetActiveUniform = (void (*)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *))dlsym(v, "glGetActiveUniform");
  glGetAttachedShaders = (void (*)(GLuint, GLsizei, GLsizei *, GLuint *))dlsym(v, "glGetAttachedShaders");
  glGetAttribLocation = (GLint(*)(GLuint, const GLchar *))dlsym(v, "glGetAttribLocation");
  glGetBooleanv = (void (*)(GLenum, GLboolean *))dlsym(v, "glGetBooleanv");
  glGetBufferParameteriv = (void (*)(GLenum, GLenum, GLint *))dlsym(v, "glGetBufferParameteriv");
  glGetError = (GLenum(*)(void))dlsym(v, "glGetError");
  glGetFloatv = (void (*)(GLenum, GLfloat *))dlsym(v, "glGetFloatv");
  glGetFramebufferAttachmentParameteriv = (void (*)(GLenum, GLenum, GLenum, GLint *))dlsym(v, "glGetFramebufferAttachmentParameteriv");
  glGetIntegerv = (void (*)(GLenum, GLint *))dlsym(v, "glGetIntegerv");
  glGetProgramiv = (void (*)(GLuint, GLenum, GLint *))dlsym(v, "glGetProgramiv");
  glGetProgramInfoLog = (void (*)(GLuint, GLsizei, GLsizei *, GLchar *))dlsym(v, "glGetProgramInfoLog");
  glGetRenderbufferParameteriv = (void (*)(GLenum, GLenum, GLint *))dlsym(v, "glGetRenderbufferParameteriv");
  glGetShaderiv = (void (*)(GLuint, GLenum, GLint *))dlsym(v, "glGetShaderiv");
  glGetShaderInfoLog = (void (*)(GLuint, GLsizei, GLsizei *, GLchar *))dlsym(v, "glGetShaderInfoLog");
  glGetShaderPrecisionFormat = (void (*)(GLenum, GLenum, GLint *, GLint *))dlsym(v, "glGetShaderPrecisionFormat");
  glGetShaderSource = (void (*)(GLuint, GLsizei, GLsizei *, GLchar *))dlsym(v, "glGetShaderSource");
  glGetString = (const GLubyte *(*)(GLenum))dlsym(v, "glGetString");
  glGetTexParameterfv = (void (*)(GLenum, GLenum, GLfloat *))dlsym(v, "glGetTexParameterfv");
  glGetTexParameteriv = (void (*)(GLenum, GLenum, GLint *))dlsym(v, "glGetTexParameteriv");
  glGetUniformLocation = (GLint(*)(GLuint, const GLchar *))dlsym(v, "glGetUniformLocation");
  glGetUniformfv = (void (*)(GLuint, GLint, GLfloat *))dlsym(v, "glGetUniformfv");
  glGetUniformiv = (void (*)(GLuint, GLint, GLint *))dlsym(v, "glGetUniformiv");
  glGetVertexAttribfv = (void (*)(GLuint, GLenum, GLfloat *))dlsym(v, "glGetVertexAttribfv");
  glGetVertexAttribiv = (void (*)(GLuint, GLenum, GLint *))dlsym(v, "glGetVertexAttribiv");
  glGetVertexAttribPointerv = (void (*)(GLuint, GLenum, void **))dlsym(v, "glGetVertexAttribPointerv");
  glHint = (void (*)(GLenum, GLenum))dlsym(v, "glHint");
  glIsBuffer = (GLboolean(*)(GLuint))dlsym(v, "glIsBuffer");
  glIsEnabled = (GLboolean(*)(GLenum))dlsym(v, "glIsEnabled");
  glIsFramebuffer = (GLboolean(*)(GLuint))dlsym(v, "glIsFramebuffer");
  glIsProgram = (GLboolean(*)(GLuint))dlsym(v, "glIsProgram");
  glIsRenderbuffer = (GLboolean(*)(GLuint))dlsym(v, "glIsRenderbuffer");
  glIsShader = (GLboolean(*)(GLuint))dlsym(v, "glIsShader");
  glIsTexture = (GLboolean(*)(GLuint))dlsym(v, "glIsTexture");
  glLineWidth = (void (*)(GLfloat))dlsym(v, "glLineWidth");
  glLinkProgram = (void (*)(GLuint))dlsym(v, "glLinkProgram");
  glPixelStorei = (void (*)(GLenum, GLint))dlsym(v, "glPixelStorei");
  glPolygonOffset = (void (*)(GLfloat, GLfloat))dlsym(v, "glPolygonOffset");
  glReadPixels = (void (*)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *))dlsym(v, "glReadPixels");
  glReleaseShaderCompiler = (void (*)(void))dlsym(v, "glReleaseShaderCompiler");
  glRenderbufferStorage = (void (*)(GLenum, GLenum, GLsizei, GLsizei))dlsym(v, "glRenderbufferStorage");
  glSampleCoverage = (void (*)(GLfloat, GLboolean))dlsym(v, "glSampleCoverage");
  glScissor = (void (*)(GLint, GLint, GLsizei, GLsizei))dlsym(v, "glScissor");
  glShaderBinary = (void (*)(GLsizei, const GLuint *, GLenum, const void *, GLsizei))dlsym(v, "glShaderBinary");
  glShaderSource = (void (*)(GLuint, GLsizei, const GLchar *const *, const GLint *))dlsym(v, "glShaderSource");
  glStencilFunc = (void (*)(GLenum, GLint, GLuint))dlsym(v, "glStencilFunc");
  glStencilFuncSeparate = (void (*)(GLenum, GLenum, GLint, GLuint))dlsym(v, "glStencilFuncSeparate");
  glStencilMask = (void (*)(GLuint))dlsym(v, "glStencilMask");
  glStencilMaskSeparate = (void (*)(GLenum, GLuint))dlsym(v, "glStencilMaskSeparate");
  glStencilOp = (void (*)(GLenum, GLenum, GLenum))dlsym(v, "glStencilOp");
  glStencilOpSeparate = (void (*)(GLenum, GLenum, GLenum, GLenum))dlsym(v, "glStencilOpSeparate");
  glTexImage2D = (void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *))dlsym(v, "glTexImage2D");
  glTexParameterf = (void (*)(GLenum, GLenum, GLfloat))dlsym(v, "glTexParameterf");
  glTexParameterfv = (void (*)(GLenum, GLenum, const GLfloat *))dlsym(v, "glTexParameterfv");
  glTexParameteri = (void (*)(GLenum, GLenum, GLint))dlsym(v, "glTexParameteri");
  glTexParameteriv = (void (*)(GLenum, GLenum, const GLint *))dlsym(v, "glTexParameteriv");
  glTexSubImage2D = (void (*)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *))dlsym(v, "glTexSubImage2D");
  glUniform1f = (void (*)(GLint, GLfloat))dlsym(v, "glUniform1f");
  glUniform1fv = (void (*)(GLint, GLsizei, const GLfloat *))dlsym(v, "glUniform1fv");
  glUniform1i = (void (*)(GLint, GLint))dlsym(v, "glUniform1i");
  glUniform1iv = (void (*)(GLint, GLsizei, const GLint *))dlsym(v, "glUniform1iv");
  glUniform2f = (void (*)(GLint, GLfloat, GLfloat))dlsym(v, "glUniform2f");
  glUniform2fv = (void (*)(GLint, GLsizei, const GLfloat *))dlsym(v, "glUniform2fv");
  glUniform2i = (void (*)(GLint, GLint, GLint))dlsym(v, "glUniform2i");
  glUniform2iv = (void (*)(GLint, GLsizei, const GLint *))dlsym(v, "glUniform2iv");
  glUniform3f = (void (*)(GLint, GLfloat, GLfloat, GLfloat))dlsym(v, "glUniform3f");
  glUniform3fv = (void (*)(GLint, GLsizei, const GLfloat *))dlsym(v, "glUniform3fv");
  glUniform3i = (void (*)(GLint, GLint, GLint, GLint))dlsym(v, "glUniform3i");
  glUniform3iv = (void (*)(GLint, GLsizei, const GLint *))dlsym(v, "glUniform3iv");
  glUniform4f = (void (*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat))dlsym(v, "glUniform4f");
  glUniform4fv = (void (*)(GLint, GLsizei, const GLfloat *))dlsym(v, "glUniform4fv");
  glUniform4i = (void (*)(GLint, GLint, GLint, GLint, GLint))dlsym(v, "glUniform4i");
  glUniform4iv = (void (*)(GLint, GLsizei, const GLint *))dlsym(v, "glUniform4iv");
  glUniformMatrix2fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat *))dlsym(v, "glUniformMatrix2fv");
  glUniformMatrix3fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat *))dlsym(v, "glUniformMatrix3fv");
  glUniformMatrix4fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat *))dlsym(v, "glUniformMatrix4fv");
  glUseProgram = (void (*)(GLuint))dlsym(v, "glUseProgram");
  glValidateProgram = (void (*)(GLuint))dlsym(v, "glValidateProgram");
  glVertexAttrib1f = (void (*)(GLuint, GLfloat))dlsym(v, "glVertexAttrib1f");
  glVertexAttrib1fv = (void (*)(GLuint, const GLfloat *))dlsym(v, "glVertexAttrib1fv");
  glVertexAttrib2f = (void (*)(GLuint, GLfloat, GLfloat))dlsym(v, "glVertexAttrib2f");
  glVertexAttrib2fv = (void (*)(GLuint, const GLfloat *))dlsym(v, "glVertexAttrib2fv");
  glVertexAttrib3f = (void (*)(GLuint, GLfloat, GLfloat, GLfloat))dlsym(v, "glVertexAttrib3f");
  glVertexAttrib3fv = (void (*)(GLuint, const GLfloat *))dlsym(v, "glVertexAttrib3fv");
  glVertexAttrib4f = (void (*)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat))dlsym(v, "glVertexAttrib4f");
  glVertexAttrib4fv = (void (*)(GLuint, const GLfloat *))dlsym(v, "glVertexAttrib4fv");
  glVertexAttribPointer = (void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *))dlsym(v, "glVertexAttribPointer");
  glViewport = (void (*)(GLint, GLint, GLsizei, GLsizei))dlsym(v, "glViewport");
  glBufferData = (void (*)(GLenum, GLsizeiptr, const void *, GLenum))dlsym(v, "glBufferData");
  glBufferSubData = (void (*)(GLenum, GLintptr, GLsizeiptr, const void *))dlsym(v, "glBufferSubData");

  // --- GL ES 3.0 Core Functions ---
  glReadBuffer = (void (*)(GLenum))dlsym(v, "glReadBuffer");
  glDrawRangeElements = (void (*)(GLenum, GLuint, GLuint, GLsizei, GLenum, const void *))dlsym(v, "glDrawRangeElements");
  glTexImage3D = (void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *))dlsym(v, "glTexImage3D");
  glTexSubImage3D = (void (*)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void *))dlsym(v, "glTexSubImage3D");
  glCopyTexSubImage3D = (void (*)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei))dlsym(v, "glCopyTexSubImage3D");
  glCompressedTexImage3D = (void (*)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void *))dlsym(v, "glCompressedTexImage3D");
  glCompressedTexSubImage3D = (void (*)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void *))dlsym(v, "glCompressedTexSubImage3D");
  glGenQueries = (void (*)(GLsizei, GLuint *))dlsym(v, "glGenQueries");
  glDeleteQueries = (void (*)(GLsizei, const GLuint *))dlsym(v, "glDeleteQueries");
  glIsQuery = (GLboolean(*)(GLuint))dlsym(v, "glIsQuery");
  glBeginQuery = (void (*)(GLenum, GLuint))dlsym(v, "glBeginQuery");
  glEndQuery = (void (*)(GLenum))dlsym(v, "glEndQuery");
  glGetQueryiv = (void (*)(GLenum, GLenum, GLint *))dlsym(v, "glGetQueryiv");
  glGetQueryObjectuiv = (void (*)(GLuint, GLenum, GLuint *))dlsym(v, "glGetQueryObjectuiv");
  glUnmapBuffer = (void (*)(GLenum))dlsym(v, "glUnmapBuffer");
  glMapBufferRange = (const void *(*)(GLenum, GLintptr, GLsizeiptr, GLbitfield))dlsym(v, "glMapBufferRange");
  glFlushMappedBufferRange = (void (*)(GLenum, GLintptr, GLsizeiptr))dlsym(v, "glFlushMappedBufferRange");
  glGenSamplers = (void (*)(GLsizei, GLuint *))dlsym(v, "glGenSamplers");
  glDeleteSamplers = (void (*)(GLsizei, const GLuint *))dlsym(v, "glDeleteSamplers");
  glIsSampler = (GLboolean(*)(GLuint))dlsym(v, "glIsSampler");
  glBindSampler = (void (*)(GLuint, GLuint))dlsym(v, "glBindSampler");
  glSamplerParameterf = (void (*)(GLuint, GLenum, GLfloat))dlsym(v, "glSamplerParameterf");
  glSamplerParameterfv = (void (*)(GLuint, GLenum, const GLfloat *))dlsym(v, "glSamplerParameterfv");
  glSamplerParameteri = (void (*)(GLuint, GLenum, GLint))dlsym(v, "glSamplerParameteri");
  glSamplerParameteriv = (void (*)(GLuint, GLenum, const GLint *))dlsym(v, "glSamplerParameteriv");
  glGetSamplerParameterfv = (void (*)(GLuint, GLenum, GLfloat *))dlsym(v, "glGetSamplerParameterfv");
  glGetSamplerParameteriv = (void (*)(GLuint, GLenum, GLint *))dlsym(v, "glGetSamplerParameteriv");
  glVertexAttribDivisor = (void (*)(GLuint, GLuint))dlsym(v, "glVertexAttribDivisor");
  glBindTransformFeedback = (void (*)(GLenum, GLuint))dlsym(v, "glBindTransformFeedback");
  glDeleteTransformFeedbacks = (void (*)(GLsizei, const GLuint *))dlsym(v, "glDeleteTransformFeedbacks");
  glIsTransformFeedback = (GLboolean(*)(GLuint))dlsym(v, "glIsTransformFeedback");
  glGenTransformFeedbacks = (void (*)(GLsizei, GLuint *))dlsym(v, "glGenTransformFeedbacks");
  glPauseTransformFeedback = (void (*)(void))dlsym(v, "glPauseTransformFeedback");
  glResumeTransformFeedback = (void (*)(void))dlsym(v, "glResumeTransformFeedback");
  glBeginTransformFeedback = (void (*)(GLenum))dlsym(v, "glBeginTransformFeedback");
  glEndTransformFeedback = (void (*)(void))dlsym(v, "glEndTransformFeedback");
  glTransformFeedbackVaryings = (void (*)(GLuint, GLsizei, const GLchar *const *, GLenum))dlsym(v, "glTransformFeedbackVaryings");
  glGetTransformFeedbackVarying = (void (*)(GLuint, GLuint, GLsizei, GLsizei *, GLsizei *, GLenum *, GLchar *))dlsym(v, "glGetTransformFeedbackVarying");
  glBindBufferRange = (void (*)(GLenum, GLuint, GLuint, GLintptr, GLsizeiptr))dlsym(v, "glBindBufferRange");
  glBindBufferBase = (void (*)(GLenum, GLuint, GLuint))dlsym(v, "glBindBufferBase");
  glGetActiveUniformBlockName = (void (*)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *))dlsym(v, "glGetActiveUniformBlockName");
  glGetActiveUniformBlockiv = (void (*)(GLuint, GLuint, GLenum, GLint *))dlsym(v, "glGetActiveUniformBlockiv");
  glGetUniformBlockIndex = (void (*)(GLuint, const GLchar *))dlsym(v, "glGetUniformBlockIndex");
  glUniformBlockBinding = (void (*)(GLuint, GLuint, GLuint))dlsym(v, "glUniformBlockBinding");
  glGetUniformIndices = (void (*)(GLuint, GLsizei, const GLchar *const *, GLuint *))dlsym(v, "glGetUniformIndices");
  glGetActiveUniformsiv = (void (*)(GLuint, GLsizei, const GLuint *, GLenum, GLint *))dlsym(v, "glGetActiveUniformsiv");
  glFenceSync = (GLsync(*)(GLenum, GLbitfield))dlsym(v, "glFenceSync");
  glIsSync = (GLboolean(*)(GLsync))dlsym(v, "glIsSync");
  glDeleteSync = (void (*)(GLsync))dlsym(v, "glDeleteSync");
  glClientWaitSync = (GLenum(*)(GLsync, GLbitfield, GLuint64))dlsym(v, "glClientWaitSync");
  glWaitSync = (void (*)(GLsync, GLbitfield, GLuint64))dlsym(v, "glWaitSync");
  glGetInteger64v = (void (*)(GLenum, GLint64 *))dlsym(v, "glGetInteger64v");
  glGetSynciv = (void (*)(GLsync, GLenum, GLsizei, GLsizei *, GLint *))dlsym(v, "glGetSynciv");
  glCopyBufferSubData = (void (*)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr))dlsym(v, "glCopyBufferSubData");
  glGetBufferParameteri64v = (void (*)(GLenum, GLenum, GLint64 *))dlsym(v, "glGetBufferParameteri64v");
  glBlitFramebuffer = (void (*)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum))dlsym(v, "glBlitFramebuffer");
  glCheckFramebufferStatus = (GLenum(*)(GLenum))dlsym(v, "glCheckFramebufferStatus");
  glDrawArraysInstanced = (void (*)(GLenum, GLint, GLsizei, GLsizei))dlsym(v, "glDrawArraysInstanced");
  glDrawElementsInstanced = (void (*)(GLenum, GLsizei, GLenum, const void *, GLsizei))dlsym(v, "glDrawElementsInstanced");
  glFramebufferTextureLayer = (void (*)(GLenum, GLenum, GLuint, GLint, GLint))dlsym(v, "glFramebufferTextureLayer");
  glGetIntegeri_v = (void (*)(GLenum, GLuint, GLint *))dlsym(v, "glGetIntegeri_v");
  glGetProgramBinary = (void (*)(GLuint, GLsizei, GLsizei *, GLenum *, void *))dlsym(v, "glGetProgramBinary");
  glGetStringi = (const GLubyte *(*)(GLenum, GLuint))dlsym(v, "glGetStringi");
  glProgramBinary = (void (*)(GLuint, GLenum, const void *, GLsizei))dlsym(v, "glProgramBinary");
  glProgramParameteri = (void (*)(GLuint, GLenum, GLint))dlsym(v, "glProgramParameteri");
  glTexStorage2D = (void (*)(GLenum, GLsizei, GLenum, GLsizei, GLsizei))dlsym(v, "glTexStorage2D");
  glTexStorage3D = (void (*)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei))dlsym(v, "glTexStorage3D");
  glUniformMatrix2x3fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat *))dlsym(v, "glUniformMatrix2x3fv");
  glUniformMatrix3x2fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat *))dlsym(v, "glUniformMatrix3x2fv");
  glUniformMatrix2x4fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat *))dlsym(v, "glUniformMatrix2x4fv");
  glUniformMatrix4x2fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat *))dlsym(v, "glUniformMatrix4x2fv");
  glUniformMatrix3x4fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat *))dlsym(v, "glUniformMatrix3x4fv");
  glUniformMatrix4x3fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat *))dlsym(v, "glUniformMatrix4x3fv");
  glVertexAttribI4i = (void (*)(GLuint, GLint, GLint, GLint, GLint))dlsym(v, "glVertexAttribI4i");
  glVertexAttribI4iv = (void (*)(GLuint, const GLint *))dlsym(v, "glVertexAttribI4iv");
  glVertexAttribI4ui = (void (*)(GLuint, GLuint, GLuint, GLuint, GLuint))dlsym(v, "glVertexAttribI4ui");
  glVertexAttribI4uiv = (void (*)(GLuint, const GLuint *))dlsym(v, "glVertexAttribI4uiv");
  glVertexAttribIPointer = (void (*)(GLuint, GLint, GLenum, GLsizei, const void *))dlsym(v, "glVertexAttribIPointer");
  glGetVertexAttribIiv = (void (*)(GLuint, GLenum, GLint *))dlsym(v, "glGetVertexAttribIiv");
  glGetVertexAttribIuiv = (void (*)(GLuint, GLenum, GLuint *))dlsym(v, "glGetVertexAttribIuiv");
  glGetFragDataLocation = (GLint(*)(GLuint, const GLchar *))dlsym(v, "glGetFragDataLocation");
  glGenVertexArrays = (void (*)(GLsizei, GLuint *))dlsym(v, "glGenVertexArrays");
  glBindVertexArray = (void (*)(GLuint))dlsym(v, "glBindVertexArray");
  glDeleteVertexArrays = (void (*)(GLsizei, const GLuint *))dlsym(v, "glDeleteVertexArrays");
  /*
    // --- GL ES 3.1 Core Functions ---
    glDispatchCompute = (void (*)(GLuint, GLuint, GLuint))dlsym(v, "glDispatchCompute");
    glDispatchComputeIndirect = (void (*)(GLintptr))dlsym(v, "glDispatchComputeIndirect");
    glDrawArraysIndirect = (void (*)(GLenum, const void *))dlsym(v, "glDrawArraysIndirect");
    glDrawElementsIndirect = (void (*)(GLenum, GLenum, const void *))dlsym(v, "glDrawElementsIndirect");
    glFramebufferTexture = (void (*)(GLenum, GLenum, GLuint, GLint))dlsym(v, "glFramebufferTexture");
    glGetProgramResourceIndex = (void (*)(GLuint, GLenum, const GLchar *))dlsym(v, "glGetProgramResourceIndex");
    glGetProgramResourceName = (void (*)(GLuint, GLenum, GLuint, GLsizei, GLsizei *, GLchar *))dlsym(v, "glGetProgramResourceName");
    glGetProgramResourceiv = (void (*)(GLuint, GLenum, GLuint, GLsizei, const GLenum *, GLsizei, GLsizei *, GLint *))dlsym(v, "glGetProgramResourceiv");
    glGetProgramResourceLocation = (GLint(*)(GLuint, GLenum, const GLchar *))dlsym(v, "glGetProgramResourceLocation");
    glShaderStorageBlockBinding = (void (*)(GLuint, GLuint, GLuint))dlsym(v, "glShaderStorageBlockBinding");
    glTexBuffer = (void (*)(GLenum, GLenum, GLuint))dlsym(v, "glTexBuffer");
    glTexBufferRange = (void (*)(GLenum, GLenum, GLuint, GLintptr, GLsizeiptr))dlsym(v, "glTexBufferRange");
    glTexImage2DMultisample = (void (*)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean))dlsym(v, "glTexImage2DMultisample");
  */
  return v;
}

// END IMPLEMENTS OPENGL ES

#define MAX_OPENGLES_INFO_TEMP 8192
#define MAX_MSG                512
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

#define check(X)    \
  do {              \
    X;              \
    getErrorGL(#X); \
  } while (0)
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
typedef struct {
  GLuint id;
  uivec2 size;
  void *data;
} opengles_texture;
typedef struct {
  GLuint vao, vbo, ibo;
  int flags;
  size_t vertex_len, index_len;
  mesh_vertex *vertexs;
  mesh_index *indices;
  float trans[16];
} opengles_mesh;
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
  opengles_texture textures[MAX_RESOURCE];
  opengles_mesh meshes[MAX_RESOURCE];
  char *opengles_info_temp;
} *src = NULL;

// core implementation
static const char *opengles_info(void) {
  // TODO: more detail about graphics engine
  if (!src->opengles_info_temp) {
    src->opengles_info_temp = (char *)malloc(MAX_OPENGLES_INFO_TEMP + 1);
    const char *c = (const char *)glGetString(GL_VERSION);
    strcpy(src->opengles_info_temp, c);
    strcat(src->opengles_info_temp, "\n");
    c = (const char *)glGetString(GL_RENDERER);
    strcat(src->opengles_info_temp, c);
    strcat(src->opengles_info_temp, ":");
    c = (const char *)glGetString(GL_VENDOR);
    strcat(src->opengles_info_temp, c);
    strcat(src->opengles_info_temp, ":");
    c = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    strcat(src->opengles_info_temp, c);

    // error when implemented
    /*
    GLint j;
    glGetIntegerv(GL_NUM_EXTENSIONS, &j);
    GLuint i;
    if (j > 30) j = 30;
    for (i = 0; i < j; ++i) {
      c = (const char *)glGetStringi(GL_EXTENSIONS, i);
      strcat(src->opengles_info_temp, c);
      strcat(src->opengles_info_temp, (i % 3) ? "," : "\n");
    }
    */
  }
  return src->opengles_info_temp;
}
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
    if (src->textures[i].size.x == 0)
      break;
    ++i;
  }
  if (i >= MAX_RESOURCE)
    return 0; // reach limit texture total so return default texture
  src->textures[i].size = size;
  src->textures[i].data = data;
  check(glGenTextures(1, &src->textures[i].id));
  check(glBindTexture(GL_TEXTURE_2D, src->textures[i].id));
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
  check(glBindTexture(GL_TEXTURE_2D, src->textures[t].id));
}
static void opengles_setTextureParam(const int param, const int val) {
  check(glTexParameteri(GL_TEXTURE_2D, param, val));
}
static void opengles_deleteTexture(const texture t) {
  check(glDeleteTextures(1, &src->textures[t].id));
  free(src->textures[t].data);
  memset((void *)(src->textures + t), 0, sizeof(opengles_texture));
}
static void opengles_flatRender(const texture t, flat_vertex *v, const size_t l) {
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
  check(glBindTexture(GL_TEXTURE_2D, src->textures[t].id));
  check(glUniform1i(src->ui.uniform_tex, 0));
  check(glBindVertexArray(src->ui.vao));
  check(glBindBuffer(GL_ARRAY_BUFFER, src->ui.vbo));
  check(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * l * sizeof(flat_vertex), (void *)v));
  check(glDrawElements(GL_TRIANGLES, 6 * l, GL_UNSIGNED_SHORT, NULL));
  check(glBindVertexArray(0));
  check(glBindTexture(GL_TEXTURE_2D, 0));
  check(glUseProgram(0));
}
static mesh opengles_genMesh(mesh_vertex *v, const size_t vl, mesh_index *i, const size_t il) {
  mesh m = 0;
  while (m < MAX_RESOURCE) {
    if (src->meshes[m].vertex_len == 0)
      break;
    ++m;
  }
  if (m >= MAX_RESOURCE)
    return -1; // reach limit mesh total so return invalid number
  src->meshes[m].vertex_len = vl;
  src->meshes[m].vertexs = v;
  src->meshes[m].index_len = il;
  src->meshes[m].indices = i;
  matrix4_idt(src->meshes[m].trans);

  check(glGenVertexArrays(1, &src->meshes[m].vao));
  check(glGenBuffers(2, &src->meshes[m].vbo));
  check(glBindVertexArray(src->meshes[m].vao));
  check(glBindBuffer(GL_ARRAY_BUFFER, src->meshes[m].vbo));
  check(glBufferData(GL_ARRAY_BUFFER, vl * sizeof(mesh_vertex), (void *)v, GL_STATIC_DRAW));
  check(glEnableVertexAttribArray(0));
  check(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex), (void *)0));
  check(glEnableVertexAttribArray(1));
  check(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(mesh_vertex), (void *)sizeof(vec3)));
  check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, src->meshes[m].ibo));
  check(glBufferData(GL_ELEMENT_ARRAY_BUFFER, il * sizeof(mesh_index), (void *)i, GL_STATIC_DRAW));
  check(glBindVertexArray(0));
  src->meshes[m].flags |= MESH_VERTEX_DIRTY | MESH_INDEX_DIRTY;
  return m;
}
static void opengles_setMeshTransform(const mesh ms, float *mat) {
  memcpy((void *)src->meshes[ms].trans, mat, 16 * sizeof(float));
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
    opengles_mesh m = src->meshes[ms[i]];
    check(glUniformMatrix4fv(src->world.uniform_transProj, 1, GL_FALSE, m.trans));
    check(glBindVertexArray(m.vao));
    if (m.flags & MESH_VERTEX_DIRTY) {
      check(glBindBuffer(GL_ARRAY_BUFFER, m.vbo));
      check(glBufferSubData(GL_ARRAY_BUFFER, 0, m.vertex_len * sizeof(mesh_vertex), (void *)m.vertexs));
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
  check(glDeleteVertexArrays(1, &src->meshes[m].vao));
  check(glDeleteBuffers(2, &src->meshes[m].vbo));
  free(src->meshes[m].vertexs);
  free(src->meshes[m].indices);
  memset(src->meshes + m, 0, sizeof(opengles_mesh));
}

static void killEGL(const int EGLTermReq) {
  if (!EGLTermReq || !src->display)
    return;
  if (src->textures[0].id) {
    // world draw
    check(glDeleteProgram(src->world.shader));
    // flat draw
    check(glDeleteProgram(src->ui.shader));
    check(glDeleteVertexArrays(1, &src->ui.vao));
    check(glDeleteBuffers(2, &src->ui.vbo));
    // mesh
    for (mesh i = 0; i < MAX_RESOURCE; ++i) {
      if (src->meshes[i].vertex_len == 0)
        continue;
      check(glDeleteVertexArrays(1, &src->meshes[i].vao));
      check(glDeleteBuffers(2, &src->meshes[i].vbo));
    }
    // texture
    for (texture i = 0; i < MAX_RESOURCE; ++i) {
      if (src->textures[i].size.x == 0)
        continue;
      check(glDeleteTextures(1, &src->textures[i].id));
      src->textures[i].id = 0;
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
static void opengles_onWindowCreate(void *w) {
  src->window = (ANativeWindow *)w;
}
static void opengles_onWindowDestroy(void) {
  killEGL(TERM_EGL_SURFACE);
  src->window = NULL;
}
static void opengles_onWindowResizeDisplay(void) {
  src->flags |= RESIZE_DISPLAY;
}
static void opengles_onWindowResize(void) {
  src->flags |= RESIZE_ONLY;
}
static void opengles_resizeInsets(float x, float y, float z, float w) {
  src->insets.x = x;
  src->insets.y = y;
  src->insets.z = z;
  src->insets.w = w;
  src->screenSize.x = src->viewportSize.x - x - z;
  src->screenSize.y = src->viewportSize.y - y - w;
  src->flags |= UI_UPDATE;
}
static int opengles_preRender(void) {
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
        EGL_CONFIG_EVA(EGL_SAMPLES);
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
    if (!src->textures[0].id) {

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
      {
        const void *tempbuf;
        void *ast;
        size_t tempbufl;
        GLuint vi, fi;
        // flat draw
        {
          check(src->ui.shader = glCreateProgram());
          check(vi = glCreateShader(GL_VERTEX_SHADER));
          ast = global_engine.assetBuffer("shaders/flatdraw.vert", &tempbuf, &tempbufl);
          check(glShaderSource(vi, 1, (const GLchar **)&tempbuf, (const GLint *)&tempbufl));
          global_engine.assetClose(ast);
          checkCompileShader(vi);
          check(glAttachShader(src->ui.shader, vi));
          check(fi = glCreateShader(GL_FRAGMENT_SHADER));
          ast = global_engine.assetBuffer("shaders/flatdraw.frag", &tempbuf, &tempbufl);
          check(glShaderSource(fi, 1, (const GLchar **)&tempbuf, (const GLint *)&tempbufl));
          global_engine.assetClose(ast);
          checkCompileShader(fi);
          check(glAttachShader(src->ui.shader, fi));
          checkLinkProgram(src->ui.shader);
          check(glDeleteShader(vi));
          check(glDeleteShader(fi));
          check(src->ui.uniform_proj = glGetUniformLocation(src->ui.shader, "u_proj"));
          check(src->ui.uniform_tex = glGetUniformLocation(src->ui.shader, "u_tex"));
          check(glGenVertexArrays(1, &src->ui.vao));
          check(glGenBuffers(2, &src->ui.vbo));
          check(glBindVertexArray(src->ui.vao));
          uint16_t indexs[MAX_UI_DRAW * 6];
          for (uint16_t i = 0, j = 0, k = 0; i < MAX_UI_DRAW; i++, j += 6) {
            indexs[j] = k++;
            indexs[j + 1] = indexs[j + 5] = k++;
            indexs[j + 2] = indexs[j + 4] = k++;
            indexs[j + 3] = k++;
          }
          // 0, 1, 2, 3, 2, 1
          check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, src->ui.ibo));
          check(glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_UI_DRAW * 6 * sizeof(unsigned short), (void *)indexs, GL_STATIC_DRAW));
          check(glBindBuffer(GL_ARRAY_BUFFER, src->ui.vbo));
          check(glBufferData(GL_ARRAY_BUFFER, MAX_UI_DRAW * 4 * sizeof(flat_vertex), NULL, GL_DYNAMIC_DRAW));
          check(glEnableVertexAttribArray(0));
          check(glEnableVertexAttribArray(1));
          check(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(flat_vertex), (void *)0));
          check(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(flat_vertex), (void *)sizeof(vec2)));
        }
        // world draw
        {
          check(src->world.shader = glCreateProgram());
          check(vi = glCreateShader(GL_VERTEX_SHADER));
          ast = global_engine.assetBuffer("shaders/worlddraw.vert", &tempbuf, &tempbufl);
          check(glShaderSource(vi, 1, (const GLchar **)&tempbuf, (const GLint *)&tempbufl));
          global_engine.assetClose(ast);
          checkCompileShader(vi);
          check(glAttachShader(src->world.shader, vi));
          check(fi = glCreateShader(GL_FRAGMENT_SHADER));
          ast = global_engine.assetBuffer("shaders/worlddraw.frag", &tempbuf, &tempbufl);
          check(glShaderSource(fi, 1, (const GLchar **)&tempbuf, (const GLint *)&tempbufl));
          global_engine.assetClose(ast);
          checkCompileShader(fi);
          check(glAttachShader(src->world.shader, fi));
          checkLinkProgram(src->world.shader);
          check(glDeleteShader(vi));
          check(glDeleteShader(fi));
          check(src->world.uniform_proj = glGetUniformLocation(src->world.shader, "worldview_proj"));
          check(src->world.uniform_transProj = glGetUniformLocation(src->world.shader, "trans_proj"));
        }
      }
      // texture
      // start from 0 to validate default texture
      for (texture t = 0; t < MAX_RESOURCE; ++t) {
        if (src->textures[t].size.x == 0)
          continue;
        check(glGenTextures(1, &src->textures[t].id));
        check(glBindTexture(GL_TEXTURE_2D, src->textures[t].id));
        check(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        check(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, src->textures[t].size.x, src->textures[t].size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, src->textures[t].data));
        check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
      }
      check(glBindTexture(GL_TEXTURE_2D, 0));
      // mesh
      for (mesh m = 0; m < MAX_RESOURCE; ++m) {
        if (src->meshes[m].vertex_len == 0)
          continue;
        check(glGenVertexArrays(1, &src->meshes[m].vao));
        check(glGenBuffers(2, &src->meshes[m].vbo));
        check(glBindVertexArray(src->meshes[m].vao));
        check(glBindBuffer(GL_ARRAY_BUFFER, src->meshes[m].vbo));
        check(glBufferData(GL_ARRAY_BUFFER, src->meshes[m].vertex_len * sizeof(mesh_vertex), (void *)src->meshes[m].vertexs, GL_STATIC_DRAW));
        check(glEnableVertexAttribArray(0));
        check(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex), (void *)0));
        check(glEnableVertexAttribArray(1));
        check(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(mesh_vertex), (void *)sizeof(vec3)));
        check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, src->meshes[m].ibo));
        check(glBufferData(GL_ELEMENT_ARRAY_BUFFER, src->meshes[m].index_len * sizeof(mesh_index), (void *)src->meshes[m].indices, GL_STATIC_DRAW));
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
static void opengles_postRender(void) {
  if (!eglSwapBuffers(src->display, src->surface)) {
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
      LOGE("EGL error swapbuffers");
      break;
    }
  }
}
static void opengles_term(void) {
  killEGL(TERM_EGL_DISPLAY);
  dlclose(src->egllib);
  dlclose(src->gleslib);
  // texture
  for (texture i = 0; i < MAX_RESOURCE; ++i) {
    if (src->textures[i].size.x == 0)
      continue;
    free(src->textures[i].data);
  }
  // mesh
  for (mesh i = 0; i < MAX_RESOURCE; ++i) {
    if (src->meshes[i].vertex_len == 0)
      continue;
    free(src->meshes[i].vertexs);
    free(src->meshes[i].indices);
  }
  if (src->opengles_info_temp) {
    free(src->opengles_info_temp);
  }
  free(src);
}

int opengles_init(void) {
  src = (struct androidGraphics *)calloc(1, sizeof(struct androidGraphics));
  // support EGL 1.3 , OpenGLES 3.0
  if (!(src->egllib = loadEGL()) || !(src->gleslib = loadGLES()))
    LOGE("openGLES library error");

  androidGraphics_onWindowCreate = opengles_onWindowCreate;
  androidGraphics_onWindowDestroy = opengles_onWindowDestroy;
  androidGraphics_onWindowResizeDisplay = opengles_onWindowResizeDisplay;
  androidGraphics_onWindowResize = opengles_onWindowResize;
  androidGraphics_resizeInsets = opengles_resizeInsets;
  androidGraphics_preRender = opengles_preRender;
  androidGraphics_postRender = opengles_postRender;
  androidGraphics_term = opengles_term;

  global_engine.engine_graphics_info = opengles_info;
  global_engine.getScreenSize = opengles_getScreenSize;
  global_engine.toScreenCoordinate = opengles_toScreenCoordinate;
  global_engine.clear = opengles_clear;
  global_engine.clearColor = opengles_clearColor;
  global_engine.genTexture = opengles_genTexture;
  global_engine.bindTexture = opengles_bindTexture;
  global_engine.setTextureParam = opengles_setTextureParam;
  global_engine.deleteTexture = opengles_deleteTexture;
  global_engine.flatRender = opengles_flatRender;
  global_engine.genMesh = opengles_genMesh;
  global_engine.setMeshTransform = opengles_setMeshTransform;
  global_engine.meshRender = opengles_meshRender;
  global_engine.deleteMesh = opengles_deleteMesh;

  {
    // add default texture
    src->textures[0].size.x = 1;
    src->textures[0].size.y = 1;
    src->textures[0].data = malloc(4);
    memset(src->textures[0].data, 0xff, 4);
  }
  return 1;
}