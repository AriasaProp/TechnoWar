#include "translated_opengles.h"

// make opengles lastest possible version
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

//maximum output log message in char
#define MAX_GL_MSG 512

tgf_gles::tgf_gles() {
	
}
tgf_gles::~tgf_gles() {
	
}
const char *tgf_gles::renderer() {
	return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
}
void tgf_gles::clearcolormask(const unsigned int &m, const float &r, const float &g, const float &b, const float &a) {
	glClearColor(r, g, b, a);
	glClear(m);
}
void tgf_gles::viewport(const unsigned int &x, const unsigned int &y, const unsigned int &w, const unsigned int &h) {
	glViewport(x, y, w, h);
}
void tgf_gles::gen_shader(unsigned int &p, const char *v, const char *f) {
	p = glCreateProgram();
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	try {
		GLint temp;
		glShaderSource(vertex, 1, &v, 0);
		glCompileShader(vertex);
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &temp);
		if (temp == 0) {
			GLchar msg[MAX_GL_MSG];
			glGetShaderInfoLog(vertex, MAX_GL_MSG, 0, msg);
			throw(msg);
		}
		glShaderSource(fragment, 1, &f, 0);
		glCompileShader(fragment);
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &temp);
		if (temp == 0){
			GLchar msg[MAX_GL_MSG];
			glGetShaderInfoLog(fragment, MAX_GL_MSG, 0, msg);
			throw(msg);
		}
		glAttachShader(p, vertex);
		glAttachShader(p, fragment);
		glLinkProgram(p);
		glGetProgramiv(p, GL_LINK_STATUS, &temp);
		if (temp == 0){
			GLchar msg[MAX_GL_MSG];
			glGetProgramInfoLog(p, MAX_GL_MSG, 0, msg);
			throw(msg);
		}
	} catch (...) {
		glDeleteProgram(p);
		p = 0;
	}
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}
void tgf_gles::bind_shader(const unsigned int &p) {
	glUseProgram(p);
}
void tgf_gles::delete_shader(unsigned int &p) {
	glDeleteProgram(p);
	p = 0;
}
void tgf_gles::gen_buffer(unsigned int *b) {
	glGenBuffers(1, b);
}
void tgf_gles::bind_buffer(unsigned int type, unsigned int *b) {
	glBindBuffer(type, *b);
}
void tgf_gles::buffer_data(unsigned int type, long bytes_len, const void *data, unsigned int datatype) {
	glBufferData(type, bytes_len, (const GLvoid*)data, datatype);
}
void tgf_gles::delete_buffer(unsigned int *b) {
	glDeleteBuffers(1, b);
	*b = 0;
}
void tgf_gles::gen_vertex_array(unsigned int *v) {
	glGenVertexArrays(1, v);
}
void tgf_gles::bind_vertex_array(unsigned int *v) {
	glBindVertexArray(*v);
}
void tgf_gles::delete_vertex_array(unsigned int *v) {
	glDeleteVertexArrays(1, v);
	*v = 0;
}
void tgf_gles::vertex_attrib_pointer(unsigned int loc, unsigned int size, int type, bool normalize, unsigned int stride, void *offset) {
	glVertexAttribPointer(loc, size, type, normalize, stride, offset);
}
void tgf_gles::enable_vertex_attrib_array(unsigned int loc) {
	glEnableVertexAttribArray(loc);
}
void tgf_gles::draw_elements(int drawType, unsigned int indSize, int inType, const void *offset) {
	glDrawElements(drawType, indSize, inType, offset);
}

