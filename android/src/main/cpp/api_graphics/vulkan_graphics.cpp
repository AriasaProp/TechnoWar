#include "vulkan_graphics.h"
/*
#include <cassert>

struct ui_batch {
	bool dirty_projection;
	int shader;
	int u_projection;
	unsigned int vao, vbo, ibo;
	float ui_projection[16];
};
struct world_btch {
	bool dirty_worldProj;
	int shader;
	int u_worldProj;
	int u_transProj;
	float worldProj[16];
};

#include <vulkan/vulkan.hpp>

Main *m_Main = nullptr;
bool vulkan_valid = false;
void validate();
void resize_viewport(const int,const int);
void invalidate();

void vulkan_graphics::onResume() {
	resume = true;
	running = true;
}
void vulkan_graphics::onWindowInit(ANativeWindow *w){
	window = w;
}
void vulkan_graphics::needResize() {
	resize = true;
}
void vulkan_graphics::render() {
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
  	if (!context) {
  		const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  		context = eglCreateContext(display, eConfig, nullptr, ctxAttr);
  	}
  	if (!surface)
			surface = eglCreateWindowSurface(display, eConfig, window, nullptr);
  	eglMakeCurrent(display, surface, surface, context);
    int32_t width, height;
  	eglQuerySurface(display, surface, EGL_WIDTH, &width);
  	eglQuerySurface(display, surface, EGL_HEIGHT, &height);
  	
  	if (vulkan_valid) {
			validate();
  	}
  	if (!m_Main) {
  		m_Main = new Main;
  		m_Main->create();
  		resume = false;
  	}
		resize_viewport(width, height);
		resize = false;
  }
	if (resize) {
		resize = false;
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglMakeCurrent(display, surface, surface, context);
    int32_t width, height;
		eglQuerySurface(display, surface, EGL_WIDTH, &width);
		eglQuerySurface(display, surface, EGL_HEIGHT, &height);
		resize_viewport(width, height);
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
    	invalidate();
    	eglDestroyContext(display, context);
    	context = EGL_NO_CONTEXT;
    }
    if (EGLTermReq & TERM_EGL_DISPLAY) {
  		eglTerminate(display);
    	display = EGL_NO_DISPLAY;
    }
	}
}
void vulkan_graphics::onWindowTerm(){
	if (!display) return;
	eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  if (surface) {
    eglDestroySurface(display, surface);
  	surface = EGL_NO_SURFACE;
  }
	window = NULL;
}
void vulkan_graphics::onPause() {
	pause = true;
	render();
	running = false;
}
void vulkan_graphics::onDestroy() {
	destroyed = true;
	render();
}

#define MAX_UI_DRAW 100
std::unordered_set<engine::texture_core*> managedTexture;
std::unordered_set<engine::mesh_core*> managedMesh;
int *temp;
unsigned int *utemp;
char *msg;
float width, height;
ui_batch *ubatch;
world_btch *ws;

float vulkan_graphics::getWidth() { return width; }
float vulkan_graphics::getHeight() { return height; }
void vulkan_graphics::clear(const unsigned int &m) {
	*utemp = 0;
	if (m&1)
		*utemp |= GL_COLOR_BUFFER_BIT;
	if (m&2)
		*utemp |= GL_DEPTH_BUFFER_BIT;
	if (m&4)
		*utemp |= GL_STENCIL_BUFFER_BIT;
	glClear(*utemp);
}
void vulkan_graphics::clearcolor(const float &r, const float &g, const float &b, const float &a) {
	glClearColor(r, g, b, a);
}
engine::texture_core *vulkan_graphics::gen_texture(const int &width, const int &height, unsigned char *data) {
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
void vulkan_graphics::bind_texture(engine::texture_core *t) {
	glBindTexture(GL_TEXTURE_2D, t?t->id:0);
}
void vulkan_graphics::set_texture_param(const int &param, const int &val) {
	glTexParameteri(GL_TEXTURE_2D, param, val);
}
void vulkan_graphics::delete_texture(engine::texture_core *t) {
	std::unordered_set<engine::texture_core*>::iterator it = managedTexture.find(t);
	if (it == managedTexture.end()) return;
	managedTexture.erase(it);
	glDeleteTextures(1, &t->id);
	delete[] t->data;
	delete t;
}
void vulkan_graphics::flat_render(engine::flat_vertex *v, unsigned int len) {
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
engine::mesh_core *vulkan_graphics::gen_mesh(engine::mesh_core::data *v,unsigned int v_len,unsigned short *i, unsigned int i_len) {
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
void vulkan_graphics::mesh_render(engine::mesh_core **meshes,const unsigned int &count) {
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
void vulkan_graphics::delete_mesh(engine::mesh_core *m) {
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
	if (vulkan_valid) return;
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
	vulkan_valid = true;
}
void invalidate() {
	//invalidating gles resources
	if (!vulkan_valid) return;
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
	vulkan_valid = false;
}

vulkan_graphics::vulkan_graphics() {
	temp = new int[2];
	utemp = new unsigned int[2];
	ubatch = new ui_batch;
	memset(ubatch,0,sizeof(ui_batch));
	ws = new world_btch;
	memset(ws,0,sizeof(world_btch));
	
  engine::graph = this;
}

vulkan_graphics::~vulkan_graphics() {
	invalidate();
	managedTexture.clear();
	managedMesh.clear();
	delete ubatch;
	delete ws;
	delete[] temp;
	delete[] utemp;
  engine::graph = nullptr;
}

bool initialize(android_app* app) {
  // Load Android vulkan and retrieve vulkan API function pointers
  if (!InitVulkan()) {
    LOGE("Vulkan is unavailable, install vulkan and re-start");
    return false;
  }


  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = "tutorial01_load_vulkan",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "tutorial",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_MAKE_VERSION(1, 1, 0),
  };


  // prepare necessary extensions: Vulkan on Android need these to function
  std::vector<const char *> instanceExt, deviceExt;
  instanceExt.push_back("VK_KHR_surface");
  instanceExt.push_back("VK_KHR_android_surface");
  deviceExt.push_back("VK_KHR_swapchain");


  // Create the Vulkan instance
  VkInstanceCreateInfo instanceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = static_cast<uint32_t>(instanceExt.size()),
      .ppEnabledExtensionNames = instanceExt.data(),
  };
  CALL_VK(vkCreateInstance(&instanceCreateInfo, nullptr, &tutorialInstance));


  // if we create a surface, we need the surface extension
  VkAndroidSurfaceCreateInfoKHR createInfo{
      .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .window = app->window};
  CALL_VK(vkCreateAndroidSurfaceKHR(tutorialInstance, &createInfo, nullptr,
                                    &tutorialSurface));


  // Find one GPU to use:
  // On Android, every GPU device is equal -- supporting
  // graphics/compute/present
  // for this sample, we use the very first GPU device found on the system
  uint32_t gpuCount = 0;
  CALL_VK(vkEnumeratePhysicalDevices(tutorialInstance, &gpuCount, nullptr));
  VkPhysicalDevice tmpGpus[gpuCount];
  CALL_VK(vkEnumeratePhysicalDevices(tutorialInstance, &gpuCount, tmpGpus));
  tutorialGpu = tmpGpus[0];  // Pick up the first GPU Device


  // check for vulkan info on this GPU device
  VkPhysicalDeviceProperties gpuProperties;
  vkGetPhysicalDeviceProperties(tutorialGpu, &gpuProperties);
  LOGI("Vulkan Physical Device Name: %s", gpuProperties.deviceName);
  LOGI("Vulkan Physical Device Info: apiVersion: %x \n\t driverVersion: %x",
       gpuProperties.apiVersion, gpuProperties.driverVersion);
  LOGI("API Version Supported: %d.%d.%d",
       VK_VERSION_MAJOR(gpuProperties.apiVersion),
       VK_VERSION_MINOR(gpuProperties.apiVersion),
       VK_VERSION_PATCH(gpuProperties.apiVersion));


  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(tutorialGpu, tutorialSurface,
                                            &surfaceCapabilities);


  LOGI("Vulkan Surface Capabilities:\n");
  LOGI("\timage count: %u - %u\n", surfaceCapabilities.minImageCount,
       surfaceCapabilities.maxImageCount);
  LOGI("\tarray layers: %u\n", surfaceCapabilities.maxImageArrayLayers);
  LOGI("\timage size (now): %dx%d\n", surfaceCapabilities.currentExtent.width,
       surfaceCapabilities.currentExtent.height);
  LOGI("\timage size (extent): %dx%d - %dx%d\n",
       surfaceCapabilities.minImageExtent.width,
       surfaceCapabilities.minImageExtent.height,
       surfaceCapabilities.maxImageExtent.width,
       surfaceCapabilities.maxImageExtent.height);
  LOGI("\tusage: %x\n", surfaceCapabilities.supportedUsageFlags);
  LOGI("\tcurrent transform: %u\n", surfaceCapabilities.currentTransform);
  LOGI("\tallowed transforms: %x\n", surfaceCapabilities.supportedTransforms);
  LOGI("\tcomposite alpha flags: %u\n", surfaceCapabilities.currentTransform);


  // Find a GFX queue family
  uint32_t queueFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(tutorialGpu, &queueFamilyCount, nullptr);
  assert(queueFamilyCount);
  std::vector<VkQueueFamilyProperties>  queueFamilyProperties(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(tutorialGpu, &queueFamilyCount,
                                           queueFamilyProperties.data());


  uint32_t queueFamilyIndex;
  for (queueFamilyIndex=0; queueFamilyIndex < queueFamilyCount;
       queueFamilyIndex++) {
    if (queueFamilyProperties[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      break;
    }
  }
  assert(queueFamilyIndex < queueFamilyCount);


  // Create a logical device from GPU we picked
  float priorities[] = {
      1.0f,
  };
  VkDeviceQueueCreateInfo queueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = queueFamilyIndex,
      .queueCount = 1,
      .pQueuePriorities = priorities,
  };


  VkDeviceCreateInfo deviceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = nullptr,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = static_cast<uint32_t>(deviceExt.size()),
      .ppEnabledExtensionNames = deviceExt.data(),
      .pEnabledFeatures = nullptr,
  };


  CALL_VK(
      vkCreateDevice(tutorialGpu, &deviceCreateInfo, nullptr, &tutorialDevice));
  initialized_ = true;
  return 0;
}


void terminate(void) {
  vkDestroySurfaceKHR(tutorialInstance, tutorialSurface, nullptr);
  vkDestroyDevice(tutorialDevice, nullptr);
  vkDestroyInstance(tutorialInstance, nullptr);


  initialized_ = false;
}
*/