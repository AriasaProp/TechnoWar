#include "translated_opengles.h"

// make opengles lastest possible version
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

GLuint *temp = 0;

tgf_gles::tgf_gles() {
		temp = new GLuint[3];
		
}
tgf_gles::~tgf_gles() {
		delete[] temp;
}

const char *tgf_gles::renderer() {
		return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
}

void tgf_gles::clearcolormask(unsigned int m, float r, float g, float b, float a) {
		glClearColor(r, g, b, a);
		glClear(m);
}

void tgf_gles::viewport(unsigned int x, unsigned int y, unsigned int w, unsigned int h) {
		glViewport(x, y, w, h);
}

unsigned int tgf_gles::gen_shader(const char *v, const char *f) {
		GLuint program = glCreateProgram();
		GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
		try {
		    glShaderSource(vertex, 1, &v);
		    glCompileShader(vertex);
		    glGetShaderiv(vertex, GL_COMPILE_STATUS, temp);
		    if (!temp[0])
		        throw(glGetShaderInfoLog(vertex));
		    glShaderSource(fragment, 1, &f);
		    glCompileShader(fragment);
		    glGetShaderiv(fragment, GL_COMPILE_STATUS, temp);
		    if (!temp[0])
		        throw(glGetShaderInfoLog(fragment)); 
		    glAttachShader(program, vertex);
		    glAttachShader(program, fragment);
		    glLinkProgram(program);
		    glGetProgramiv(program, GL_LINK_STATUS, temp);
		    if (!temp[0])
		        throw(glGetProgramInfoLog(program));
		} catch (...) {
				glDeleteProgram(program);
				program = 0;
		}
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		return program;
}

unsigned int tgf_gles::gen_buffer() {
		glGenBuffers(1, temp);
		return temp[0];
}
void tgf_gles::bind_buffer(int type, const unsigned int id) {
		glBindBuffers(type, id);
}
void tgf_gles::buffer_data(int type, const unsigned int data_len, const unsigned char *data, int datatype) {
		glBufferData(type, data_len, data, datatype)
}
unsigned int tgf_gles::gen_vertex_array() {
		glGenVertexArrays(1, temp);
		return temp[0];
}
void tgf_gles::bind_vertex_array(const unsigned int id) {
		glBindVertexArrays(id);
}
