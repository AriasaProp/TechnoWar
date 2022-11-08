#include "mainListener.h"
#include <cstring>
#include <cmath>

static void inline mulMatrix(float *mata, float *matb) {
#define M00 0
#define M01 4
#define M02 8
#define M03 12
#define M10 1
#define M11 5
#define M12 9
#define M13 13
#define M20 2
#define M21 6
#define M22 10
#define M23 14
#define M30 3
#define M31 7
#define M32 11
#define M33 15
	float tmp[16];
  tmp[M00] = mata[M00] * matb[M00] + mata[M01] * matb[M10] + mata[M02] * matb[M20] + mata[M03] * matb[M30];
  tmp[M01] = mata[M00] * matb[M01] + mata[M01] * matb[M11] + mata[M02] * matb[M21] + mata[M03] * matb[M31];
  tmp[M02] = mata[M00] * matb[M02] + mata[M01] * matb[M12] + mata[M02] * matb[M22] + mata[M03] * matb[M32];
  tmp[M03] = mata[M00] * matb[M03] + mata[M01] * matb[M13] + mata[M02] * matb[M23] + mata[M03] * matb[M33];
  tmp[M10] = mata[M10] * matb[M00] + mata[M11] * matb[M10] + mata[M12] * matb[M20] + mata[M13] * matb[M30];
  tmp[M11] = mata[M10] * matb[M01] + mata[M11] * matb[M11] + mata[M12] * matb[M21] + mata[M13] * matb[M31];
  tmp[M12] = mata[M10] * matb[M02] + mata[M11] * matb[M12] + mata[M12] * matb[M22] + mata[M13] * matb[M32];
  tmp[M13] = mata[M10] * matb[M03] + mata[M11] * matb[M13] + mata[M12] * matb[M23] + mata[M13] * matb[M33];
  tmp[M20] = mata[M20] * matb[M00] + mata[M21] * matb[M10] + mata[M22] * matb[M20] + mata[M23] * matb[M30];
  tmp[M21] = mata[M20] * matb[M01] + mata[M21] * matb[M11] + mata[M22] * matb[M21] + mata[M23] * matb[M31];
  tmp[M22] = mata[M20] * matb[M02] + mata[M21] * matb[M12] + mata[M22] * matb[M22] + mata[M23] * matb[M32];
  tmp[M23] = mata[M20] * matb[M03] + mata[M21] * matb[M13] + mata[M22] * matb[M23] + mata[M23] * matb[M33];
  tmp[M30] = mata[M30] * matb[M00] + mata[M31] * matb[M10] + mata[M32] * matb[M20] + mata[M33] * matb[M30];
  tmp[M31] = mata[M30] * matb[M01] + mata[M31] * matb[M11] + mata[M32] * matb[M21] + mata[M33] * matb[M31];
  tmp[M32] = mata[M30] * matb[M02] + mata[M31] * matb[M12] + mata[M32] * matb[M22] + mata[M33] * matb[M32];
  tmp[M33] = mata[M30] * matb[M03] + mata[M31] * matb[M13] + mata[M32] * matb[M23] + mata[M33] * matb[M33];
  memcpy(mata, tmp, sizeof(float) * 16);
#undef M00
#undef M01
#undef M02
#undef M03
#undef M10
#undef M11
#undef M12
#undef M13
#undef M20
#undef M21
#undef M22
#undef M23
#undef M30
#undef M31
#undef M32
#undef M33
}

unsigned int width, height;
float r = 0, g = 0, b = 0;
unsigned int sp, VAO, VBO, IBO;
int sp_world_matrix, sp_view_matrix;
bool binded = false;
float world_proj[16] = {
		1.0f,0,0,0,
		0,1.0f,0,0,
		0,0,1.0f,0,
		0,0,0,1.0f
	};
float view_proj[16] = {
	1.0f,0,0,0,
	0,1.0f,0,0,
	0,0,-1.0f,0,
	0,0,0,1.0f
};

void bind() {
	if (binded) return;
	const char *vShaderSrc = "#version 300 es"
		"\nuniform mat4 world_proj;"
		"\nuniform mat4 view_proj;"
		"\nlayout(location = 0) in vec4 a_position;"
		"\nlayout(location = 1) in vec4 a_color;"
		"\nout vec4 v_color;"
		"\nvoid main() {"
		"\n    v_color = a_color;"
		"\n    gl_Position = world_proj * view_proj * a_position;"
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
	tgf->get_shader_uniform_location(sp, "world_proj", sp_world_matrix);
	tgf->get_shader_uniform_location(sp, "view_proj", sp_view_matrix);
	tgf->uniform_matrix4fv(sp_world_matrix, 1, false, world_proj);
	tgf->gen_vertex_array(VAO);
	tgf->gen_buffer(VBO);
	tgf->gen_buffer(IBO);
	tgf->bind_vertex_array(VAO);
	tgf->bind_buffer(TGF_ARRAY_BUFFER, VBO);
	struct {
		const unsigned char color[32] = {
			0xff, 0x00, 0x00, 0xff, 
			0xff, 0xff, 0x00, 0xff, 
			0x00, 0x00, 0xff, 0xff, 
			0x00, 0xff, 0x00, 0xff, 
			0xff, 0x00, 0x00, 0xff, 
			0xff, 0xff, 0x00, 0xff, 
			0x00, 0x00, 0xff, 0xff, 
			0x00, 0xff, 0x00, 0xff
		};
		/*
		const float position[24] = {
			+0.5f, +0.5f, 0.f, 
			+0.5f, -0.5f, 0.f, 
			-0.5f, -0.5f, 0.f, 
			-0.5f, +0.5f, 0.f, 
			+0.5f, +0.5f, 1.0f, 
			+0.5f, -0.5f, 1.0f, 
			-0.5f, -0.5f, 1.0f, 
			-0.5f, +0.5f, 1.0f
		};
		*/
		const float position[24] = {
			+350.0f, +350.0f, 0.0f, 
			+350.0f, -350.0f, 0.0f, 
			-350.0f, -350.0f, 0.0f, 
			-350.0f, +350.0f, 0.0f, 
			+350.0f, +350.0f, 700.0f, 
			+350.0f, -350.0f, 700.0f, 
			-350.0f, -350.0f, 700.0f, 
			-350.0f, +350.0f, 700.0f
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
void Main::create(unsigned int w, unsigned int h) {
	width = w, height = h;
	if (!tgf) return;
	r = g = b = 1;
	//resume();
	tgf->viewport(0, 0, width, height);
	world_proj[0] = 2.0f/width;
	world_proj[5] = 2.0f/height;
	world_proj[11] = 2.0f/height; //depth
}
void Main::resume() {
	if (!tgf) return;
	r = 1, g = b = 0;
}
void Main::resize(unsigned int w, unsigned int h) {
	width = w, height = h;
	if (!tgf) return;
	tgf->viewport(0, 0, width, height);
	world_proj[0] = 2.0f/width;
	world_proj[5] = 2.0f/height;
	world_proj[11] = 2.0f/height; //depth
	tgf->bind_shader(sp);
	tgf->uniform_matrix4fv(sp_world_matrix, 1, false, world_proj);
	tgf->bind_shader(0);
}
const float allRot = M_PI / 360.0f;
float rotatE[16]{
	cos(allRot)*cos(allRot),cos(allRot)*sin(allRot)*sin(allRot) - sin(allRot)*cos(allRot),cos(allRot)*sin(allRot)*cos(allRot) + sin(allRot)*sin(allRot),0,
	sin(allRot)*cos(allRot),sin(allRot)*sin(allRot)*sin(allRot) + cos(allRot)*cos(allRot),sin(allRot)*sin(allRot)*cos(allRot) - cos(allRot)*sin(allRot),0,
	-sin(allRot),cos(allRot)*sin(allRot),cos(allRot)*cos(allRot),0,
	0,0,0,1.0f
};
void Main::render(float delta) {
	mulMatrix(view_proj, rotatE);
	if (!tgf) return;
	tgf->clearcolormask(TGF_COLOR_BUFFER_BIT|TGF_DEPTH_BUFFER_BIT|TGF_STENCIL_BUFFER_BIT, r, g, b, 1.f);
	bind();
	tgf->bind_shader(sp);
	tgf->uniform_matrix4fv(sp_view_matrix, 1, false, view_proj);
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