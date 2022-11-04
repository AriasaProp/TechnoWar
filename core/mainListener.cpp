#include "mainListener.h"
#include <cstring>
float r = 0, g = 0, b = 0;
unsigned int sp;//, VAO, VBO, IBO;
bool binded = false;
void bind(TranslatedGraphicsFunction *tgf) {
	const char *vShaderSrc = "#version 300 es"
		"\nlayout(location = 0) in vec4 a_position;"
		"\nlayout(location = 1) in vec4 a_color;"
		"\nout vec4 v_color;"
		"\nvoid main() {"
		"\n    v_color = a_color;"
		"\n    gl_Position = a_position;"
		"\n}", 
	*fShaderSrc = "#version 300 es"
		"\nprecision mediump float;"
		"\nin vec4 v_color;"
		"\nout vec4 o_fragColor;"
		"\nvoid main() {"
		"\n    o_fragColor = v_color;"
		"\n}";
	tgf->gen_shader(&sp, vShaderSrc, fShaderSrc);
	if(!sp)
			r = 0, g = 1, b = 1;
	binded = true;
	/*
	tgf->gen_vertex_array(&VAO);
	tgf->bind_vertex_array(&VAO);
	tgf->gen_buffer(&VBO);
	tgf->bind_buffer(TGF::ARRAY_BUFFER, &VBO);
	float vertices[]{
		0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
		0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
		-0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 1.0f
	};
	tgf->buffer_data(TGF::ARRAY_BUFFER, sizeof(vertices), vertices, TGF::STATIC_DRAW);
	tgf->gen_buffer(&IBO);
	tgf->bind_buffer(TGF::ELEMENT_ARRAY_BUFFER, &IBO);
	unsigned int indices[]{ 0, 1, 3, 1, 2, 3};
	tgf->buffer_data(TGF::ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, TGF::STATIC_DRAW);
	tgf->vertex_attrib_pointer(0, 2, TGF::FLOAT, false, 2 * sizeof(float), (void*)0);
	tgf->enable_vertex_attrib_array(0);
	tgf->vertex_attrib_pointer(1, 4, TGF::FLOAT, false, 6 * sizeof(float), (void*)2);
	tgf->enable_vertex_attrib_array(1);
	tgf->bind_vertex_array(0);
	*/
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
	tgf->clearcolormask(TGF::COLOR_BUFFER_BIT|TGF::DEPTH_BUFFER_BIT|TGF::STENCIL_BUFFER_BIT, r, g, b, 1.f);
	if (!binded) bind(tgf);
	/*
	tgf->bind_shader(&sp);
	tgf->bind_vertex_array(&VAO);
	tgf->draw_elements(TGF::TRIANGLES, 6, TGF::UNSIGNED_INT, 0);
	tgf->bind_vertex_array(0);
	tgf->bind_shader(0);
	*/
}
void Main::pause() {
	if (!tgf) return;
	tgf->delete_shader(&sp);
	/*
	tgf->delete_vertex_array(&VAO);
	tgf->delete_buffer(&VBO);
	tgf->delete_buffer(&IBO);
	VAO = VBO = IBO = 0;
	*/
	binded = false;
}
void Main::destroy() {
	if (!tgf) return;
}