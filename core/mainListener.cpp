#include "mainListener.h"
#include "math/matrix4.h"
#include <cstring>
#include <cmath>

unsigned int width, height;
float r = 0, g = 0, b = 0;
unsigned int sp, VAO, VBO, IBO;
int sp_worldview_matrix, sp_trans_matrix;
bool binded = false;
float worldview_proj[16] = {
	1.0f,0,0,0,
	0,1.0f,0,0,
	0,0,1.0f,0,
	0,0,-0.5f,1.0f
};
float trans_proj[16] = {
	1.0f,0,0,0,
	0,1.0f,0,0,
	0,0,1.0f,0,
	0,0,0,1.0f
};

void bind() {
	if (binded) return;
	const char *vShaderSrc = "#version 300 es"
		"\nuniform mat4 worldview_proj;"
		"\nuniform mat4 trans_proj;"
		"\nlayout(location = 0) in vec4 a_position;"
		"\nlayout(location = 1) in vec4 a_color;"
		"\nout vec4 v_color;"
		"\nvoid main() {"
		"\n    v_color = a_color;"
		"\n    gl_Position = worldview_proj * trans_proj * a_position;"
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
	tgf->get_shader_uloc(sp, "worldview_proj", sp_worldview_matrix);
	tgf->get_shader_uloc(sp, "trans_proj", sp_trans_matrix);
	tgf->u_matrix4fv(sp_worldview_matrix, 1, false, worldview_proj);
	tgf->gen_vertex_array(VAO);
	tgf->gen_buffer(VBO);
	tgf->gen_buffer(IBO);
	tgf->bind_vertex_array(VAO);
	tgf->bind_buffer(TGF_ARRAY_BUFFER, VBO);
	struct {
		const unsigned char color[96] = {
			//red
			0xff, 0x00, 0x00, 0xff,
			0xff, 0x00, 0x00, 0xff,
			0xff, 0x00, 0x00, 0xff,
			0xff, 0x00, 0x00, 0xff,
			//yellow
			0xff, 0xff, 0x00, 0xff, 
			0xff, 0xff, 0x00, 0xff, 
			0xff, 0xff, 0x00, 0xff, 
			0xff, 0xff, 0x00, 0xff, 
			//blue
			0x00, 0x00, 0xff, 0xff, 
			0x00, 0x00, 0xff, 0xff, 
			0x00, 0x00, 0xff, 0xff, 
			0x00, 0x00, 0xff, 0xff, 
			//green
			0x00, 0xff, 0x00, 0xff, 
			0x00, 0xff, 0x00, 0xff, 
			0x00, 0xff, 0x00, 0xff, 
			0x00, 0xff, 0x00, 0xff, 
			//purple
			0xff, 0x00, 0xff, 0xff, 
			0xff, 0x00, 0xff, 0xff, 
			0xff, 0x00, 0xff, 0xff, 
			0xff, 0x00, 0xff, 0xff, 
			//light blue
			0x00, 0xff, 0xff, 0xff, 
			0x00, 0xff, 0xff, 0xff, 
			0x00, 0xff, 0xff, 0xff, 
			0x00, 0xff, 0xff, 0xff
		};
		const float position[72] = {
			//front
			+350.0f, +350.0f, -350.0f, 
			+350.0f, -350.0f, -350.0f, 
			-350.0f, -350.0f, -350.0f, 
			-350.0f, +350.0f, -350.0f,
			//left
			-350.0f, +350.0f, -350.0f, 
			-350.0f, -350.0f, -350.0f, 
			-350.0f, -350.0f, +350.0f, 
			-350.0f, +350.0f, +350.0f,
			//right
			+350.0f, +350.0f, +350.0f,
			+350.0f, -350.0f, +350.0f, 
			+350.0f, -350.0f, -350.0f, 
			+350.0f, +350.0f, -350.0f, 
			//bot
			+350.0f, -350.0f, -350.0f, 
			+350.0f, -350.0f, +350.0f, 
			-350.0f, -350.0f, +350.0f,
			-350.0f, -350.0f, -350.0f, 
			//top
			+350.0f, +350.0f, +350.0f, 
			+350.0f, +350.0f, -350.0f, 
			-350.0f, +350.0f, -350.0f, 
			-350.0f, +350.0f, +350.0f,
			//back
			-350.0f, +350.0f, +350.0f, 
			-350.0f, -350.0f, +350.0f, 
			+350.0f, -350.0f, +350.0f, 
			+350.0f, +350.0f, +350.0f
		};
	} vertices;
	tgf->buffer_data(TGF_ARRAY_BUFFER, sizeof(vertices), (void*)&vertices, TGF_STATIC_DRAW);
	tgf->bind_buffer(TGF_ELEMENT_ARRAY_BUFFER, IBO);
	const unsigned short indices[]{
		0,1,3,1,2,3,//front
		4,5,7,5,6,7,//left
		8,9,11,9,10,11,//right
		12,13,15,13,14,15,//bot
		16,17,19,17,18,19,//top
		20,21,23,21,22,23//back
	};
	tgf->buffer_data(TGF_ELEMENT_ARRAY_BUFFER, sizeof(indices), (void*)indices, TGF_STATIC_DRAW);
	tgf->enable_vertex_attrib_array(0);
	tgf->vertex_attrib_pointer(0, 3, TGF_FLOAT, false, 3 * sizeof(float), (void*)sizeof(vertices.color));
	tgf->enable_vertex_attrib_array(1);
	tgf->vertex_attrib_pointer(1, 4, TGF_UNSIGNED_BYTE, true, 4 * sizeof(unsigned char), (void*)0);
	tgf->bind_vertex_array(0);
	tgf->bind_shader(0);
	tgf->depth_rangef(0, 1000);
	tgf->enable_capability(TGF_CULL_FACE);
	tgf->enable_capability(TGF_DEPTH_TEST);
	tgf->cull_face(TGF_FRONT);
	tgf->depth_func(TGF_LESS);
	binded = true;
}
void Main::create(unsigned int w, unsigned int h) {
	width = w, height = h;
	if (!tgf) return;
	r = g = b = 1;
	//resume();
	tgf->viewport(0, 0, width, height);
	worldview_proj[0] = 2.0f/width;
	worldview_proj[5] = 2.0f/height;
	worldview_proj[10] = 1.0f/10000.0f; //depth
}
void Main::resume() {
	if (!tgf) return;
	r = 1, g = b = 0;
}
void Main::resize(unsigned int w, unsigned int h) {
	width = w, height = h;
	if (!tgf) return;
	tgf->viewport(0, 0, width, height);
	worldview_proj[0] = 2.0f/width;
	worldview_proj[5] = 2.0f/height;
	tgf->bind_shader(sp);
	tgf->u_matrix4fv(sp_worldview_matrix, 1, false, worldview_proj);
	tgf->bind_shader(0);
}
const float allRot = M_PI / 360.0f;
void Main::render(float delta) {
	matrix4::rotate(trans_proj, allRot, allRot, 0);
	if (!tgf) return;
	tgf->clearcolormask(TGF_COLOR_BUFFER_BIT|TGF_DEPTH_BUFFER_BIT|TGF_STENCIL_BUFFER_BIT, r, g, b, 1.f);
	bind();
	tgf->bind_shader(sp);
	tgf->u_matrix4fv(sp_trans_matrix, 1, false, trans_proj);
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