#include "mainListener.h"
#include <cstring>
float r = 0, g = 0, b = 0;
unsigned int sp, VAO, VBO, IBO;
bool binded = false;
void bind() {
	if (binded) return;
	const char *vShaderSrc = "#version 300 es"
		"\nlayout(location = 0) in vec2 a_position;"
		"\nlayout(location = 1) in vec4 a_color;"
		"\nout vec4 v_color;"
		"\nvoid main() {"
		"\n    v_color = a_color;"
		"\n    gl_Position = vec4(a_position,0.0f, 1.0f);"
		"\n}\0", 
	*fShaderSrc = "#version 300 es"
		"\nprecision mediump float;"
		"\nin vec4 v_color;"
		"\nout vec4 o_fragColor;"
		"\nvoid main() {"
		"\n    o_fragColor = v_color;"
		"\n}\0";
	tgf->gen_shader(sp, vShaderSrc, fShaderSrc);
	tgf->bind_shader(sp);
	tgf->gen_vertex_array(VAO);
	tgf->gen_buffer(VBO);
	tgf->gen_buffer(IBO);
	tgf->bind_vertex_array(VAO);
	tgf->bind_buffer(TGF_ARRAY_BUFFER, VBO);
	const float vertices[]{
		0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 
		0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 
		-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
		-0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f
	};
	tgf->buffer_data(TGF_ARRAY_BUFFER, sizeof(vertices), (void*)vertices, TGF_STATIC_DRAW);
	tgf->bind_buffer(TGF_ELEMENT_ARRAY_BUFFER, IBO);
	const unsigned short indices[]{ 0, 1, 3, 1, 2, 3};
	tgf->buffer_data(TGF_ELEMENT_ARRAY_BUFFER, sizeof(indices), (void*)indices, TGF_STATIC_DRAW);
	tgf->enable_vertex_attrib_array(0);
	tgf->vertex_attrib_pointer(0, 2, TGF_FLOAT, false, 6 * sizeof(float), (void*)0);
	tgf->enable_vertex_attrib_array(1);
	tgf->vertex_attrib_pointer(1, 4, TGF_FLOAT, false, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	tgf->bind_vertex_array(0);
	tgf->bind_shader(0);
	
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
	tgf->viewport(0, 0, width, height);
}
void Main::render(float delta) {
	if (!tgf) return;
	tgf->clearcolormask(TGF_COLOR_BUFFER_BIT|TGF_DEPTH_BUFFER_BIT|TGF_STENCIL_BUFFER_BIT, r, g, b, 1.f);
	bind();
	tgf->bind_shader(sp);
	tgf->bind_vertex_array(VAO);
	tgf->draw_elements(TGF_TRIANGLES, 6, TGF_UNSIGNED_SHORT, 0);
	tgf->bind_vertex_array(0);
	tgf->bind_shader(0);
}
void Main::pause() {
	if (!tgf) return;
	tgf->delete_shader(sp);
	tgf->delete_vertex_array(VAO);
	tgf->delete_buffer(VBO);
	tgf->delete_buffer(IBO);
	binded = false;
}
void Main::destroy() {
	if (!tgf) return;
}