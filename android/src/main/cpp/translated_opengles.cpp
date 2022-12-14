#include "translated_opengles.h"

#include <memory> // alloca, malloc, calloc, alloc, etc
#include <cstring> //str.. function
#include <vector>

#include "log.h"
// make opengles lastest possible version
// minimum API version is 21
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

std::vector<unsigned int> capabilities;
std::vector<texture_core*> managedTexture;
std::vector<shader_core*> managedShader;
std::vector<mesh_core*> managedMesh;

tgf_gles::tgf_gles() {
	temp = new int[2];
	utemp = new unsigned int[2];
	msg = new char[MAX_GL_MSG];
	validate();
}
tgf_gles::~tgf_gles() {
	invalidate();
	capabilities.clear();
	managedTexture.clear();
	managedShader.clear();
	managedMesh.clear();
	delete[] temp;
	delete[] utemp;
	delete[] msg;
}
const char *tgf_gles::renderer() {
	return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
}
void tgf_gles::clearcolormask(const unsigned int &m, const float &r, const float &g, const float &b, const float &a) {
	glClearColor(r, g, b, a);
	glClear(m);
}
void tgf_gles::viewport(const int &x, const int &y, const int &w, const int &h) {
	glViewport(x, y, w, h);
}
const char *header_glsl =  "#version 300 es\n"
	"#define LOW lowp\n"
	"#define MED mediump\n"
	"#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
	"    #define HIGH highp\n"
	"#else\n"
	"    #define HIGH mediump\n"
	"#endif\n";
shader_core *tgf_gles::gen_shader(const char *v, const char *f) {
	shader_core *o = new shader_core;
	o->id = glCreateProgram();
	o->v = new char[strlen(header_glsl)+strlen(v)];
	strcpy(o->v, header_glsl);
	strcat(o->v, v);
	o->f = new char[strlen(header_glsl)+strlen(f)];
	strcpy(o->f, header_glsl);
	strcat(o->f, f);
	utemp[0] = glCreateShader(GL_VERTEX_SHADER);
	utemp[1] = glCreateShader(GL_FRAGMENT_SHADER);
	try {
		glShaderSource(utemp[0], 1, &o->v, 0);
		glCompileShader(utemp[0]);
		glGetShaderiv(utemp[0], GL_COMPILE_STATUS, temp);
		if (temp[0] == 0) {
			glGetShaderInfoLog(utemp[0], MAX_GL_MSG, 0, msg);
			throw(msg);
		}
		glShaderSource(utemp[1], 1, &o->f, 0);
		glCompileShader(utemp[1]);
		glGetShaderiv(utemp[1], GL_COMPILE_STATUS, temp);
		if (temp[0] == 0){
			glGetShaderInfoLog(utemp[1], MAX_GL_MSG, 0, msg);
			throw(msg);
		}
		glAttachShader(o->id, utemp[0]);
		glAttachShader(o->id, utemp[1]);
		glLinkProgram(o->id);
		glGetProgramiv(o->id, GL_LINK_STATUS, temp);
		if (temp[0] == 0){
			glGetProgramInfoLog(o->id, MAX_GL_MSG, 0, msg);
			throw(msg);
		}
		managedShader.push_back(o);
	} catch (char *e) {
		glDeleteProgram(o->id);
		o->id = 0;
		delete o;
		o = nullptr;
	}
	glDeleteShader(utemp[0]);
	glDeleteShader(utemp[1]);
	return o;
}
void tgf_gles::bind_shader(shader_core *p) {
	glUseProgram(p->id);
}
void tgf_gles::delete_shader(shader_core *p) {
	glDeleteProgram(p->id);
	std::vector<shader_core*>::iterator it = std::find(managedShader.begin(), managedShader.end(), p);
  if(it != managedShader.end())
    managedShader.erase(it);
  delete[] p->v;
  delete[] p->f;
  delete p;
}
int tgf_gles::get_shader_uloc(shader_core *p, const char *name) {
	return glGetUniformLocation(p->id, name);
}
void tgf_gles::u_matrix4fv(const int &loc,const int &count, const bool &trnspose, const float *mtrix) {
	glUniformMatrix4fv(loc, count, trnspose, mtrix);
}

texture_core *tgf_gles::gen_texture(const int &width, const int &height, unsigned char *data) {
	texture_core *t = new texture_core;
	glGenTextures(1, &t->id);
	t->width = width;
	t->height = height;
	memcpy(t->data, data, width*height*sizeof(unsigned char));
	glBindTexture(TGF_TEXTURE_2D, t->id);
  glPixelStorei(TGF_UNPACK_ALIGNMENT, 1);
	glTexImage2D(TGF_TEXTURE_2D, 0, TGF_RGBA8, width, height, 0, TGF_RGBA, TGF_UNSIGNED_BYTE, (void*)data);
	glBindTexture(TGF_TEXTURE_2D, 0);
	managedTexture.push_back(t);
	return t;
}
void tgf_gles::bind_texture(texture_core *t) {
	glBindTexture(TGF_TEXTURE_2D, t?t->id:0);
}
void tgf_gles::set_texture_param(const int &param, const int &val) {
	glTexParameteri(TGF_TEXTURE_2D, param, val);
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
void tgf_gles::gen_buffer(unsigned int &b) {
	glGenBuffers(1, utemp);
	b = utemp[0];
}
void tgf_gles::bind_buffer(unsigned int type, const unsigned int b) {
	glBindBuffer(type, b);
}
void tgf_gles::buffer_data(unsigned int type, long bytes_len, const void *data, unsigned int datatype) {
	glBufferData(type, bytes_len, 0, datatype);
	glBufferSubData(type, 0, bytes_len, data);
}
void tgf_gles::delete_buffer(unsigned int &b) {
	utemp[0] = b;
	glDeleteBuffers(1, utemp);
	b = 0;
}
void tgf_gles::gen_vertex_array(unsigned int &v) {
	glGenVertexArrays(1, utemp);
	v = utemp[0];
}
void tgf_gles::bind_vertex_array(const unsigned int v) {
	glBindVertexArray(v);
}
void tgf_gles::delete_vertex_array(unsigned int &v) {
	utemp[0] = v;
	glDeleteVertexArrays(1, utemp);
	v = 0;
}
	
mesh_core *tgf_gles::gen_mesh(mesh_core::data *v,unsigned int v_len,unsigned short *i, unsigned int i_len) {
	mesh_core *r = new mesh_core;
	r->vertex = new mesh_core::data[v_len];
	r->vertex_len = v_len;
	memcpy(r->vertex, v, v_len*sizeof(mesh_core::data));
	r->index = new unsigned short[i_len];
	r->index_len = i_len;
	memcpy(r->index, i, i_len*sizeof(unsigned short));
	glGenVertexArrays(1, &r->vaoId);
	glGenBuffers(2, &r->vboV);
	glBindVertexArray(r->vaoId);
	glBindBuffer(TGF_ARRAY_BUFFER, r->vboV); 
	glBufferData(TGF_ARRAY_BUFFER, r->vertex_len*sizeof(mesh_core::data), (void*)r->vertex, TGF_STATIC_DRAW);
	glBindBuffer(TGF_ELEMENT_ARRAY_BUFFER, r->vboI);
	glBufferData(TGF_ELEMENT_ARRAY_BUFFER, r->index_len*sizeof(unsigned short), (void*)r->index, TGF_STATIC_DRAW);
	const unsigned int stride = 3 * sizeof(float) + 4 * sizeof(unsigned char);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, TGF_FLOAT, false, stride, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, TGF_UNSIGNED_BYTE, true, stride, (void*)(3*sizeof(float)));
	glBindVertexArray(0);
	managedMesh.push_back(r);
	return r;
}
void tgf_gles::update_mesh(mesh_core *m, mesh_core::data *v, unsigned int v_len, unsigned short *i, unsigned int i_len) {
	glBindVertexArray(m->vaoId);
	if (v) {
		glBindBuffer(TGF_ARRAY_BUFFER, m->vboV);
		memcpy(m->vertex, v, v_len*sizeof(mesh_core::data));
		glBufferSubData(TGF_ARRAY_BUFFER, 0, v_len*sizeof(mesh_core::data), (void*)m->vertex);
	}
	if (i) {
		glBindBuffer(TGF_ELEMENT_ARRAY_BUFFER, m->vboI);
		memcpy(m->index, i, i_len*sizeof(unsigned short));
		glBufferSubData(TGF_ELEMENT_ARRAY_BUFFER, 0, i_len*sizeof(unsigned short), (void*)m->index);
	}
	glBindVertexArray(0);
}
void tgf_gles::draw_mesh(mesh_core *m) {
	glBindVertexArray(m->vaoId);
	glDrawElements(TGF_TRIANGLES, m->index_len, TGF_UNSIGNED_SHORT, (void*)0);
	glBindVertexArray(0);
}
void tgf_gles::delete_mesh(mesh_core *m) {
	glDeleteVertexArrays(1, &m->vaoId);
	glDeleteBuffers(2, &m->vboV);
	delete[] m->vertex;
	delete[] m->index;
	delete m;
}

void tgf_gles::vertex_attrib_pointer(unsigned int loc, int size, unsigned int type, bool normalize, int stride, const void *offset) {
	glVertexAttribPointer(loc, size, type, normalize, stride, offset);
}
void tgf_gles::enable_vertex_attrib_array(const unsigned int loc) {
	glEnableVertexAttribArray(loc);
}
void tgf_gles::draw_elements(int drawType, unsigned int indSize, int inType, const void *offset) {
	glDrawElements(drawType, indSize, inType, offset);
}
//env
void tgf_gles::switch_capability(const unsigned int &cap, const bool enable) {
	std::vector<unsigned int>::iterator it = std::find(capabilities.begin(), capabilities.end(), cap);
	if (enable) {
		if (it == capabilities.end()) {
			glEnable(cap);
			capabilities.push_back(cap);
		}
	} else {
		if (it != capabilities.end()) {
			glDisable(cap);
			capabilities.erase(it);
		}
	}
}
void tgf_gles::cull_face(const unsigned int &face) {
	switch_capability(TGF_CULL_FACE, true);
	glCullFace(face);
}
void tgf_gles::depth_func(const unsigned int &func) {
	switch_capability(TGF_DEPTH_TEST, true);
	glDepthFunc(func);
}
void tgf_gles::depth_rangef(float near,float far) {
	switch_capability(TGF_DEPTH_TEST, true);
	glDepthRangef(near, far);
}
void tgf_gles::depth_mask(const bool m) {
	switch_capability(TGF_DEPTH_TEST, true);
	glDepthMask(m);
}
void tgf_gles::validate() {
	if (valid) return;
	//validating gles resources
	
	//capabilities
	for (std::vector<unsigned int>::iterator i = capabilities.begin(); i != capabilities.end(); i++) {
		glEnable(*i);
	}
	//shader
	for (std::vector<shader_core*>::iterator i = managedShader.begin(); i != managedShader.end(); i++) {
		(*i)->id = glCreateProgram();
		utemp[0] = glCreateShader(GL_VERTEX_SHADER);
		utemp[1] = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(utemp[0], 1, &(*i)->v, 0);
		glCompileShader(utemp[0]);
		glShaderSource(utemp[1], 1, &(*i)->f, 0);
		glCompileShader(utemp[1]);
		glAttachShader((*i)->id, utemp[0]);
		glAttachShader((*i)->id, utemp[1]);
		glLinkProgram((*i)->id);
		glDeleteShader(utemp[0]);
		glDeleteShader(utemp[1]);
	}
	//mesh
	for (std::vector<mesh_core*>::iterator i = managedMesh.begin(); i != managedMesh.end(); i++) {
		mesh_core *r = *i;
		glGenVertexArrays(1, &r->vaoId);
		glGenBuffers(2, &r->vboV);
		glBindVertexArray(r->vaoId);
		const unsigned int stride = 3 * sizeof(float) + 4 * sizeof(unsigned char);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, TGF_FLOAT, false, stride, (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, TGF_UNSIGNED_BYTE, true, stride, (void*)(3*sizeof(float)));
		glBindBuffer(TGF_ARRAY_BUFFER, r->vboV);
		glBufferData(TGF_ARRAY_BUFFER, r->vertex_len*sizeof(mesh_core::data), (void*)r->vertex, TGF_STATIC_DRAW);
		glBindBuffer(TGF_ELEMENT_ARRAY_BUFFER, r->vboI);
		glBufferData(TGF_ELEMENT_ARRAY_BUFFER, r->index_len*sizeof(unsigned short), (void*)r->index, TGF_STATIC_DRAW);
	}
	glBindVertexArray(0);
	//texture
	for (std::vector<texture_core*>::iterator i = managedTexture.begin(); i != managedTexture.end(); i++) {
		glGenTextures(1, &(*i)->id);
		glBindTexture(TGF_TEXTURE_2D, (*i)->id);
	  glPixelStorei(TGF_UNPACK_ALIGNMENT, 1);
		glTexImage2D(TGF_TEXTURE_2D, 0, TGF_RGBA8, (*i)->width, (*i)->height, 0, TGF_RGBA, TGF_UNSIGNED_BYTE, (void*)(*i)->data);
	}
	glBindTexture(TGF_TEXTURE_2D, 0);
	
	valid = true;
}
void tgf_gles::invalidate() {
	if (!valid) return;
	//invalidating gles resources
		
	//capabilities
	for (std::vector<unsigned int>::iterator i = capabilities.begin(); i != capabilities.end(); i++) {
		glDisable(*i);
	}
	//shader
	for (std::vector<shader_core*>::iterator i = managedShader.begin(); i != managedShader.end(); i++) {
		glDeleteProgram((*i)->id);
	}
	//mesh
	for (std::vector<mesh_core*>::iterator i = managedMesh.begin(); i != managedMesh.end(); i++) {
		mesh_core *r = *i;
		glDeleteVertexArrays(1, &r->vaoId);
		glDeleteBuffers(2, &r->vboV);
	}
	//texture
	for (std::vector<texture_core*>::iterator i = managedTexture.begin(); i != managedTexture.end(); i++) {
		glDeleteTextures(1, &(*i)->id);
	}
	
	valid = false;
}
