#include "main_game.h"

#include "user_interface/user_interface.h"
#include "math/matrix4.h"
#include <cstring>
#include <cmath>
#include <cstdlib>     /* srand, rand */
#include <time.h>       /* time */

graphics *m_graphics = nullptr;
input *m_input = nullptr;

mesh_core *mp;
Actor ml, mb, mc;

void Main::create(graphics *_graphics, input *_input) {
	m_graphics = _graphics;
	m_input = _input;
	ml.x = 120;
	ml.y = 120;
	ml.width = 400;
	ml.height = 350;
	ml.color = 0xffff00ff;
	ml.tex = m_graphics->gen_texture(2,2,(unsigned char[]){0xff, 0xff, 0xff, 0xff,0x00, 0x00, 0x55, 0xff,0xff, 0x00, 0x00, 0xff,0x55, 0x00, 0x00, 0xff});
	user_interface::addActor(&ml);
	mb.x = 700;
	mb.y = 100;
	mb.width = 300;
	mb.height = 250;
	mb.color = 0x00ff00ff;
	user_interface::addActor(&mb);
	mc.x = 1100;
	mc.y = 100;
	mc.width = 250;
	mc.height = 200;
	mc.color = 0x00ff00ff;
	user_interface::addActor(&mc);
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
	(void)delta;
	m_input->process_event();
	m_graphics->clear(1|2|4);
	
	srand(time(NULL));
	matrix4::rotate(mp->trans,
		M_PI / std::fmax(float(rand()%1000), 240.0f),
		M_PI / std::fmax(float(rand()%1000), 240.0f),
		M_PI / std::fmax(float(rand()%1000), 240.0f)
	);
	m_graphics->mesh_render(&mp, 1);
	memcpy(&ml.color, (unsigned char[]){
		(unsigned char)(0xff * ((float)m_input->getX(0)/m_graphics->getWidth())),
		(unsigned char)(0xff * ((float)m_input->getY(0)/m_graphics->getHeight())),
		0x00,
		0xff
	}, 4*sizeof(unsigned char));
	memcpy(&mb.color, (unsigned char[]){
		(unsigned char)(0xff * ((float)m_input->getX(1)/m_graphics->getWidth())),
		(unsigned char)(0xff * ((float)m_input->getY(1)/m_graphics->getHeight())),
		0x00,
		0xff
	}, 4*sizeof(unsigned char));
	memcpy(&mc.color, (unsigned char[]){
		(unsigned char)(0xff * ((float)m_input->getX(2)/m_graphics->getWidth())),
		(unsigned char)(0xff * ((float)m_input->getY(2)/m_graphics->getHeight())),
		0x00,
		0xff
	}, 4*sizeof(unsigned char));
	user_interface::draw(m_graphics);
}
void Main::pause() {
}
void Main::destroy() {
	m_graphics->delete_mesh(mp);
	delete ml.tex;
	user_interface::clearActor();
}