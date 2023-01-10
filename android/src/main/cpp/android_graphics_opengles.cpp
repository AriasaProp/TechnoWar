#include "android_graphics_opengles.h"

#include <memory> // alloca, malloc, calloc, alloc, etc
#include <cstring> //str.. function
#include <unordered_set>
#include <algorithm>

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

std::unordered_set<texture_core*> managedTexture;
std::unordered_set<mesh_core*> managedMesh;
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
android_graphics_opengles::android_graphics_opengles() {
	temp = new int[2];
	utemp = new unsigned int[2];
	msg = new char[MAX_GL_MSG];
	ubatch = new ui_batch;
	memset(ubatch,0,sizeof(ui_batch));
	ws = new world_btch;
	memset(ws,0,sizeof(world_btch));
	validate();
}
android_graphics::~android_graphics(){}
android_graphics_opengles::~android_graphics_opengles() {
	invalidate();
	managedTexture.clear();
	managedMesh.clear();
	delete ubatch;
	delete ws;
	delete[] temp;
	delete[] utemp;
	delete[] msg;
}
float android_graphics_opengles::getWidth() {
	return width;
}
float android_graphics_opengles::getHeight() {
	return height;
}
void android_graphics_opengles::clear(const unsigned int &m) {
	*utemp = 0;
	if (m&1)
		*utemp |= GL_COLOR_BUFFER_BIT;
	if (m&2)
		*utemp |= GL_DEPTH_BUFFER_BIT;
	if (m&4)
		*utemp |= GL_STENCIL_BUFFER_BIT;
	glClear(*utemp);
}
void android_graphics_opengles::clearcolor(const float &r, const float &g, const float &b, const float &a) {
	glClearColor(r, g, b, a);
}
texture_core *android_graphics_opengles::gen_texture(const int &width, const int &height, unsigned char *data) {
	texture_core *t = new texture_core;
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
void android_graphics_opengles::bind_texture(texture_core *t) {
	glBindTexture(GL_TEXTURE_2D, t?t->id:0);
}
void android_graphics_opengles::set_texture_param(const int &param, const int &val) {
	glTexParameteri(GL_TEXTURE_2D, param, val);
}
void android_graphics_opengles::delete_texture(texture_core *t) {
	std::unordered_set<texture_core*>::iterator it = managedTexture.find(t);
	if (it == managedTexture.end()) return;
	managedTexture.erase(it);
	glDeleteTextures(1, &t->id);
	delete[] t->data;
	delete t;
}
void android_graphics_opengles::flat_render(flat_vertex *v, unsigned int len) {
	glDisable(GL_DEPTH_TEST);
	glUseProgram(ubatch->shader);
	if (ubatch->dirty_projection) {
		glUniformMatrix4fv(ubatch->u_projection, 1, false, ubatch->ui_projection);
		ubatch->dirty_projection = false;
	}
	glBindVertexArray(ubatch->vao);
	glBindBuffer(GL_ARRAY_BUFFER, ubatch->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4*len*sizeof(flat_vertex), (void*)v);
	glDrawElements(GL_TRIANGLES, 6*len, GL_UNSIGNED_SHORT, (void*)0);
	glBindVertexArray(0);
	glUseProgram(0);
}
mesh_core *android_graphics_opengles::gen_mesh(mesh_core::data *v,unsigned int v_len,unsigned short *i, unsigned int i_len) {
	mesh_core *r = new mesh_core;
	r->vertex_len = v_len;
	r->vertex = new mesh_core::data[v_len];
	memcpy(r->vertex, v, v_len*sizeof(mesh_core::data));
	r->index_len = i_len;
	r->index = new unsigned short[i_len];
	memcpy(r->index, i, i_len*sizeof(unsigned short));
	glGenVertexArrays(1, &r->vao);
	glGenBuffers(2, &r->vbo);
	glBindVertexArray(r->vao);
	glBindBuffer(GL_ARRAY_BUFFER, r->vbo); 
	glBufferData(GL_ARRAY_BUFFER, r->vertex_len*sizeof(mesh_core::data), (void*)r->vertex, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(mesh_core::data), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(mesh_core::data), (void*)(3*sizeof(float)));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, r->index_len*sizeof(unsigned short), (void*)r->index, GL_STATIC_DRAW);
	glBindVertexArray(0);
	managedMesh.insert(r);
	return r;
}
void android_graphics_opengles::mesh_render(mesh_core **meshes,const unsigned int &count) {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(ws->shader);
	if (ws->dirty_worldProj) {
		glUniformMatrix4fv(ws->u_worldProj, 1, false, ws->worldProj);
		ws->dirty_worldProj = false;
	}
	for (unsigned int i = 0; i < count; ++i) {
		mesh_core *m = *(meshes+i);
		glUniformMatrix4fv(ws->u_transProj, 1, false, m->trans);
		glBindVertexArray(m->vao);
		if (m->dirty_vertex) {
			glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, m->vertex_len*sizeof(mesh_core::data), (void*)m->vertex);
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
void android_graphics_opengles::delete_mesh(mesh_core *m) {
	std::unordered_set<mesh_core*>::iterator it = managedMesh.find(m);
	if (it == managedMesh.end()) return;
	managedMesh.erase(it);
	glDeleteVertexArrays(1, &m->vao);
	glDeleteBuffers(2, &m->vbo);
	delete[] m->vertex;
	delete[] m->index;
	delete m;
}
void android_graphics_opengles::resize_viewport(const int w, const int h) {
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
void android_graphics_opengles::validate() {
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
		glBufferData(GL_ARRAY_BUFFER, MAX_TEXTURE_UI*4*sizeof(flat_vertex), NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(flat_vertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(flat_vertex), (void*)(2 * sizeof(float)));
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
	for (std::unordered_set<mesh_core*>::iterator i = managedMesh.begin(); i != managedMesh.end(); ++i) {
		glGenVertexArrays(1, &(*i)->vao);
		glGenBuffers(2, &(*i)->vbo);
		glBindVertexArray((*i)->vao);
		glBindBuffer(GL_ARRAY_BUFFER, (*i)->vbo);
		glBufferData(GL_ARRAY_BUFFER, (*i)->vertex_len*sizeof(mesh_core::data), (void*)(*i)->vertex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(mesh_core::data), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(mesh_core::data), (void*)(3*sizeof(float)));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*i)->ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (*i)->index_len*sizeof(unsigned short), (void*)(*i)->index, GL_STATIC_DRAW);
	}
	glBindVertexArray(0);
	//texture
	for (std::unordered_set<texture_core*>::iterator i = managedTexture.begin(); i != managedTexture.end(); ++i) {
		glGenTextures(1, &(*i)->id);
		glBindTexture(GL_TEXTURE_2D, (*i)->id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (*i)->width, (*i)->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)(*i)->data);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	valid = true;
}
void android_graphics_opengles::invalidate() {
	//invalidating gles resources
	if (!valid) return;
	//world draw
	glDeleteProgram(ws->shader);
	//flat draw
	glDeleteProgram(ubatch->shader);
	glDeleteVertexArrays(1, &ubatch->vao);
	glDeleteBuffers(2, &ubatch->vbo);
	//mesh
	for (std::unordered_set<mesh_core*>::iterator i = managedMesh.begin(); i != managedMesh.end(); ++i) {
		glDeleteVertexArrays(1, &(*i)->vao);
		glDeleteBuffers(2, &(*i)->vbo);
	}
	//texture
	for (std::unordered_set<texture_core*>::iterator i = managedTexture.begin(); i != managedTexture.end(); ++i) {
		glDeleteTextures(1, &(*i)->id);
	}
	valid = false;
}
