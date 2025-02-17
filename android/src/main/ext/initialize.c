#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>
// make opengles lastest possible version
#include <EGL/egl.h>
#include <GLES3/gl32.h> //API 24

#define TERM_EGL_SURFACE 1
#define TERM_EGL_CONTEXT 2
#define TERM_EGL_DISPLAY 4

static ASensorEvent android_input_sensor_event[2];
static ASensorManager *android_input_sensorManager;
static ASensorEventQueue *android_input_sensorEventQueue;
static int android_input_sensor_enabled;

// android funct
void android_input_set_input_queue (ALooper *looper, AInputQueue *i) {
  if (android_input_queue)
    AInputQueue_detachLooper (android_input_queue);
  android_input_queue = i;
  if (android_input_queue)
    AInputQueue_attachLooper (android_input_queue, looper, ALOOPER_POLL_CALLBACK, android_input_process_input, (void *)minput);
}
void android_input_attach_sensor () {
  if (android_input_sensor_enabled) return;
  for (size_t i = 0; i < MAX_SENSOR_COUNT; ++i) {
	  ASensorEventQueue_enableSensor (android_input_sensorEventQueue, android_input_sensor_cache[i].sensor);
	  ASensorEventQueue_setEventRate (android_input_sensorEventQueue, android_input_sensor_cache[i].sensor, SENSOR_EVENT_RATE);
  }
  sensor_enabled = 1;
  process_sensor_event (0, 0, 0);
}
void android_input_detach_sensor () {
  if (!sensor_enabled) return;
  ASensorEventQueue_disableSensor (sensorEventQueue, accelerometerSensor);
  for (size_t i = 0; i < MAX_SENSOR_COUNT; ++i) {
	  ASensorEventQueue_disableSensor (android_input_sensorEventQueue, android_input_sensor_cache[i].sensor);
	  android_input_sensor_cache[i].value = 0;
  }
  sensor_enabled = 0;
}

float android_graphics_cur_safe_insets[4];
static ANativeWindow *window = nullptr;
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
} mgl_data = nullptr;
// self
static inline void killEGL (const unsigned int EGLTermReq) {
  if (!EGLTermReq || !mgl_data.display) return;
  eglMakeCurrent (mgl_data.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  if (mgl_data.surface && (EGLTermReq & 5)) {
    // invalidate Framebuffer, RenderBuffer
    eglDestroySurface (mgl_data.display, mgl_data.surface);
    mgl_data.surface = EGL_NO_SURFACE;
  }
  if (mgl_data.context && (EGLTermReq & 6)) {
    // invalidating gles resources
    // world draw
    glDeleteProgram (mgl_data.world_shader);
    // flat draw
    glDeleteProgram (mgl_data.ui_shader);
    glDeleteVertexArrays (1, &mgl_data.ui_vao);
    glDeleteBuffers (2, &mgl_data.ui_vbo);
    // mesh
    for (engine_mesh_core *i : mgl_data.managedMesh) {
      glDeleteVertexArrays (1, &i->vao);
      glDeleteBuffers (2, &i->vbo);
    }
    // texture
    for (opengles_texture *i : mgl_data.managedTexture) {
      glDeleteTextures (1, &i->id);
    }
    // reset null texture
    glDeleteTextures (1, &mgl_data.nullTextureId);

    eglDestroyContext (mgl_data.display, mgl_data.context);
    mgl_data.context = EGL_NO_CONTEXT;
  }
  if (EGLTermReq & 4) {
    eglTerminate (mgl_data.display);
    mgl_data.display = EGL_NO_DISPLAY;
  }
}

// android
void onWindowChange (ANativeWindow *w) {
  if (window)
    killEGL (TERM_EGL_SURFACE);
  window = w;
}
void onWindowResizeDisplay () {
  mgl_data.flags |= RESIZE_DISPLAY;
}
void onWindowResize () {
  mgl_data.flags |= RESIZE_ONLY;
}
void preRender () {
  if (!window) return;
  if (!mgl_data.display || !mgl_data.context || !mgl_data.surface) {
    while (!mgl_data.display) {
      // proof
      mgl_data.context = EGL_NO_CONTEXT;
      mgl_data.surface = EGL_NO_SURFACE;
      mgl_data.display = eglGetDisplay (EGL_DEFAULT_DISPLAY);
      eglInitialize (mgl_data.display, nullptr, nullptr);
      mgl_data.eConfig = nullptr;
    }
    while (!mgl_data.eConfig) {
      const EGLint configAttr[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_CONFORMANT, EGL_OPENGL_ES2_BIT, EGL_ALPHA_SIZE, 0, EGL_NONE};
      EGLint temp;
      eglChooseConfig (mgl_data.display, configAttr, nullptr, 0, &temp);
      EGLConfig *configs = (EGLConfig *)alloca (temp * sizeof (EGLConfig));
      EGLConfig *configs_end = configs + temp;
      eglChooseConfig (mgl_data.display, configAttr, configs, temp, &temp);
      mgl_data.eConfig = *configs;
      size_t k = 0, l;
      do {
        EGLConfig &cfg = *configs;
        eglGetConfigAttrib (mgl_data.display, cfg, EGL_BUFFER_SIZE, &temp);
        l = temp;
        eglGetConfigAttrib (mgl_data.display, cfg, EGL_DEPTH_SIZE, &temp);
        l += temp;
        eglGetConfigAttrib (mgl_data.display, cfg, EGL_STENCIL_SIZE, &temp);
        l += temp;
        if (l > k) {
          k = l;
          mgl_data.eConfig = cfg;
        }
      } while (++configs < configs_end);
    }
    bool newCntx = false;
    while (!mgl_data.context) {
      const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
      mgl_data.context = eglCreateContext (mgl_data.display, mgl_data.eConfig, nullptr, ctxAttr);
      newCntx = true;
    }
    while (!mgl_data.surface)
      mgl_data.surface = eglCreateWindowSurface (mgl_data.display, mgl_data.eConfig, window, nullptr);

    eglMakeCurrent (mgl_data.display, mgl_data.surface, mgl_data.surface, mgl_data.context);
    eglQuerySurface (mgl_data.display, mgl_data.surface, EGL_WIDTH, &mgl_data.wWidth);
    eglQuerySurface (mgl_data.display, mgl_data.surface, EGL_HEIGHT, &mgl_data.wHeight);
    if (newCntx) {
      // made root for null texture test
      {
        glGenTextures (1, &mgl_data.nullTextureId);
        unsigned char data[4]{0xff, 0xff, 0xff, 0xff};
        glBindTexture (GL_TEXTURE_2D, mgl_data.nullTextureId);
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
        mgl_data.ui_shader = glCreateProgram ();
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
        glAttachShader (mgl_data.ui_shader, vi);
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
        glAttachShader (mgl_data.ui_shader, fi);
        glLinkProgram (mgl_data.ui_shader);
        glDeleteShader (vi);
        glDeleteShader (fi);
        mgl_data.u_uiProj = glGetUniformLocation (mgl_data.ui_shader, "u_proj");
        mgl_data.u_uiTex = glGetUniformLocation (mgl_data.ui_shader, "u_tex");
        glGenVertexArrays (1, &mgl_data.ui_vao);
        glGenBuffers (2, &mgl_data.ui_vbo);
        glBindVertexArray (mgl_data.ui_vao);
        unsigned short indexs[MAX_UI_DRAW * 6];
        for (unsigned short i = 0, j = 0, k = 0; i < MAX_UI_DRAW; i++, j += 6) {
          indexs[j] = k++;
          indexs[j + 1] = indexs[j + 5] = k++;
          indexs[j + 2] = indexs[j + 4] = k++;
          indexs[j + 3] = k++;
        }
        // 0, 1, 2, 3, 2, 1
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, mgl_data.ui_ibo);
        glBufferData (GL_ELEMENT_ARRAY_BUFFER, MAX_UI_DRAW * 6 * sizeof (unsigned short), (void *)indexs, GL_STATIC_DRAW);
        glBindBuffer (GL_ARRAY_BUFFER, mgl_data.ui_vbo);
        glBufferData (GL_ARRAY_BUFFER, MAX_UI_DRAW * 4 * sizeof (engine_flat_vertex), NULL, GL_DYNAMIC_DRAW);
        glVertexAttribPointer (0, 2, GL_FLOAT, false, sizeof (engine_flat_vertex), (void *)offsetof (engine_flat_vertex, x));
        glVertexAttribPointer (1, 4, GL_UNSIGNED_BYTE, true, sizeof (engine_flat_vertex), (void *)offsetof (engine_flat_vertex, color));
        glVertexAttribPointer (2, 2, GL_FLOAT, false, sizeof (engine_flat_vertex), (void *)offsetof (engine_flat_vertex, u));
        glEnableVertexAttribArray (0);
        glEnableVertexAttribArray (1);
        glEnableVertexAttribArray (2);
      }
      // world draw
      {
        mgl_data.world_shader = glCreateProgram ();
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
        glAttachShader (mgl_data.world_shader, vi);
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
        glAttachShader (mgl_data.world_shader, fi);
        glLinkProgram (mgl_data.world_shader);
        glDeleteShader (vi);
        glDeleteShader (fi);
        mgl_data.u_worldProj = glGetUniformLocation (mgl_data.world_shader, "worldview_proj");
        mgl_data.u_worldTransProj = glGetUniformLocation (mgl_data.world_shader, "trans_proj");
      }
      // mesh
      for (engine_mesh_core *i : mgl_data.managedMesh) {
        glGenVertexArrays (1, &i->vao);
        glGenBuffers (2, &i->vbo);
        glBindVertexArray (i->vao);
        glBindBuffer (GL_ARRAY_BUFFER, i->vbo);
        glBufferData (GL_ARRAY_BUFFER, i->vertex_len * sizeof (engine_mesh_core_data), (void *)i->vertex, GL_STATIC_DRAW);
        glEnableVertexAttribArray (0);
        glVertexAttribPointer (0, 3, GL_FLOAT, false, sizeof (engine_mesh_core_data), (void *)offsetof (engine_mesh_core_data, pos));
        glEnableVertexAttribArray (1);
        glVertexAttribPointer (1, 4, GL_UNSIGNED_BYTE, true, sizeof (engine_mesh_core_data), (void *)offsetof (engine_mesh_core_data, color));
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, i->ibo);
        glBufferData (GL_ELEMENT_ARRAY_BUFFER, i->index_len * sizeof (unsigned short), (void *)i->index, GL_STATIC_DRAW);
      }
      glBindVertexArray (0);
      // texture
      for (opengles_texture *i : mgl_data.managedTexture) {
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
    glViewport (0, 0, mgl_data.wWidth, mgl_data.wHeight);
    mgl_data.flags |= RESIZE_ONLY;
    mgl_data.flags &= ~RESIZE_DISPLAY;
  } else if (mgl_data.flags & RESIZE_DISPLAY) {
    eglMakeCurrent (mgl_data.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglMakeCurrent (mgl_data.display, mgl_data.surface, mgl_data.surface, mgl_data.context);
    eglQuerySurface (mgl_data.display, mgl_data.surface, EGL_WIDTH, &mgl_data.wWidth);
    eglQuerySurface (mgl_data.display, mgl_data.surface, EGL_HEIGHT, &mgl_data.wHeight);
    mgl_data.flags |= RESIZE_ONLY;
    mgl_data.flags &= ~RESIZE_DISPLAY;
  }
  if (mgl_data.flags & RESIZE_ONLY) {
    const float fixedWidth = mgl_data.wWidth;
    const float fixedHeight = mgl_data.wHeight;

    mgl_data.uiProj[0] = mgl_data.worldProj[0] = 2.f / fixedWidth;
    mgl_data.uiProj[5] = mgl_data.worldProj[5] = 2.f / fixedHeight;
    // ui safe insets update
    mgl_data.uiProj[12] = (2.0f * android_graphics_cur_safe_insets[0] / fixedWidth) - 1.0f;
    mgl_data.uiProj[13] = (2.0f * android_graphics_cur_safe_insets[3] / fixedHeight) - 1.0f;
    mgl_data.game_width = fixedWidth - android_graphics_cur_safe_insets[0] - android_graphics_cur_safe_insets[2];
    mgl_data.game_height = fixedHeight - android_graphics_cur_safe_insets[1] - android_graphics_cur_safe_insets[3];

    mgl_data.flags |= PROJ_UI | PROJ_WORLD;
    mgl_data.flags &= ~RESIZE_ONLY;
  }
}
void postRender (int isDestroy) {
  unsigned int EGLTermReq = (isDestroy) ? TERM_EGL_DISPLAY : 0;
  if (!eglSwapBuffers (mgl_data.display, mgl_data.surface)) {
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
// set engine
void init_engine (AAssetManager *UNUSED(mngr), int UNUSED(sdk), ALooper *looper) {
  
}
// unset engine
void term_engine () {
  android_input_detach_sensor ();
  if (android_input_queue)
    AInputQueue_detachLooper (android_input_queue);
  
}