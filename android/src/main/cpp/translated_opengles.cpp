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

//maximum output log message in char
#define MAX_GL_MSG 1024

std::vector<shader_core*> managedShader;

tgf_gles::tgf_gles() {
	temp = new int[2];
	utemp = new unsigned int[2];
	msg = new char[MAX_GL_MSG];
	valid = true;
}
tgf_gles::~tgf_gles() {
	delete[] temp;
	delete[] utemp;
	delete[] msg;
	valid = false;
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
	o->v = v;
	o->f = f;
	utemp[0] = glCreateShader(GL_VERTEX_SHADER);
	utemp[1] = glCreateShader(GL_FRAGMENT_SHADER);
	try {
		char *vSrc = (char*)alloca(strlen(header_glsl)+strlen(v));
		strcpy(vSrc, header_glsl);
		strcat(vSrc, v);
		glShaderSource(utemp[0], 1, &vSrc, 0);
		glCompileShader(utemp[0]);
		glGetShaderiv(utemp[0], GL_COMPILE_STATUS, temp);
		if (temp[0] == 0) {
			glGetShaderInfoLog(utemp[0], MAX_GL_MSG, 0, msg);
			throw(msg);
		}
		char *fSrc = (char *)alloca(strlen(header_glsl)+strlen(f));
		strcpy(fSrc, header_glsl);
		strcat(fSrc, f);
		glShaderSource(utemp[1], 1, &fSrc, 0);
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
	glUseProgram(p?p->id:0);
}
void tgf_gles::delete_shader(shader_core *p) {
	glDeleteProgram(p->id);
	std::vector<shader_core*>::iterator it = std::find(managedShader.begin(), managedShader.end(), p);
  if(it != v.end())
    managedShader.erase(it);
  delete p;
  *p = NULL;
}
int tgf_gles::get_shader_uloc(shader_core *p, const char *name) {
	return glGetUniformLocation(p->id, name);
}
void tgf_gles::u_matrix4fv(const int &loc,const int &count, const bool &trnspose, const float *mtrix) {
	glUniformMatrix4fv(loc, count, trnspose, mtrix);
}
void tgf_gles::gen_buffer(unsigned int &b) {
	glGenBuffers(1, utemp);
	b = utemp[0];
}
void tgf_gles::bind_buffer(unsigned int type, const unsigned int b) {
	glBindBuffer(type, b);
}
void tgf_gles::buffer_data(unsigned int type, long bytes_len, const void *data, unsigned int datatype) {
	glBufferData(type, bytes_len, data, datatype);
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
void tgf_gles::enable_capability(const unsigned int &cap) {
	glEnable(cap);
}
void tgf_gles::disable_capability(const unsigned int &cap) {
	glDisable(cap);
}
void tgf_gles::cull_face(const unsigned int &face) {
	glCullFace(face);
}
void tgf_gles::depth_func(const unsigned int &func) {
	glDepthFunc(func);
}
void tgf_gles::depth_rangef(float near,float far) {
	glDepthRangef(near, far);
}

void tgf_gles::validate() {
	if (valid) return;
	//validating gles resources
	for (std::vector<shader_core*>::iterator i = managedShader.begin(); i != managedShader.end(); i++) {
		(*i)->id = glCreateProgram();
		utemp[0] = glCreateShader(GL_VERTEX_SHADER);
		utemp[1] = glCreateShader(GL_FRAGMENT_SHADER);
		char *vSrc = (char*)alloca(strlen(header_glsl)+strlen((*i)->v));
		strcpy(vSrc, header_glsl);
		strcat(vSrc, (*i)->v);
		glShaderSource(utemp[0], 1, &vSrc, 0);
		glCompileShader(utemp[0]);
		char *fSrc = (char *)alloca(strlen(header_glsl)+strlen((*i)->f));
		strcpy(fSrc, header_glsl);
		strcat(fSrc, (*i)->f);
		glShaderSource(utemp[1], 1, &fSrc, 0);
		glCompileShader(utemp[1]);
		glAttachShader((*i)->id, utemp[0]);
		glAttachShader((*i)->id, utemp[1]);
		glLinkProgram((*i)->id);
		glDeleteShader(utemp[0]);
		glDeleteShader(utemp[1]);
	}
	valid = true;
}
void tgf_gles::invalidate() {
	if (!valid) return;
	for (std::vector<shader_core*>::iterator i = managedShader.begin(); i != managedShader.end(); i++) {
		glDeleteProgram((*i)->id);
		(*i)->id = 0;
	}
	valid = false;
}
