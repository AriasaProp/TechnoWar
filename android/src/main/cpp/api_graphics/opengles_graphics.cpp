#include "opengles_graphics.hpp"
#include <cassert>
#include <cstddef>
// make opengles lastest possible version
#include <GLES3/gl32.h> //API 24

struct ui_batch {
	bool dirty_projection;
	GLint shader, u_projection;
	GLuint vao, vbo, ibo;
	float ui_projection[16];
};
struct world_batch {
	bool dirty_worldProj;
	GLint shader, u_worldProj, u_transProj;
	float worldProj[16];
};

Main *m_Main = nullptr;
static inline void resize_viewport(const int,const int);
ui_batch *ubatch;
world_batch *ws;

float opengles_graphics::getWidth() { return (float)width; }
float opengles_graphics::getHeight() { return (float)height; }

void opengles_graphics::onResume() {
	resume = true;
	running = true;
}
void opengles_graphics::onWindowInit(ANativeWindow *w){
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
  	bool newCntx = false;
  	if (!context) {
  		const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  		context = eglCreateContext(display, eConfig, nullptr, ctxAttr);
  		newCntx = true;
  	}
  	if (!surface) {
			surface = eglCreateWindowSurface(display, eConfig, window, nullptr);
  	}
  	eglMakeCurrent(display, surface, surface, context);
  	eglQuerySurface(display, surface, EGL_WIDTH, &width);
  	eglQuerySurface(display, surface, EGL_HEIGHT, &height);
		resize_viewport(width, height);
  	if (newCntx) {
			//validating gles resources
			glDepthRangef(0.0f, 1.0f);
			glClearDepthf(1.0f);
			glDepthFunc(GL_LESS);
			GLuint vi, fi;
			//flat draw
			{
				ubatch->shader = glCreateProgram();
				vi = glCreateShader(GL_VERTEX_SHADER);
				const char *vt = "#version 320 es"
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
					"\nout vec4 v_color;"
					"\nvoid main() {"
					"\n    v_color = a_color;"
					"\n    gl_Position = proj * a_position;"
					"\n}";
				glShaderSource(vi, 1, &vt, 0);
				glCompileShader(vi);
				glAttachShader(ubatch->shader, vi);
				fi = glCreateShader(GL_FRAGMENT_SHADER);
				const char *ft = "#version 320 es"
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
				glShaderSource(fi, 1, &ft, 0);
				glCompileShader(fi);
				glAttachShader(ubatch->shader, fi);
				glLinkProgram(ubatch->shader);
				glDeleteShader(vi);
				glDeleteShader(fi);
				ubatch->u_projection = glGetUniformLocation(ubatch->shader, "u_proj");
				glGenVertexArrays(1, &ubatch->vao);
				glGenBuffers(2, &ubatch->vbo);
				glBindVertexArray(ubatch->vao);
				unsigned short indexs[MAX_UI_DRAW*6];
				for (unsigned short i = 0, j = 0, k = 0; i < MAX_UI_DRAW; i++, j += 6) {
					indexs[j] = k++;
					indexs[j+1] = indexs[j+5] = k++;
					indexs[j+2] = indexs[j+4] = k++;
					indexs[j+3] = k++;
				}
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ubatch->ibo);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_UI_DRAW*6*sizeof(unsigned short), (void*)indexs, GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, ubatch->vbo);
				glBufferData(GL_ARRAY_BUFFER, MAX_UI_DRAW*4*sizeof(engine::flat_vertex), NULL, GL_DYNAMIC_DRAW);
				glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(engine::flat_vertex), (void*)offsetof(engine::flat_vertex, x));
				glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(engine::flat_vertex), (void*)offsetof(engine::flat_vertex, color));
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glBindVertexArray(0);
			}
			//world draw
			{
				ws->shader = glCreateProgram();
				vi = glCreateShader(GL_VERTEX_SHADER);
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
				glShaderSource(vi, 1, &vt, 0);
				glCompileShader(vi);
				glAttachShader(ws->shader, vi);
				fi = glCreateShader(GL_FRAGMENT_SHADER);
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
				glShaderSource(fi, 1, &ft, 0);
				glCompileShader(fi);
				glAttachShader(ws->shader, fi);
				glLinkProgram(ws->shader);
				glDeleteShader(vi);
				glDeleteShader(fi);
				ws->u_worldProj = glGetUniformLocation(ws->shader, "worldview_proj");
				ws->u_transProj = glGetUniformLocation(ws->shader, "trans_proj");
			}
			//mesh
			for (std::unordered_set<engine::mesh_core*>::iterator i = managedMesh.begin(); i != managedMesh.end(); ++i) {
				glGenVertexArrays(1, &(*i)->vao);
				glGenBuffers(2, &(*i)->vbo);
				glBindVertexArray((*i)->vao);
				glBindBuffer(GL_ARRAY_BUFFER, (*i)->vbo);
				glBufferData(GL_ARRAY_BUFFER, (*i)->vertex_len*sizeof(engine::mesh_core::data), (void*)(*i)->vertex, GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(engine::mesh_core::data), NULL);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(engine::mesh_core::data), (void*)(3*sizeof(float)));
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*i)->ibo);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, (*i)->index_len*sizeof(unsigned short), (void*)(*i)->index, GL_STATIC_DRAW);
			}
			glBindVertexArray(0);
			//texture
			for (std::unordered_set<engine::texture_core*>::iterator i = managedTexture.begin(); i != managedTexture.end(); ++i) {
				glGenTextures(1, &(*i)->id);
				glBindTexture(GL_TEXTURE_2D, (*i)->id);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (*i)->width, (*i)->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)(*i)->data);
			}
			glBindTexture(GL_TEXTURE_2D, 0);
		}
  	if (!m_Main) {
  		m_Main = new Main;
  		resume = false;
  	}
  } else if (resize) {
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglMakeCurrent(display, surface, surface, context);
    int32_t width, height;
		eglQuerySurface(display, surface, EGL_WIDTH, &width);
		eglQuerySurface(display, surface, EGL_HEIGHT, &height);
		resize_viewport(width, height);
	}
	resize = false;
  if (resume) {
  	m_Main->resume();
		resume = false;
  }
  state.angle += .01f;
  if (state.angle > 1) {
      state.angle = 0;
  }
  m_Main->render();
  if (pause) {
  	m_Main->pause();
		pause = false;
  }
  unsigned int EGLTermReq = 0;
  if (destroyed) {
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
	if (EGLTermReq && display) {
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (surface && (EGLTermReq & 5)) {
    	{
    		//invalidate Framebuffer, RenderBuffer
    	}
      eglDestroySurface(display, surface);
    	surface = EGL_NO_SURFACE;
    }
		if (context && (EGLTermReq & 6)) {
			{
				//invalidating gles resources
				//world draw
				glDeleteProgram(ws->shader);
				//flat draw
				glDeleteProgram(ubatch->shader);
				glDeleteVertexArrays(1, &ubatch->vao);
				glDeleteBuffers(2, &ubatch->vbo);
				//mesh
				for (std::unordered_set<engine::mesh_core*>::iterator i = managedMesh.begin(); i != managedMesh.end(); ++i) {
					glDeleteVertexArrays(1, &(*i)->vao);
					glDeleteBuffers(2, &(*i)->vbo);
				}
				//texture
				for (std::unordered_set<engine::texture_core*>::iterator i = managedTexture.begin(); i != managedTexture.end(); ++i) {
					glDeleteTextures(1, &(*i)->id);
				}
			}
    	eglDestroyContext(display, context);
    	context = EGL_NO_CONTEXT;
    }
    if (EGLTermReq & 4) {
  		eglTerminate(display);
    	display = EGL_NO_DISPLAY;
    }
	}
}
void opengles_graphics::onWindowTerm(){
	if (!display) return;
	eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  if (surface) {
    eglDestroySurface(display, surface);
  	surface = EGL_NO_SURFACE;
  }
	window = NULL;
}
void opengles_graphics::onPause() {
	pause = true;
	render();
	running = false;
}
void opengles_graphics::onDestroy() {
	destroyed = true;
	render();
}
void opengles_graphics::clear(const unsigned int &m) {
	GLuint c = 0;
	if (m&1)
		c |= GL_COLOR_BUFFER_BIT;
	if (m&2)
		c |= GL_DEPTH_BUFFER_BIT;
	if (m&4)
		c |= GL_STENCIL_BUFFER_BIT;
	glClear(c);
}
void opengles_graphics::clearcolor(const float &r, const float &g, const float &b, const float &a) {
	glClearColor(r, g, b, a);
}
engine::texture_core *opengles_graphics::gen_texture(const int &width, const int &height, unsigned char *data) {
	engine::texture_core *t = new engine::texture_core;
	glGenTextures(1, &t->id);
	t->width = width;
	t->height = height;
	memcpy(t->data, data, width*height*sizeof(unsigned char));
	glBindTexture(GL_TEXTURE_2D, t->id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)data);
	glBindTexture(GL_TEXTURE_2D, 0);
	managedTexture.insert(t);
	return t;
}
void opengles_graphics::bind_texture(engine::texture_core *t) {
	glBindTexture(GL_TEXTURE_2D, t?t->id:0);
}
void opengles_graphics::set_texture_param(const int &param, const int &val) {
	glTexParameteri(GL_TEXTURE_2D, param, val);
}
void opengles_graphics::delete_texture(engine::texture_core *t) {
	std::unordered_set<engine::texture_core*>::iterator it = managedTexture.find(t);
	if (it == managedTexture.end()) return;
	managedTexture.erase(it);
	glDeleteTextures(1, &t->id);
	delete[] t->data;
	delete t;
}
void opengles_graphics::flat_render(engine::flat_vertex *v, unsigned int len) {
	glDisable(GL_DEPTH_TEST);
	glUseProgram(ubatch->shader);
	if (ubatch->dirty_projection) {
		glUniformMatrix4fv(ubatch->u_projection, 1, false, ubatch->ui_projection);
		ubatch->dirty_projection = false;
	}
	glBindVertexArray(ubatch->vao);
	glBindBuffer(GL_ARRAY_BUFFER, ubatch->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4*len*sizeof(engine::flat_vertex), (void*)v);
	glDrawElements(GL_TRIANGLES, 6*len, GL_UNSIGNED_SHORT, NULL);
	glBindVertexArray(0);
	glUseProgram(0);
}
engine::mesh_core *opengles_graphics::gen_mesh(engine::mesh_core::data *v,unsigned int v_len,unsigned short *i, unsigned int i_len) {
	engine::mesh_core *r = new engine::mesh_core;
	r->vertex_len = v_len;
	r->vertex = new engine::mesh_core::data[v_len];
	memcpy(r->vertex, v, v_len*sizeof(engine::mesh_core::data));
	r->index_len = i_len;
	r->index = new unsigned short[i_len];
	memcpy(r->index, i, i_len*sizeof(unsigned short));
	glGenVertexArrays(1, &r->vao);
	glGenBuffers(2, &r->vbo);
	glBindVertexArray(r->vao);
	glBindBuffer(GL_ARRAY_BUFFER, r->vbo); 
	glBufferData(GL_ARRAY_BUFFER, r->vertex_len*sizeof(engine::mesh_core::data), (void*)r->vertex, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(engine::mesh_core::data), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(engine::mesh_core::data), (void*)(3*sizeof(float)));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, r->index_len*sizeof(unsigned short), (void*)r->index, GL_STATIC_DRAW);
	glBindVertexArray(0);
	managedMesh.insert(r);
	return r;
}
void opengles_graphics::mesh_render(engine::mesh_core **meshes,const unsigned int &count) {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(ws->shader);
	if (ws->dirty_worldProj) {
		glUniformMatrix4fv(ws->u_worldProj, 1, false, ws->worldProj);
		ws->dirty_worldProj = false;
	}
	for (unsigned int i = 0; i < count; ++i) {
		engine::mesh_core *m = *(meshes+i);
		glUniformMatrix4fv(ws->u_transProj, 1, false, m->trans);
		glBindVertexArray(m->vao);
		if (m->dirty_vertex) {
			glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, m->vertex_len*sizeof(engine::mesh_core::data), (void*)m->vertex);
			m->dirty_vertex = false;
		}
		if (m->dirty_index) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m->index_len*sizeof(unsigned short), (void*)m->index);
			m->dirty_index = false;
		}
		glDrawElements(GL_TRIANGLES, m->index_len, GL_UNSIGNED_SHORT, NULL);
	}
	glBindVertexArray(0);
	glUseProgram(0);
}
void opengles_graphics::delete_mesh(engine::mesh_core *m) {
	std::unordered_set<engine::mesh_core*>::iterator it = managedMesh.find(m);
	if (it == managedMesh.end()) return;
	managedMesh.erase(it);
	glDeleteVertexArrays(1, &m->vao);
	glDeleteBuffers(2, &m->vbo);
	delete[] m->vertex;
	delete[] m->index;
	delete m;
}

static inline void resize_viewport(const int w, const int h) {
	glViewport(0, 0, w, h);
	ubatch->ui_projection[0] = ws->worldProj[0] = 2.f/float(w);
	ws->worldProj[5] = ubatch->ui_projection[5] = 2.f/float(h);
	ubatch->dirty_projection = true;
	ws->dirty_worldProj = true;
}

opengles_graphics::opengles_graphics() {
	ubatch = new ui_batch;
	ws = new world_batch;
	memset(ubatch,0,sizeof(ui_batch));
	memset(ws,0,sizeof(world_batch));
	//memset(ubatch->ui_projection, 0, 16*sizeof(float));
	ubatch->ui_projection[10] = 1;
	ubatch->ui_projection[12] = -1;
	ubatch->ui_projection[13] = -1;
	ubatch->ui_projection[15] = 1;
	ws->worldProj[15] = 1;
	ws->worldProj[10] = 0.00001f;
  engine::graph = this;
}

opengles_graphics::~opengles_graphics() {
	managedTexture.clear();
	managedMesh.clear();
	delete ubatch;
	delete ws;
  engine::graph = nullptr;
}

