#include "mainListener.h"
#include <cstring>
float r = 0, g = 0, b = 0;
unsigned int sp, VAO, VBO, IBO;
int sp_matrix;
bool binded = false;
void bind() {
	if (binded) return;
	const char *vShaderSrc = "#version 300 es"
		"\nuniform mat4 u_proj;"
		"\nlayout(location = 0) in vec4 a_position;"
		"\nlayout(location = 1) in vec4 a_color;"
		"\nout vec4 v_color;"
		"\nvoid main() {"
		"\n    v_color = a_color;"
		"\n    gl_Position = u_proj * a_position;"
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
	tgf->get_uniform_location(sp, "u_proj", sp_matrix);
	tgf->gen_vertex_array(VAO);
	tgf->gen_buffer(VBO);
	tgf->gen_buffer(IBO);
	tgf->bind_vertex_array(VAO);
	tgf->bind_buffer(TGF_ARRAY_BUFFER, VBO);
	struct {
		const unsigned char color[16] = {
			0xff, 0x00, 0x00, 0xff, 
			0xff, 0xff, 0x00, 0xff, 
			0x00, 0x00, 0xff, 0xff, 
			0x00, 0xff, 0x00, 0xff, 
			0xff, 0x00, 0x00, 0xff, 
			0xff, 0xff, 0x00, 0xff, 
			0x00, 0x00, 0xff, 0xff, 
			0x00, 0xff, 0x00, 0xff
		};
		const float position[8] = {
			+350.0f, +350.0f, -350.0f, 
			+350.0f, -350.0f, -350.0f, 
			-350.0f, -350.0f, -350.0f, 
			-350.0f, +350.0f, -350.0f, 
			+350.0f, +350.0f, +350.0f, 
			+350.0f, -350.0f, +350.0f, 
			-350.0f, -350.0f, +350.0f, 
			-350.0f, +350.0f, +350.0f
		};
	} vertices;
	tgf->buffer_data(TGF_ARRAY_BUFFER, sizeof(vertices), (void*)&vertices, TGF_STATIC_DRAW);
	tgf->bind_buffer(TGF_ELEMENT_ARRAY_BUFFER, IBO);
	const unsigned short indices[]{
		0,1,3,1,2,3,//front
		4,5,0,5,1,0,//right
		3,2,7,2,6,7,//left
		1,6,2,6,5,2,//bot
		4,0,7,0,3,7,//top
		7,6,4,6,5,4 //back
	};
	tgf->buffer_data(TGF_ELEMENT_ARRAY_BUFFER, sizeof(indices), (void*)indices, TGF_STATIC_DRAW);
	tgf->enable_vertex_attrib_array(0);
	tgf->vertex_attrib_pointer(0, 3, TGF_FLOAT, false, 3 * sizeof(float), (void*)sizeof(vertices.color));
	tgf->enable_vertex_attrib_array(1);
	tgf->vertex_attrib_pointer(1, 4, TGF_UNSIGNED_BYTE, true, 4 * sizeof(unsigned char), (void*)0);
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
	tgf->bind_shader(sp);
	tgf->uniform_matrix4fv(sp_matrix, 1, false, {1.0f/(float)width,0,0,0, 0,1.0f/(float)height,0,0, 0,0,0.0001f,0, 0,0,0,1.0f});
	tgf->bind_shader(0);
}
void Main::render(float delta) {
	if (!tgf) return;
	tgf->clearcolormask(TGF_COLOR_BUFFER_BIT|TGF_DEPTH_BUFFER_BIT|TGF_STENCIL_BUFFER_BIT, r, g, b, 1.f);
	bind();
	tgf->bind_shader(sp);
	tgf->bind_vertex_array(VAO);
	tgf->draw_elements(TGF_TRIANGLES, 36, TGF_UNSIGNED_SHORT, 0);
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