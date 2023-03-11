#include <algorithm>
#include <initializer_list>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cassert>
#include <unistd.h>
#include <unordered_set>
#include <unordered_map>
#include <sys/resource.h>
#include <pthread.h>
#include <poll.h>
#include <sched.h>
#include <string>

#include <EGL/egl.h>
#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>

#include "log.h"
#include "engine.h"
#include "main_game.h"
#include "api_graphics/android_graphics.h"
#include "api_graphics/opengles_graphics.h"
#include "api_graphics/vulkan_graphics.h"

namespace core_set {
	void define_core_set(ALooper*);
	//graphics Android
	void validate();
	void resize_viewport(const int,const int);
	void invalidate();
	//input Android
	void set_input_queue(ALooper*, AInputQueue*);
	void process_input();
	void attach_sensor();
	void process_sensor();
	void detach_sensor();
	
	void undefine_core_set();
}

struct android_app {
    bool destroyed;
    int appCmdState;
    int msgread, msgwrite;
    size_t savedStateSize;
    ANativeActivity* activity;
    AConfiguration* config;
    void* savedState;
    ALooper* looper;
    ANativeWindow* window; //update in mainThread
    AInputQueue* inputQueue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t thread;
}; 

enum {
    APP_CMD_START,
    APP_CMD_RESUME,
    APP_CMD_INPUT_INIT,
    APP_CMD_INIT_WINDOW,
    APP_CMD_GAINED_FOCUS,
    APP_CMD_WINDOW_RESIZED,
    APP_CMD_CONFIG_CHANGED,
    APP_CMD_LOST_FOCUS,
    APP_CMD_LOW_MEMORY,
    APP_CMD_SAVE_STATE,
    APP_CMD_TERM_WINDOW,
    APP_CMD_INPUT_TERM,
    APP_CMD_PAUSE,
    APP_CMD_STOP,
    APP_CMD_DESTROY,
};

static void* android_app_entry(void* param) {
    android_app *app = (android_app*)param;
    app->config = AConfiguration_new();
    AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
    app->looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(app->looper, app->msgread, 1, ALOOPER_EVENT_INPUT, NULL, nullptr);
	  android_graphics *eng = new opengles_graphics;
	  core_set::define_core_set(app->looper);
	  if (app->savedState) {
	      eng->state = *(saved_state*)app->savedState;
	  }
	  while (!eng->destroyed) {
	    switch (ALooper_pollAll(eng->running ? 0 : -1, nullptr, nullptr, nullptr)) {
	      case 2: //input queue
	      	core_set::process_input();
	      	break;
	      case 3: //sensor queue
	      	core_set::process_sensor();
	      	break;
	    	case 1: //android activity queue
					int8_t cmd;
			    if (read(app->msgread, &cmd, sizeof(cmd)) != sizeof(cmd)) break;
					switch (cmd) {
				    case APP_CMD_RESUME:
				    	eng->onResume();
			        pthread_mutex_lock(&app->mutex);
						  if (app->savedState != NULL) {
					      free(app->savedState);
					      app->savedState = NULL;
					      app->savedStateSize = 0;
						  }
						  pthread_mutex_unlock(&app->mutex);
			        break;
			      case APP_CMD_INIT_WINDOW:
			      	eng->onWindowInit(app->window);
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
			        break;
				    case APP_CMD_WINDOW_RESIZED:
				    	eng->needResize();
				    	break;
				    case APP_CMD_GAINED_FOCUS:
				    	core_set::attach_sensor();
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
				      break;
			      case APP_CMD_INPUT_INIT:
		        	core_set::set_input_queue(app->looper, app->inputQueue);
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
				      break;
			      case APP_CMD_INPUT_TERM:
			      	if (app->inputQueue != NULL) {
			        	core_set::set_input_queue(app->looper, NULL);
				        app->inputQueue = NULL;
			      	}
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
			        break;
				    case APP_CMD_LOST_FOCUS:
				    	core_set::detach_sensor();
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
				      break;
			      case APP_CMD_TERM_WINDOW:
			      	eng->onWindowTerm();
			        app->window = NULL;
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
			        break;
			      case APP_CMD_CONFIG_CHANGED:
			        AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
			        break;
			      case APP_CMD_SAVE_STATE:
			        pthread_mutex_lock(&app->mutex);
						  if (app->savedState != NULL) {
					      free(app->savedState);
					      app->savedState = NULL;
					      app->savedStateSize = 0;
						  }
			  			pthread_mutex_unlock(&app->mutex);
				      app->savedState = malloc(sizeof(saved_state));
				      *((saved_state*)app->savedState) = eng->state;
				      app->savedStateSize = sizeof(saved_state);
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
			        break;
			      case APP_CMD_PAUSE:
			      	eng->onPause();
					    pthread_mutex_lock(&app->mutex);
					    app->appCmdState = cmd;
					    pthread_cond_broadcast(&app->cond);
					    pthread_mutex_unlock(&app->mutex);
				      break;
			      case APP_CMD_DESTROY:
				  		eng->onDestroy();
			        break;
			      default:
			      	// ?
			      	break;
			  	}
			  	break;
			  default:
					eng->render();
			  	break;
	    }
	  }
	  core_set::undefine_core_set();
	  delete eng;
    pthread_mutex_lock(&app->mutex);
    if (app->savedState != NULL) {
      free(app->savedState);
      app->savedState = NULL;
      app->savedStateSize = 0;
    }
    if (app->inputQueue != NULL) {
        AInputQueue_detachLooper(app->inputQueue);
        app->inputQueue = NULL;
    }
    AConfiguration_delete(app->config);
    app->destroyed = true;
    pthread_cond_broadcast(&app->cond);
    pthread_mutex_unlock(&app->mutex);
    return NULL;
}
static void android_app_write_cmd(android_app *app, int8_t cmd) {
    if (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd)) {
        LOGI("Failure writing android_app cmd: %s\n", strerror(errno));
    }
}
static void onDestroy(ANativeActivity* activity) {
    android_app *app = (android_app*)activity->instance;
    pthread_mutex_lock(&app->mutex);
    android_app_write_cmd(app, APP_CMD_DESTROY);
    app->destroyed = false;
    while (!app->destroyed) {
    	pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
    close(app->msgread);
    close(app->msgwrite);
    pthread_cond_destroy(&app->cond);
    pthread_mutex_destroy(&app->mutex);
    delete app;
    activity->instance = nullptr;
}
static void onStart(ANativeActivity* activity) {
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_START);
}
static void onResume(ANativeActivity* activity) {
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_RESUME);
}
static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
    android_app *app = (android_app*)activity->instance;
    void* savedState = NULL;
    pthread_mutex_lock(&app->mutex);
    android_app_write_cmd(app, APP_CMD_SAVE_STATE);
    while (app->appCmdState != APP_CMD_SAVE_STATE) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    if (app->savedState != NULL) {
        savedState = app->savedState;
        *outLen = app->savedStateSize;
        app->savedState = NULL;
        app->savedStateSize = 0;
    }
    pthread_mutex_unlock(&app->mutex);
    return savedState;
}
static void onPause(ANativeActivity* activity) {
    android_app *app = (android_app*)activity->instance;
    pthread_mutex_lock(&app->mutex);
    android_app_write_cmd(app, APP_CMD_PAUSE);
    while (app->appCmdState != APP_CMD_PAUSE) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onStop(ANativeActivity* activity) {
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_STOP);
}
static void onConfigurationChanged(ANativeActivity* activity) {
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_CONFIG_CHANGED);
}
static void onLowMemory(ANativeActivity* activity) {
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_LOW_MEMORY);
}
static void onWindowFocusChanged(ANativeActivity* activity, int focused) {
    android_app *app = (android_app*)activity->instance;
    const int8_t foc = focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS;
    pthread_mutex_lock(&app->mutex);
    android_app_write_cmd(app, foc);
    while (app->appCmdState != foc) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
    android_app *app = (android_app*)activity->instance;
    if (app->window != NULL) { //window should null when window create
      pthread_mutex_lock(&app->mutex);
	    android_app_write_cmd(app, APP_CMD_TERM_WINDOW);
	    while (app->appCmdState != APP_CMD_TERM_WINDOW) {
	        pthread_cond_wait(&app->cond, &app->mutex);
	    }
	    pthread_mutex_unlock(&app->mutex);
    }
    app->window = window;
    pthread_mutex_lock(&app->mutex);
    android_app_write_cmd(app, APP_CMD_INIT_WINDOW);
    while (app->appCmdState != APP_CMD_INIT_WINDOW) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onNativeWindowResized(ANativeActivity* activity, ANativeWindow*) {
    android_app_write_cmd((android_app*)activity->instance, APP_CMD_WINDOW_RESIZED);
}
static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow*) {
    android_app *app = (android_app*)activity->instance;
    if(app->window == NULL) return;
    pthread_mutex_lock(&app->mutex);
    android_app_write_cmd(app, APP_CMD_TERM_WINDOW);
    while (app->appCmdState != APP_CMD_TERM_WINDOW) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    android_app *app = (android_app*)activity->instance;
    if (app->inputQueue != NULL) {
	    pthread_mutex_lock(&app->mutex);
    	android_app_write_cmd(app, APP_CMD_INPUT_TERM);
	    while (app->appCmdState != APP_CMD_INPUT_TERM) {
        pthread_cond_wait(&app->cond, &app->mutex);
	    }
	    pthread_mutex_unlock(&app->mutex);
    }
    app->inputQueue = queue;
    pthread_mutex_lock(&app->mutex);
  	android_app_write_cmd(app, APP_CMD_INPUT_INIT);
    while (app->appCmdState != APP_CMD_INPUT_INIT) {
      pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue*) {
    android_app *app = (android_app*)activity->instance;
    if(app->inputQueue == NULL) return;
    pthread_mutex_lock(&app->mutex);
  	android_app_write_cmd(app, APP_CMD_INPUT_TERM);
    while (app->appCmdState != APP_CMD_INPUT_TERM) {
      pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
}
void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
    activity->callbacks->onNativeWindowResized = onNativeWindowResized;
    activity->callbacks->onConfigurationChanged = onConfigurationChanged;
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onLowMemory = onLowMemory;
    activity->callbacks->onSaveInstanceState = onSaveInstanceState;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onDestroy = onDestroy;
    
    android_app* app = new android_app;
    activity->instance = app;
    memset(app, 0, sizeof(android_app));
    app->activity = activity;
    pthread_mutex_init(&app->mutex, NULL);
    pthread_cond_init(&app->cond, NULL);
    if (savedState != NULL) {
        app->savedState = malloc(savedStateSize);
        app->savedStateSize = savedStateSize;
        memcpy(app->savedState, savedState, savedStateSize);
    }
    if (pipe(&app->msgread)) {
        LOGI("could not create pipe: %s", strerror(errno));
    }
    pthread_attr_t attr; 
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&app->thread, &attr, android_app_entry, app);
    pthread_attr_destroy(&attr);
}

//define engine extern
namespace core_set {
	//engine extern graphics definition
	
	// make opengles lastest possible version
	// minimum API version is 21
	#if __ANDROID_API__ >= 24
	#include <GLES3/gl32.h>
	#elif __ANDROID_API__ >= 21
	#include <GLES3/gl31.h>
	#else
	#include <GLES3/gl3.h>
	#endif
	
	#define MAX_UI_DRAW 100
	
	std::unordered_set<engine::texture_core*> managedTexture;
	std::unordered_set<engine::mesh_core*> managedMesh;
	bool valid = false;
	int *temp = 0;
	unsigned int *utemp = 0;
	char *msg = 0;
	float width, height;
	
	struct ui_batch {
		bool dirty_projection;
		int shader;
		int u_projection;
		unsigned int vao, vbo, ibo;
		float ui_projection[16];
	} *ubatch = nullptr;
	struct world_btch {
		bool dirty_worldProj;
		int shader;
		int u_worldProj;
		int u_transProj;
		float worldProj[16];
	} *ws = nullptr;
	
	float getWidth() {
		return width;
	}
	float getHeight() {
		return height;
	}
	void clear(const unsigned int &m) {
		*utemp = 0;
		if (m&1)
			*utemp |= GL_COLOR_BUFFER_BIT;
		if (m&2)
			*utemp |= GL_DEPTH_BUFFER_BIT;
		if (m&4)
			*utemp |= GL_STENCIL_BUFFER_BIT;
		glClear(*utemp);
	}
	void clearcolor(const float &r, const float &g, const float &b, const float &a) {
		glClearColor(r, g, b, a);
	}
	engine::texture_core *gen_texture(const int &width, const int &height, unsigned char *data) {
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
	void bind_texture(engine::texture_core *t) {
		glBindTexture(GL_TEXTURE_2D, t?t->id:0);
	}
	void set_texture_param(const int &param, const int &val) {
		glTexParameteri(GL_TEXTURE_2D, param, val);
	}
	void delete_texture(engine::texture_core *t) {
		std::unordered_set<engine::texture_core*>::iterator it = managedTexture.find(t);
		if (it == managedTexture.end()) return;
		managedTexture.erase(it);
		glDeleteTextures(1, &t->id);
		delete[] t->data;
		delete t;
	}
	void flat_render(engine::flat_vertex *v, unsigned int len) {
		glDisable(GL_DEPTH_TEST);
		glUseProgram(ubatch->shader);
		if (ubatch->dirty_projection) {
			glUniformMatrix4fv(ubatch->u_projection, 1, false, ubatch->ui_projection);
			ubatch->dirty_projection = false;
		}
		glBindVertexArray(ubatch->vao);
		glBindBuffer(GL_ARRAY_BUFFER, ubatch->vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 4*len*sizeof(engine::flat_vertex), (void*)v);
		glDrawElements(GL_TRIANGLES, 6*len, GL_UNSIGNED_SHORT, (void*)0);
		glBindVertexArray(0);
		glUseProgram(0);
	}
	engine::mesh_core *gen_mesh(engine::mesh_core::data *v,unsigned int v_len,unsigned short *i, unsigned int i_len) {
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
		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(engine::mesh_core::data), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(engine::mesh_core::data), (void*)(3*sizeof(float)));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, r->index_len*sizeof(unsigned short), (void*)r->index, GL_STATIC_DRAW);
		glBindVertexArray(0);
		managedMesh.insert(r);
		return r;
	}
	void mesh_render(engine::mesh_core **meshes,const unsigned int &count) {
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
			glDrawElements(GL_TRIANGLES, m->index_len, GL_UNSIGNED_SHORT, (void*)0);
		}
		glBindVertexArray(0);
		glUseProgram(0);
	}
	void delete_mesh(engine::mesh_core *m) {
		std::unordered_set<engine::mesh_core*>::iterator it = managedMesh.find(m);
		if (it == managedMesh.end()) return;
		managedMesh.erase(it);
		glDeleteVertexArrays(1, &m->vao);
		glDeleteBuffers(2, &m->vbo);
		delete[] m->vertex;
		delete[] m->index;
		delete m;
	}
	void resize_viewport(const int w, const int h) {
		glViewport(0, 0, w, h);
		width = (float)w, height = (float)h;
		memset(ws->worldProj, 0, 16 * sizeof(float));
		ws->worldProj[0] = 2.f/width;
		ws->worldProj[5] = 2.f/height;
		ws->worldProj[10] = 0.0005f;
		ws->worldProj[15] = 1;
		ws->dirty_worldProj = true;
		memset(ubatch->ui_projection, 0, 16 * sizeof(float));
		ubatch->ui_projection[0] = 2.f/width;
		ubatch->ui_projection[5] = 2.f/height;
		ubatch->ui_projection[10] = ubatch->ui_projection[15] = 1.f;
		ubatch->ui_projection[12] = ubatch->ui_projection[13] = -1.f;
		ubatch->dirty_projection = true;
	}
	void validate() {
		if (valid) return;
		//validating gles resources
		glDepthRangef(0.0f, 1.0f);
		glClearDepthf(1.0f);
		glDepthFunc(GL_LESS);
		//flat draw
		memset(ubatch,0,sizeof(ui_batch));
		{
			ubatch->shader = glCreateProgram();
			utemp[0] = glCreateShader(GL_VERTEX_SHADER);
			utemp[1] = glCreateShader(GL_FRAGMENT_SHADER);
			const char *vt = "#version 300 es"
				"\n#define LOW lowp"
				"\n#define MED mediump"
				"\n#ifdef GL_FRAGMENT_PRECISION_HIGH"
				"\n    #define HIGH highp"
				"\n#else"
				"\n    #define HIGH mediump"
				"\n#endif"
				"\nuniform mat4 proj;"
				"\nlayout(location = 0) in vec4 a_position;"
				"\nlayout(location = 1) in vec4 a_color;"
				"\nout vec4 v_color;"
				"\nvoid main() {"
				"\n    v_color = a_color;"
				"\n    gl_Position = proj * a_position;"
				"\n}\0";
			glShaderSource(utemp[0], 1, &vt, 0);
			glCompileShader(utemp[0]);
			glAttachShader(ubatch->shader, utemp[0]);
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
				"\n}\0";
			glShaderSource(utemp[1], 1, &ft, 0);
			glCompileShader(utemp[1]);
			glAttachShader(ubatch->shader, utemp[1]);
			glLinkProgram(ubatch->shader);
			glDeleteShader(utemp[0]);
			glDeleteShader(utemp[1]);
			ubatch->u_projection = glGetUniformLocation(ubatch->shader, "proj");
			glGenVertexArrays(1, &ubatch->vao);
			glGenBuffers(2, &ubatch->vbo);
			glBindVertexArray(ubatch->vao);
			unsigned short indexs[MAX_UI_DRAW*6];
			for (size_t i = 0; i < MAX_UI_DRAW; i++) {
				const unsigned short j = i * 6, k = i * 4;
				indexs[j] = k;
				indexs[j+1] = k+1;
				indexs[j+2] = k+2;
				indexs[j+3] = k+3;
				indexs[j+4] = k+2;
				indexs[j+5] = k+1;
			}
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ubatch->ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_UI_DRAW*6*sizeof(unsigned short), (void*)indexs, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, ubatch->vbo);
			glBufferData(GL_ARRAY_BUFFER, MAX_TEXTURE_UI*4*sizeof(engine::flat_vertex), NULL, GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(engine::flat_vertex), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(engine::flat_vertex), (void*)(2 * sizeof(float)));
			glBindVertexArray(0);
		}
		//world draw
		memset(ws,0,sizeof(world_btch));
		{
			ws->shader = glCreateProgram();
			utemp[0] = glCreateShader(GL_VERTEX_SHADER);
			utemp[1] = glCreateShader(GL_FRAGMENT_SHADER);
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
				"\n}\0";
			glShaderSource(utemp[0], 1, &vt, 0);
			glCompileShader(utemp[0]);
			glAttachShader(ws->shader, utemp[0]);
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
				"\n}\0";
			glShaderSource(utemp[1], 1, &ft, 0);
			glCompileShader(utemp[1]);
			glAttachShader(ws->shader, utemp[1]);
			glLinkProgram(ws->shader);
			glDeleteShader(utemp[0]);
			glDeleteShader(utemp[1]);
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
			glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(engine::mesh_core::data), (void*)0);
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
		valid = true;
	}
	void invalidate() {
		//invalidating gles resources
		if (!valid) return;
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
		valid = false;
	}
	
	//engine extern input definition
	#define MAX_TOUCH_POINTERS_COUNT 30
	
	struct touch_pointer {
		bool active;
		float xs, ys;
		float x, y;
	} *input_pointer_cache;
	struct key_event {
		int keyCode;
		enum event {
			KEY_UP,
			KEY_DOWN
		} type;
	};
	std::unordered_set<key_event*> key_events;
	std::unordered_map<std::string, float*> _sensor;
	float *m_accelerometer;
	float *m_gyroscope;
	std::unordered_set<int> key_pressed;
	std::unordered_set<int> just_key_pressed;
	ASensorEvent *s_event;
	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	const ASensor* gyroscopeSensor;
	ASensorEventQueue* sensorEventQueue;
	
	float	*getSensorValue(const char *sensor_name) {	return _sensor[sensor_name]; }
	int getX(unsigned int p = 0) {	return input_pointer_cache[p].x; }
	int getDeltaX(unsigned int p = 0) {	return input_pointer_cache[p].x - input_pointer_cache[p].xs; }
	int getY(unsigned int p = 0) {	return input_pointer_cache[p].y; }
	int getDeltaY(unsigned int p = 0) {	return input_pointer_cache[p].y - input_pointer_cache[p].ys; }
	bool justTouched() {	return false; }
	bool isTouched(unsigned int p = 0) { return input_pointer_cache[p].active; }
	float getPressure(unsigned int p = 0) {
		(void) p;
		return false;
	}
	bool isButtonPressed(int button) {
		(void) button;
		return false;
	}
	bool isButtonJustPressed(int button) {
		(void) button;
		return false;
	}
	bool isKeyPressed(int key) {return key_pressed.find(key) != key_pressed.end(); }
	bool isKeyJustPressed(int key) {return just_key_pressed.find(key) != just_key_pressed.end(); }
	void process_event() {
		if(just_key_pressed.size() > 0) {
			just_key_pressed.clear();
		}
		for (key_event *k : key_events) {
			switch(k->type) {
				case key_event::event::KEY_UP: {
				}
					break;
				case key_event::event::KEY_DOWN:{
					std::unordered_set<int>::iterator key = just_key_pressed.find(k->keyCode);
					if(key != just_key_pressed.end()) {
						just_key_pressed.insert(k->keyCode);
					}
				}
					break;
			}
			delete k;
		}
		key_events.clear();
	}
	AInputQueue *inputQueue = NULL;
	void set_input_queue(ALooper *looper, AInputQueue *i) {
		if (inputQueue)
			AInputQueue_detachLooper(inputQueue);
		inputQueue = i;
		if (inputQueue)
			AInputQueue_attachLooper(inputQueue, looper, 2, NULL, nullptr);
	}
	AInputEvent* i_event;
	void process_input() {
		if (inputQueue == NULL) return;
		if (AInputQueue_getEvent(inputQueue, &i_event) < 0) return;
	  if (AInputQueue_preDispatchEvent(inputQueue, i_event) != 0) return;
	  int32_t handled = 0;
		switch (AInputEvent_getType(i_event)) {
			case AINPUT_EVENT_TYPE_KEY: {
				int32_t keyCode = AKeyEvent_getKeyCode(i_event);
				switch (AKeyEvent_getAction(i_event)) {
					case AKEY_EVENT_ACTION_DOWN: {
						std::unordered_set<int>::iterator key = key_pressed.find(keyCode);
						if(key != key_pressed.end()) {
							key_pressed.insert(keyCode);
						}
						key_events.insert(new key_event{keyCode,key_event::event::KEY_DOWN});
					}
						break;
					case AKEY_EVENT_ACTION_UP: {
						std::unordered_set<int>::iterator key = key_pressed.find(keyCode);
						if(key != key_pressed.end()) {
							key_pressed.erase(key);
						}
						key_events.insert(new key_event{keyCode,key_event::event::KEY_UP});
					}
						break;
					case AKEY_EVENT_ACTION_MULTIPLE:
						break;
				}
			}
				break;
			case AINPUT_EVENT_TYPE_MOTION: {
				const int32_t motion = AMotionEvent_getAction(i_event);
				switch(motion&AMOTION_EVENT_ACTION_MASK) {
			    case AMOTION_EVENT_ACTION_POINTER_DOWN:
			    case AMOTION_EVENT_ACTION_DOWN:
						if (AMotionEvent_getEdgeFlags(i_event) != 0)
							break;
			    {
						const int8_t pointer_index = (motion&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
						if (pointer_index >= MAX_TOUCH_POINTERS_COUNT)
							break;
						touch_pointer &ip = input_pointer_cache[pointer_index];
			    	ip.active = true;
		        ip.xs = ip.x = AMotionEvent_getX(i_event, pointer_index);
		        ip.ys = ip.y = AMotionEvent_getY(i_event, pointer_index);
			    }
			    	break;
			    case AMOTION_EVENT_ACTION_MOVE:
			    	for (size_t i = 0, j = AMotionEvent_getPointerCount(i_event); (i<j) && (i < MAX_TOUCH_POINTERS_COUNT); i++) {
							touch_pointer &ip = input_pointer_cache[i];
							if (!ip.active) continue;
			        ip.x = AMotionEvent_getX(i_event, i);
			        ip.y = AMotionEvent_getY(i_event, i);
			    	}
			    	break;
			    case AMOTION_EVENT_ACTION_POINTER_UP:
			    case AMOTION_EVENT_ACTION_UP:
			    case AMOTION_EVENT_ACTION_OUTSIDE:
			    {
						const int8_t pointer_index = (motion&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
						if (pointer_index >= MAX_TOUCH_POINTERS_COUNT)
							break;
						touch_pointer &ip = input_pointer_cache[pointer_index];
						if (!ip.active) break;
			    	ip.active = false;
		        ip.x = AMotionEvent_getX(i_event, pointer_index);
		        ip.y = AMotionEvent_getY(i_event, pointer_index);
			    }
			    	break;
			    case AMOTION_EVENT_ACTION_CANCEL:
			    	memset(input_pointer_cache, 0, MAX_TOUCH_POINTERS_COUNT*sizeof(touch_pointer));
			    	break;
			    case AMOTION_EVENT_ACTION_SCROLL:
			    case AMOTION_EVENT_ACTION_HOVER_ENTER:
			    case AMOTION_EVENT_ACTION_HOVER_MOVE:
			    case AMOTION_EVENT_ACTION_HOVER_EXIT:
			    	break;
				}
		    handled = 1;
			}
		    break;
		}
	  AInputQueue_finishEvent(inputQueue, i_event, handled);
	}
	bool sensor_enabled = false;
	void attach_sensor() {
		if (sensor_enabled) return;
	  ASensorEventQueue_enableSensor(sensorEventQueue,accelerometerSensor);
	  ASensorEventQueue_setEventRate(sensorEventQueue,accelerometerSensor,50000/3);
	  ASensorEventQueue_enableSensor(sensorEventQueue,gyroscopeSensor);
	  ASensorEventQueue_setEventRate(sensorEventQueue,gyroscopeSensor,50000/3);
		sensor_enabled = true;
	}
	void process_sensor() {
		if (!sensor_enabled) return;
		unsigned int i, j;
		float *sensor_temp;
		while ((i = ASensorEventQueue_getEvents(sensorEventQueue,s_event, 2)) > 0) {
			for (j = 0; j < i; j++) {
				ASensorEvent &e = s_event[j];
				switch (e.type) {
					case ASENSOR_TYPE_ACCELEROMETER:
						sensor_temp = _sensor["accelerometer"];
						sensor_temp[0] = e.acceleration.x/2.f + 0.5f;
						sensor_temp[1] = e.acceleration.y/2.f + 0.5f;
						sensor_temp[2] = e.acceleration.z/2.f + 0.5f;
						break;
					case ASENSOR_TYPE_GYROSCOPE:
						sensor_temp = _sensor["gyroscope"];
						sensor_temp[0] = e.acceleration.x/2.f + 0.5f;
						sensor_temp[1] = e.acceleration.y/2.f + 0.5f;
						sensor_temp[2] = e.acceleration.z/2.f + 0.5f;
						break;
					default:
						break;
				}
			}
		}
	}
	void detach_sensor() {
		if (!sensor_enabled) return;
		for (auto i = _sensor.begin(); i != _sensor.end(); i++)
			memset(i->second, 0, 3*sizeof(float));
	  ASensorEventQueue_disableSensor(sensorEventQueue, accelerometerSensor);
	  ASensorEventQueue_disableSensor(sensorEventQueue, gyroscopeSensor);
		sensor_enabled = false;
	}
	
	void define_core_set(ALooper *looper) {
		//input
		_sensor.emplace("accelerometer", new float[3]{});
		_sensor.emplace("gyroscope", new float[3]{});
		input_pointer_cache = new touch_pointer[MAX_TOUCH_POINTERS_COUNT]{};
		s_event = new ASensorEvent[2];
	  sensorManager = ASensorManager_getInstance();
	  accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_ACCELEROMETER);
	  gyroscopeSensor = ASensorManager_getDefaultSensor(sensorManager,ASENSOR_TYPE_GYROSCOPE);
		sensorEventQueue = ASensorManager_createEventQueue(sensorManager, looper, 3 , NULL, nullptr);
		
		//graphics
		temp = new int[2];
		utemp = new unsigned int[2];
		ubatch = new ui_batch;
		memset(ubatch,0,sizeof(ui_batch));
		ws = new world_btch;
		memset(ws,0,sizeof(world_btch));
		
	  //graphics
	  engine::getWidth = getWidth;
	  engine::getHeight = getHeight;
	  engine::clear = clear;
	  engine::clearcolor = clearcolor;
	  engine::gen_texture = gen_texture;
	  engine::bind_texture = bind_texture;
	  engine::set_texture_param = set_texture_param;
	  engine::delete_texture = delete_texture;
	  engine::flat_render = flat_render;
	  engine::gen_mesh = gen_mesh;
	  engine::mesh_render = mesh_render;
	  engine::delete_mesh = delete_mesh;
	  //input
	  engine::getSensorValue = getSensorValue;
	  engine::getX = getX;
	  engine::getDeltaX = getDeltaX;
	  engine::getY = getY;
	  engine::getDeltaY = getDeltaY;
	  engine::justTouched = justTouched;
	  engine::isTouched = isTouched;
	  engine::getPressure = getPressure;
	  engine::isButtonPressed = isButtonPressed;
	  engine::isButtonJustPressed = isButtonJustPressed;
	  engine::isKeyPressed = isKeyPressed;
	  engine::isKeyJustPressed = isKeyJustPressed;
	  engine::process_event = process_event;
	
	}
	void undefine_core_set() {
		//graphics
		invalidate();
		managedTexture.clear();
		managedMesh.clear();
		delete ubatch;
		delete ws;
		delete[] temp;
		delete[] utemp;
		//input
		detach_sensor();
		set_input_queue(NULL, NULL);
		for (auto i = _sensor.begin(); i != _sensor.end(); i++)
			delete[] i->second;
		_sensor.clear();
		delete[] input_pointer_cache;
		delete[] s_event;
		key_pressed.clear();
		
		//unused engine anymore
		
	}
}





