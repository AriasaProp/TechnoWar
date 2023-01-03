#include "main_game.h"

#include "math/matrix4.h"
#include <cstring>
#include <cmath>
#include <cstdlib>     /* srand, rand */
#include <time.h>       /* time */

graphics *m_graphics = nullptr;
input *m_input = nullptr;

mesh_core *mp;

flat_vertex *v_t;

void Main::create(graphics *_graphics, input *_input) {
	v_t = new flat_vertex[4];
	unsigned char clr[4];
	v_t[0].x = 120;
	v_t[0].y = 120;
	clr[0] = 0xff;
	clr[1] = 0xff;
	clr[2] = 0x00;
	clr[3] = 0xff;
	memcpy(v_t[0].color, clr, 4*sizeof(unsigned char));
	v_t[1].x = 120;
	v_t[1].y = 300;
	clr[0] = 0x00;
	clr[1] = 0xff;
	clr[2] = 0x00;
	memcpy(v_t[1].color, clr, 4*sizeof(unsigned char));
	v_t[2].x = 300;
	v_t[2].y = 120;
	clr[0] = 0x00;
	clr[1] = 0xff;
	clr[2] = 0xff;
	memcpy(v_t[2].color, clr, 4*sizeof(unsigned char));
	v_t[3].x = 300;
	v_t[3].y = 300;
	clr[0] = 0x00;
	clr[1] = 0x00;
	clr[2] = 0xff;
	memcpy(v_t[3].color, clr, 4*sizeof(unsigned char));
	m_graphics = _graphics;
	m_input = _input;
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
	mp = m_graphics->gen_mesh(vert, 24, indices, 36);
}
void Main::resume() {
}
void Main::render(float delta) {
	m_graphics->clear(1|2|4);
	
	srand(time(NULL));
	matrix4::rotate(mp->trans,
		M_PI / std::fmax(float(rand()%1000), 240.0f),
		M_PI / std::fmax(float(rand()%1000), 240.0f),
		M_PI / std::fmax(float(rand()%1000), 240.0f)
	);
	m_graphics->mesh_render(&mp, 1);
	
	m_graphics->flat_render(v_t, 4);
}
void Main::pause() {
}
void Main::destroy() {
	m_graphics->delete_mesh(mp);
	delete[] v_t;
}