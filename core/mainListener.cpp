#include "mainListener.h"
#include <cstring>
float r = 0, g = 0, b = 0;
unsigned int sp/*, VAO, VBO, IBO*/;
bool binded = false;
void bind() {
	if (binded) return;
	const char *vShaderSrc = "#version 300 es"
		"\nlayout(location = 0) in vec4 a_position;"
		"\nvoid main() {"
		"\n    gl_Position = a_position;"
		"\n}\0", 
	*fShaderSrc = "#version 300 es"
		"\nprecision mediump float;"
		"\nout vec4 o_fragColor;"
		"\nvoid main() {"
		"\n    o_fragColor = vec4(1.0, 0, 0, 1.0);"
		"\n}\0";
	tgf->gen_shader(sp, vShaderSrc, fShaderSrc);
	/*
	unsigned int VAO;
	tgf->gen_vertex_array(&VAO);
	tgf->bind_vertex_array(&VAO);
	*/
	unsigned int VBO, IBO;
	tgf->gen_buffer(&VBO);
	tgf->gen_buffer(&IBO);
	/*
	tgf->bind_buffer(TGF_ARRAY_BUFFER, &VBO);
	{
			const float vertices[]{
				0.5f, 0.5f, 0.0f, 1.0f, 
				0.5f, -0.5f, 0.0f, 1.0f, 
				-0.5f, -0.5f, 0.0f, 1.0f,
				-0.5f, 0.5f, 0.0f, 1.0f
			};
			tgf->buffer_data(TGF_ARRAY_BUFFER, sizeof(vertices), (const void*)vertices, TGF_STATIC_DRAW);
	}
	tgf->bind_buffer(TGF_ARRAY_BUFFER, 0);
	tgf->bind_buffer(TGF_ELEMENT_ARRAY_BUFFER, &IBO);
	{
			const unsigned int indices[]{ 0, 1, 3, 1, 2, 3};
			tgf->buffer_data(TGF_ELEMENT_ARRAY_BUFFER, sizeof(indices), (const void*)indices, TGF_STATIC_DRAW);
	}
	tgf->bind_buffer(TGF_ELEMENT_ARRAY_BUFFER, 0);
	*/
	/*
	tgf->enable_vertex_attrib_array(0);
	tgf->vertex_attrib_pointer(0, 4, TGF_FLOAT, false, 4 * sizeof(float), (void*)0);
	tgf->bind_vertex_array(0);
	*/
	if((VBO|IBO) == 0)
			r = 0, g = 1, b = 1;
	//tgf->delete_vertex_array(&VAO);
	tgf->delete_buffer(&VBO);
	tgf->delete_buffer(&IBO);
	
	binded = true;
}
void Main::create() {
	if (!tgf) return;
	r = g = b = 1;
	//resume();
}
void Main::resume() {
	if (!tgf) return;
	r = 1, g = b = 0;
}
void Main::resize(unsigned int width, unsigned int height) {
	if (!tgf) return;
}
void Main::render(float delta) {
	if (!tgf) return;
	tgf->clearcolormask(TGF_COLOR_BUFFER_BIT|TGF_DEPTH_BUFFER_BIT|TGF_STENCIL_BUFFER_BIT, r, g, b, 1.f);
	bind();
	tgf->bind_shader(sp);
	/*
	tgf->bind_vertex_array(&VAO);
	tgf->draw_elements(TGF_TRIANGLES, 6, TGF_UNSIGNED_INT, 0);
	tgf->bind_vertex_array(0);
	*/
	tgf->bind_shader(0);
}
void Main::pause() {
	if (!tgf) return;
	tgf->delete_shader(sp);
	/*
	tgf->delete_vertex_array(&VAO);
	tgf->delete_buffer(&VBO);
	tgf->delete_buffer(&IBO);
	*/
	binded = false;
}
void Main::destroy() {
	if (!tgf) return;
}