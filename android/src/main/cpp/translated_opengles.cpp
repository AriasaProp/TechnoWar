#include "translated_opengles.h"

// make opengles lastest possible version
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

#define MAX_GL_MSG 512

static GLuint *utemp = 0;
static GLint *stemp = 0;
static GLchar *s_msg = 0;

tgf_gles::tgf_gles() {
		utemp = new GLuint[3];
		stemp = new GLint[3];
		s_msg = new GLchar[MAX_GL_MSG];
}
tgf_gles::~tgf_gles() {
		delete[] utemp;
		delete[] stemp;
		delete[] s_msg;
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
		    glShaderSource(vertex, 1, &v, 0);
		    glCompileShader(vertex);
		    glGetShaderiv(vertex, GL_COMPILE_STATUS, stemp);
		    if (stemp[0] == 0){
		        glGetShaderInfoLog(vertex, MAX_GL_MSG, 0, s_msg);
		    		throw(s_msg);
		    }
		    glShaderSource(fragment, 1, &f, 0);
		    glCompileShader(fragment);
		    glGetShaderiv(fragment, GL_COMPILE_STATUS, stemp);
		    if (stemp[0] == 0){
		        glGetShaderInfoLog(fragment, MAX_GL_MSG, 0, s_msg);
		    		throw(s_msg);
		    }
		    glAttachShader(program, vertex);
		    glAttachShader(program, fragment);
		    glLinkProgram(program);
		    glGetProgramiv(program, GL_LINK_STATUS, stemp);
		    if (stemp[0] == 0){
		        glGetProgramInfoLog(program, MAX_GL_MSG, 0, s_msg);
		        throw(s_msg);
		    }
		} catch (...) {
				glDeleteProgram(program);
				program = 0;
		}
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		return program;
}
void tgf_gles::delete_shader(const unsigned int program) {
		glDeleteProgram(program);
}
unsigned int tgf_gles::gen_buffer() {
		glGenBuffers(1, utemp);
		return utemp[0];
}
void tgf_gles::bind_buffer(int type, const unsigned int id) {
		glBindBuffer(type, id);
}
void tgf_gles::buffer_data(int type, const unsigned int data_len, const unsigned char *data, int datatype) {
		glBufferData(type, data_len, data, datatype);
}
unsigned int tgf_gles::gen_vertex_array() {
		glGenVertexArrays(1, utemp);
		return utemp[0];
}
void tgf_gles::bind_vertex_array(const unsigned int id) {
		glBindVertexArray(id);
}
