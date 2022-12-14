#include "mainListener.h"

#include "math/matrix4.h"
#include <cstring>
#include <cmath>
#include <cstdlib>     /* srand, rand */
#include <time.h>       /* time */

TranslatedGraphicsFunction *tgf;

unsigned int width, height;
float r = 0, g = 0, b = 0;
/*
shader_core *sp;
mesh_core *mp;
int sp_worldview_matrix, sp_trans_matrix;
float worldview_proj[16];
float trans_proj[16];
*/
void Main::create(TranslatedGraphicsFunction *_tgf,unsigned int w, unsigned int h) {
	tgf = _tgf;
	width = w, height = h;
	if (!tgf) return;
	r = g = b = 1;
	tgf->viewport(0, 0, width, height);
	/*
	matrix4::toOrtho(worldview_proj, 0, width, 0, height, 0, 10000.0f);
	matrix4::idt(trans_proj);
	// create shader program {
	const char *vShaderSrc = "uniform mat4 worldview_proj;"
		"\nuniform mat4 trans_proj;"
		"\nlayout(location = 0) in vec4 a_position;"
		"\nlayout(location = 1) in vec4 a_color;"
		"\nout vec4 v_color;"
		"\nvoid main() {"
		"\n    v_color = a_color;"
		"\n    gl_Position = worldview_proj * trans_proj * a_position;"
		"\n}\0";
	const char *fShaderSrc = "precision MED float;"
		"\nin vec4 v_color;"
		"\nlayout(location = 0) out vec4 glFragColor;"
		"\nvoid main() {"
		"\n    glFragColor = v_color;"
		"\n}\0";
	sp = tgf->gen_shader(vShaderSrc, fShaderSrc);
	tgf->bind_shader(sp);
	sp_worldview_matrix = tgf->get_shader_uloc(sp, "worldview_proj");
	sp_trans_matrix = tgf->get_shader_uloc(sp, "trans_proj");
	tgf->u_matrix4fv(sp_worldview_matrix, 1, false, worldview_proj);
	// }
	// create mesh {
	mesh_core::data vert[24] = {
		//front red
		{ +350.0f, +350.0f, -350.0f, 0xff, 0x00, 0x00, 0xff },
		{ +350.0f, -350.0f, -350.0f, 0xff, 0x00, 0x00, 0xff },
		{ -350.0f, -350.0f, -350.0f, 0xff, 0x00, 0x00, 0xff },
		{ -350.0f, +350.0f, -350.0f, 0xff, 0x00, 0x00, 0xff }, 
		//left green
		{ -350.0f, +350.0f, -350.0f, 0x00, 0xff, 0x00, 0xff }, 
		{ -350.0f, -350.0f, -350.0f, 0x00, 0xff, 0x00, 0xff }, 
		{ -350.0f, -350.0f, +350.0f, 0x00, 0xff, 0x00, 0xff }, 
		{ -350.0f, +350.0f, +350.0f, 0x00, 0xff, 0x00, 0xff }, 
		//right blue
		{ +350.0f, +350.0f, +350.0f, 0x00, 0x00, 0xff, 0xff }, 
		{ +350.0f, -350.0f, +350.0f, 0x00, 0x00, 0xff, 0xff }, 
		{ +350.0f, -350.0f, -350.0f, 0x00, 0x00, 0xff, 0xff }, 
		{ +350.0f, +350.0f, -350.0f, 0x00, 0x00, 0xff, 0xff }, 
		//bot gray
		{ +350.0f, -350.0f, -350.0f, 0x33, 0x33, 0x33, 0xff }, 
		{ +350.0f, -350.0f, +350.0f, 0x33, 0x33, 0x33, 0xff }, 
		{ -350.0f, -350.0f, +350.0f, 0x33, 0x33, 0x33, 0xff }, 
		{ -350.0f, -350.0f, -350.0f, 0x33, 0x33, 0x33, 0xff }, 
		//top purple
		{ +350.0f, +350.0f, +350.0f, 0xff, 0x00, 0xff, 0xff }, 
		{ +350.0f, +350.0f, -350.0f, 0xff, 0x00, 0xff, 0xff }, 
		{ -350.0f, +350.0f, -350.0f, 0xff, 0x00, 0xff, 0xff }, 
		{ -350.0f, +350.0f, +350.0f, 0xff, 0x00, 0xff, 0xff }, 
		//back fulle
		{ -350.0f, +350.0f, +350.0f, 0x00, 0xff, 0xff, 0xff }, 
		{ -350.0f, -350.0f, +350.0f, 0xff, 0xff, 0xff, 0xff }, 
		{ +350.0f, -350.0f, +350.0f, 0x00, 0x00, 0xff, 0xff }, 
		{ +350.0f, +350.0f, +350.0f, 0x00, 0xff, 0x00, 0xff }
	};
	unsigned short indices[36] = {
		0,1,3,1,2,3,//front
		4,5,7,5,6,7,//left
		8,9,11,9,10,11,//right
		12,13,15,13,14,15,//bot
		16,17,19,17,18,19,//top
		20,21,23,21,22,23//back
	};
	mp = tgf->gen_mesh(vert, 24, indices, 36);
	//}
	*/
	tgf->switch_capability(TGF_DEPTH_TEST, true);
	tgf->switch_capability(TGF_CULL_FACE, true);
	tgf->cull_face(TGF_FRONT);
	tgf->depth_func(TGF_LESS);
}
void Main::resume() {
	if (!tgf) return;
	r = 0, g = 1, b = 0;
}
void Main::resize(unsigned int w, unsigned int h) {
	width = w, height = h;
	if (!tgf) return;
	tgf->viewport(0, 0, width, height);
	/*
	matrix4::toOrtho(worldview_proj, 0, width, 0, height, 0, 10000.0f);
	tgf->bind_shader(sp);
	tgf->u_matrix4fv(sp_worldview_matrix, 1, false, worldview_proj);
	tgf->bind_shader(0);
	*/
}
void Main::render(float delta) {
	if (!tgf) return;
	tgf->clearcolormask(TGF_COLOR_BUFFER_BIT|TGF_DEPTH_BUFFER_BIT|TGF_STENCIL_BUFFER_BIT, r, g, b, 1.f);
	/*
	tgf->bind_shader(sp);
	srand(time(NULL));
	matrix4::rotate(trans_proj,
		M_PI / std::fmax(float(rand()%1000), 240.0f),
		M_PI / std::fmax(float(rand()%1000), 240.0f),
		M_PI / std::fmax(float(rand()%1000), 240.0f)
	);
	tgf->u_matrix4fv(sp_trans_matrix, 1, false, trans_proj);
	tgf->draw_mesh(mp);
	tgf->bind_shader(0);
	*/
}
void Main::pause() {
	if (!tgf) return;
}
void Main::destroy() {
	if (!tgf) return;
	/*
	tgf->delete_shader(sp);
	tgf->delete_mesh(mp);
	*/
}