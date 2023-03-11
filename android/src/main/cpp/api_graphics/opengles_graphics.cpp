#include "opengles_graphics.h"

Main *m_Main = nullptr;

void opengles_graphics::Resume() {
	resume = true;
	running = true;
}
void opengles_graphics::WindowInit(ANativeWindow *w){
	window = w;
}
void opengles_graphics::needResize() {
	resize = true;
}
void opengles_graphics::render() {
  if (!window || !running) return;
  if (!display || !context || !surface) {
  	if (!display) {
    	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    	eglInitialize(display, nullptr, nullptr);
    	eConfig = nullptr;
  	}
  	if (!eConfig) {
		  EGLint temp;
		  const EGLint configAttr[] = {
		    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		    EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
		    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		    EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
		    EGL_ALPHA_SIZE, 0,
		    EGL_NONE
		  };
		  eglChooseConfig(display, configAttr, nullptr,0, &temp);
		  assert(temp);
		  EGLConfig *configs = (EGLConfig*) alloca(temp*sizeof(EGLConfig));
		  assert(configs);
		  eglChooseConfig(display, configAttr, configs, temp, &temp);
		  assert(temp);
		  eConfig = configs[0];
		  for (unsigned int i = 0, j = temp, k = 0, l; i < j; i++) {
		    EGLConfig& cfg = configs[i];
		    eglGetConfigAttrib(display, cfg, EGL_BUFFER_SIZE, &temp);
		    l = temp;
		    eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &temp);
		    l += temp;
		    eglGetConfigAttrib(display, cfg, EGL_STENCIL_SIZE, &temp);
		    l += temp;
		    if (l > k) {
		      k = l;
		      eConfig = cfg;
		    }
		  }
  	}
  	bool newCtx = false;
  	if (!context) {
  		newCtx = true;
  		const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  		context = eglCreateContext(display, eConfig, nullptr, ctxAttr);
  	}
  	if (!surface) {
			surface = eglCreateWindowSurface(display, eConfig, window, nullptr);
  	}
  	eglMakeCurrent(display, surface, surface, context);
    int32_t width, height;
  	eglQuerySurface(display, surface, EGL_WIDTH, &width);
  	eglQuerySurface(display, surface, EGL_HEIGHT, &height);
  	
  	if (newCtx) {
			core_set::validate();
  	}
  	if (!m_Main) {
  		m_Main = new Main;
  		m_Main->create();
  		resume = false;
  	}
		core_set::resize_viewport(width, height);
		resize = false;
  }
	if (resize) {
		resize = false;
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglMakeCurrent(display, surface, surface, context);
    int32_t width, height;
		eglQuerySurface(display, surface, EGL_WIDTH, &width);
		eglQuerySurface(display, surface, EGL_HEIGHT, &height);
		core_set::resize_viewport(width, height);
	}
  if (resume) {
  	m_Main->resume();
		resume = false;
  }
  state.angle += .01f;
  if (state.angle > 1) {
      state.angle = 0;
  }
  m_Main->render(1.f/60.f);
  if (pause) {
  	m_Main->pause();
		pause = false;
  }
  unsigned int EGLTermReq = 0;
  if (destroyed) {
  	m_Main->destroy();
  	delete(m_Main);
  	m_Main = nullptr;
  	EGLTermReq |= TERM_EGL_DISPLAY;
  }
	if (!eglSwapBuffers(display, surface)) {
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
	if (EGLTermReq) {
		if (!EGLTermReq || !display) return;
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (surface && (EGLTermReq & (TERM_EGL_SURFACE|TERM_EGL_DISPLAY))) {
      eglDestroySurface(display, surface);
    	surface = EGL_NO_SURFACE;
    }
		if (context && (EGLTermReq & (TERM_EGL_CONTEXT|TERM_EGL_DISPLAY))) {
    	core_set::invalidate();
    	eglDestroyContext(display, context);
    	context = EGL_NO_CONTEXT;
    }
    if (EGLTermReq & TERM_EGL_DISPLAY) {
  		eglTerminate(display);
    	display = EGL_NO_DISPLAY;
    }
	}
}
void opengles_graphics::WindowTerm(){
	if (!display) return;
	eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  if (surface) {
    eglDestroySurface(display, surface);
  	surface = EGL_NO_SURFACE;
  }
	window = NULL;
}
void opengles_graphics::Pause() {
	pause = true;
	render();
	running = false;
}
void opengles_graphics::Destroy() {
	destroyed = true;
	render();
}
opengles_graphics::~opengles_graphics() {
	
}
