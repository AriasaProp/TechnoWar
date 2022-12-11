#include "mainListener.h"
#include "math/matrix4.h"
#include "2dbatch/uibatch.h"
#include <cstring>
#include <cmath>
#include <cstdlib>     /* srand, rand */
#include <time.h>       /* time */

TranslatedGraphicsFunction *tgf;
UI_Batch *batcher;
texture_core *tc_1;
float tc_data[] = {(float)0xff0000ff, (float)0x00ff00ff, (float)0x0000ffff, (float)0xffffffff};
unsigned int width, height;
float r = 0, g = 0, b = 0;
unsigned int VAO, VBO, IBO;
shader_core *sp;
int sp_worldview_matrix, sp_trans_matrix;
bool binded = false;
float worldview_proj[16] = {
	1.0f,0,0,0,
	0,1.0f,0,0,
	0,0,1.0f,0,
	0,0,0.f,1.0f
};
float trans_proj[16] = {
	1.0f,0,0,0,
	0,1.0f,0,0,
	0,0,1.0f,0,
	0,0,0,1.0f
};

void bind() {
	if (binded) return;
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
	//tgf->bind_shader(0);
	tgf->depth_rangef(0, 1000);
	tgf->switch_capability(TGF_CULL_FACE, true);
	tgf->switch_capability(TGF_DEPTH_TEST, true);
	tgf->cull_face(TGF_FRONT);
	tgf->depth_func(TGF_LESS);
	binded = true;
}
void Main::create(TranslatedGraphicsFunction *_tgf,unsigned int w, unsigned int h) {
	tgf = _tgf;
	width = w, height = h;
	if (!tgf) return;
	r = g = b = 1;
	//resume();
	tgf->viewport(0, 0, width, height);
	worldview_proj[0] = 2.0f/width;
	worldview_proj[5] = 2.0f/height;
	worldview_proj[10] = 1.0f/10000.0f; //depth
	
	// create shader program {
	const char *vShaderSrc = "uniform mat4 worldview_proj;"
		"\nuniform mat4 trans_proj;"
		"\nlayout(location = 0) in vec4 a_position;"
		"\nlayout(location = 1) in vec4 a_color;"
		"\nout vec4 v_color;"
		"\nvoid main() {"
		"\n    v_color = a_color;"
		"\n    gl_Position = worldview_proj * trans_proj * a_position;"
		"\n}\0", 
	*fShaderSrc = "precision MED float;"
		"\nin vec4 v_color;"
		"\nout vec4 glFragColor;"
		"\nvoid main() {"
		"\n    glFragColor = v_color;"
		"\n}\0";
	sp = tgf->gen_shader(vShaderSrc, fShaderSrc);
	tgf->bind_shader(sp);
	sp_worldview_matrix = tgf->get_shader_uloc(sp, "worldview_proj");
	sp_trans_matrix = tgf->get_shader_uloc(sp, "trans_proj");
	tgf->u_matrix4fv(sp_worldview_matrix, 1, false, worldview_proj);
	// }
	batcher = new UI_Batch(width,height);
	tc_1 = tgf->gen_texture(2, 2, (unsigned char*)tc_data);
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
	batcher->resize(width, height);
}
void Main::render(float delta) {
	if (!tgf) return;
	srand(time(NULL));
	matrix4::rotate(trans_proj,
		M_PI / std::fmax(float(rand()%1000), 240.0f),
		M_PI / std::fmax(float(rand()%1000), 240.0f),
		M_PI / std::fmax(float(rand()%1000), 240.0f)
	);
	tgf->clearcolormask(TGF_COLOR_BUFFER_BIT|TGF_DEPTH_BUFFER_BIT|TGF_STENCIL_BUFFER_BIT, r, g, b, 1.f);
	bind();
	tgf->bind_shader(sp);
	tgf->u_matrix4fv(sp_trans_matrix, 1, false, trans_proj);
	tgf->bind_vertex_array(VAO);
	tgf->draw_elements(TGF_TRIANGLES, 36, TGF_UNSIGNED_SHORT, 0);
	tgf->bind_vertex_array(0);
	tgf->bind_shader(0);
	
	batcher->begin();
	batcher->draw(tc_1, 0, 0, width, height);
	batcher->end();
}
void Main::pause() {
	if (!tgf) return;
	tgf->delete_vertex_array(VAO);
	tgf->delete_buffer(VBO);
	tgf->delete_buffer(IBO);
	binded = false;
}
void Main::destroy() {
	if (!tgf) return;
	tgf->delete_shader(sp);
	tgf->delete_texture(tc_1);
	//delete tc_data;
	delete batcher;
}