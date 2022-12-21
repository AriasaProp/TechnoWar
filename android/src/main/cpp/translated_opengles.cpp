#include "translated_opengles.h"

#include <memory> // alloca, malloc, calloc, alloc, etc
#include <cstring> //str.. function
#include <vector>

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

std::vector<texture_core*> managedTexture;
std::vector<mesh_core*> managedMesh;
bool valid = false;
int *temp = 0;
unsigned int *utemp = 0;
char *msg = 0;

struct ui_batch {
	int shader;
	unsigned int vao,vbo;
} *ubatch = nullptr;

struct world_btch {
	bool dirty_worldProj;
	int shader;
	int u_worldProj;
	int u_transProj;
	float worldProj[16];
} *ws = nullptr;
unsigned int validC = 0;
tgf_gles::tgf_gles() {
	temp = new int[2];
	utemp = new unsigned int[2];
	msg = new char[MAX_GL_MSG];
	ubatch = new ui_batch;
	memset(ubatch,0,sizeof(ui_batch));
	ws = new world_btch;
	memset(ws,0,sizeof(world_btch));
	validate();
}
tgf_gles::~tgf_gles() {
	invalidate();
	managedTexture.clear();
	managedMesh.clear();
	delete ubatch;
	delete ws;
	delete[] temp;
	delete[] utemp;
	delete[] msg;
}
const char *tgf_gles::renderer() {
	return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
}
void tgf_gles::clearcolor(const float &r, const float &g, const float &b, const float &a) {
	glClearColor(r, g, b, a);
}
void tgf_gles::clear(const unsigned int &m) {
	glClear(m);
}
void tgf_gles::viewport(const int &x, const int &y, const int &w, const int &h) {
	glViewport(x, y, w, h);
}
texture_core *tgf_gles::gen_texture(const int &width, const int &height, unsigned char *data) {
	texture_core *t = new texture_core;
	glGenTextures(1, &t->id);
	t->width = width;
	t->height = height;
	memcpy(t->data, data, width*height*sizeof(unsigned char));
	glBindTexture(GL_TEXTURE_2D, t->id);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)data);
	glBindTexture(GL_TEXTURE_2D, 0);
	managedTexture.push_back(t);
	return t;
}
void tgf_gles::bind_texture(texture_core *t) {
	glBindTexture(GL_TEXTURE_2D, t?t->id:0);
}
void tgf_gles::set_texture_param(const int &param, const int &val) {
	glTexParameteri(GL_TEXTURE_2D, param, val);
}
void tgf_gles::delete_texture(texture_core *t) {
	glDeleteTextures(1, &t->id);
	std::vector<texture_core*>::iterator it = std::find(managedTexture.begin(), managedTexture.end(), t);
	if (it != managedTexture.end()) {
		managedTexture.erase(it);
	}
	delete[] t->data;
	delete t;
}
mesh_core *tgf_gles::gen_mesh(mesh_core::data *v,unsigned int v_len,unsigned short *i, unsigned int i_len) {
	mesh_core *r = new mesh_core;
	r->vertex_len = v_len;
	r->vertex = new mesh_core::data[v_len];
	memcpy(r->vertex, v, sizeof(r->vertex));
	r->index_len = i_len;
	r->index = new unsigned short[i_len];
	memcpy(r->index, i, sizeof(r->index));
	glGenVertexArrays(1, &r->vao);
	glGenBuffers(2, &r->vbo);
	glBindVertexArray(r->vao);
	glBindBuffer(GL_ARRAY_BUFFER, r->vbo); 
	glBufferData(GL_ARRAY_BUFFER, r->vertex_len*sizeof(mesh_core::data), (void*)r->vertex, GL_STATIC_DRAW);
	r->dirty_vertex = false;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(mesh_core::data), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(mesh_core::data), (void*)(3*sizeof(float)));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, r->index_len*sizeof(unsigned short), (void*)r->index, GL_STATIC_DRAW);
	r->dirty_index = false;
	glBindVertexArray(0);
	managedMesh.push_back(r);
	return r;
}
void tgf_gles::world_mesh(float width, float height) {
	memset(ws->worldProj,0,16*sizeof(float));
	ws->worldProj[0] = 2.f/width;
	ws->worldProj[5] = 2.f/width;
	ws->worldProj[10] = 0.0005f;
	ws->worldProj[15] = 1;
	ws->dirty_worldProj = true;
}
static bool mesh_beginned;
void tgf_gles::begin_mesh() {
	mesh_beginned = true;
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(true);
	glUseProgram(ws->shader);
	if (ws->dirty_worldProj) {
		glUniformMatrix4fv(ws->u_worldProj, 1, false, ws->worldProj);
		ws->dirty_worldProj = false;
	}
}
void tgf_gles::draw_mesh(mesh_core *m) {
	if (!mesh_beginned) return;
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
void tgf_gles::end_mesh() {
	if (!mesh_beginned) return;
	glBindVertexArray(0);
	glUseProgram(0);
	mesh_beginned = false;
	
	//test there to draw UI Batch
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	glUseProgram(ubatch->shader);
	glBindVertexArray(ubatch->vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glUseProgram(0);
}
void tgf_gles::delete_mesh(mesh_core *m) {
	std::vector<mesh_core*>::iterator it = std::find(managedMesh.begin(), managedMesh.end(), m);
	if (it == managedTexture.end()) return;
	managedMesh.erase(it);
	glDeleteVertexArrays(1, &m->vao);
	glDeleteBuffers(2, &m->vbo);
	delete[] m->vertex;
	delete[] m->index;
	delete m;
}
void tgf_gles::validate() {
	if (valid) return;
	//validating gles resources
	//world draw
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
		glCompileShader(utemp[1]);
		glAttachShader(ws->shader, utemp[0]);
		glAttachShader(ws->shader, utemp[1]);
		glLinkProgram(ws->shader);
		glDeleteShader(utemp[0]);
		glDeleteShader(utemp[1]);
		ws->u_worldProj = glGetUniformLocation(ws->shader, "worldview_proj");
		ws->u_transProj = glGetUniformLocation(ws->shader, "trans_proj");
	}
	//flat draw
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
			"\nlayout(location = 0) in vec4 a_position;"
			"\nvoid main() {"
			"\n    gl_Position = a_position;"
			"\n}\0";
		glShaderSource(utemp[0], 1, &vt, 0);
		glCompileShader(utemp[0]);
		const char *ft = "#version 300 es"
			"\n#define LOW lowp"
			"\n#define MED mediump"
			"\n#ifdef GL_FRAGMENT_PRECISION_HIGH"
			"\n    #define HIGH highp"
			"\n#else"
			"\n    #define HIGH mediump"
			"\n#endif"
			"\nprecision MED float;"
			"\nlayout(location = 0) out vec4 fragColor;"
			"\nvoid main() {"
			"\n    fragColor = vec4(1.0);"
			"\n}\0";
		glShaderSource(utemp[1], 1, &ft, 0);
		glCompileShader(utemp[1]);
		glCompileShader(utemp[1]);
		glAttachShader(ubatch->shader, utemp[0]);
		glAttachShader(ubatch->shader, utemp[1]);
		glLinkProgram(ubatch->shader);
		glDeleteShader(utemp[0]);
		glDeleteShader(utemp[1]);
		float v_t[8] = {
			-1, -1,
			-1, 0,
			0, -1,
			0, 0
		};
		glGenVertexArrays(1, &ubatch->vao);
		glGenBuffers(1, &ubatch->vbo);
		glBindVertexArray(ubatch->vao);
		glBindBuffer(GL_ARRAY_BUFFER, ubatch->vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(v_t), (void*)v_t, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), (void*)0);
		glBindVertexArray(0);
	}
	//mesh
	for (std::vector<mesh_core*>::iterator i = managedMesh.begin(); i != managedMesh.end(); i++) {
		glGenVertexArrays(1, &(*i)->vao);
		glGenBuffers(2, &(*i)->vbo);
		glBindVertexArray((*i)->vao);
		glBindBuffer(GL_ARRAY_BUFFER, (*i)->vbo);
		glBufferData(GL_ARRAY_BUFFER, (*i)->vertex_len*sizeof(mesh_core::data), (void*)(*i)->vertex, GL_STATIC_DRAW);
		(*i)->dirty_vertex = false;
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(mesh_core::data), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(mesh_core::data), (void*)(3*sizeof(float)));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*i)->ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (*i)->index_len*sizeof(unsigned short), (void*)(*i)->index, GL_STATIC_DRAW);
		(*i)->dirty_index = false;
		glBindVertexArray(0);
	}
	//texture
	for (std::vector<texture_core*>::iterator i = managedTexture.begin(); i != managedTexture.end(); i++) {
		glGenTextures(1, &(*i)->id);
		glBindTexture(GL_TEXTURE_2D, (*i)->id);
	  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (*i)->width, (*i)->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)(*i)->data);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	
	valid = true;
}
void tgf_gles::invalidate() {
	if (!valid) return;
	//invalidating gles resources
	//world draw
	{
		glDeleteProgram(ws->shader);
	}
	//flat draw
	{
		glDeleteProgram(ubatch->shader);
		glDeleteVertexArrays(1, &ubatch->vao);
		glDeleteBuffers(1, &ubatch->vbo);
	}
	//mesh
	for (std::vector<mesh_core*>::iterator i = managedMesh.begin(); i != managedMesh.end(); i++) {
		glDeleteVertexArrays(1, &(*i)->vao);
		glDeleteBuffers(2, &(*i)->vbo);
	}
	//texture
	for (std::vector<texture_core*>::iterator i = managedTexture.begin(); i != managedTexture.end(); i++) {
		glDeleteTextures(1, &(*i)->id);
	}
	
	valid = false;
}
