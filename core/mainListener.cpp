#include "mainListener.h"

#include "math/matrix4.h"
#include <cstring>
#include <cmath>
#include <cstdlib>     /* srand, rand */
#include <time.h>       /* time */

unsigned int width, height;
mesh_core *mp, *flatA;

void Main::create(unsigned int w, unsigned int h) {
	width = w, height = h;
	if (!tgf) return;
	tgf->viewport(0, 0, width, height);
	tgf->world_mesh((float)w,(float)h);
	
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
	mesh_core::data vert1[4] = {
		{15.f, 10.f, 0.f,0xff,0xff,0xff,0xff},
		{15.f, height/2.f, 0.f,0xff,0xff,0xff,0xff},
		{width/2.f, height/2.f, 0.f,0xff,0xff,0xff,0xff},
		{width/2.f, 10.f, 0.f,0xff,0xff,0xff,0xff},
	};
	unsigned short indc1[6] = {0,1,3,1,2,3};
	flatA = tgf->gen_mesh(vert1, 4, indc1, 6);
}
void Main::resume() {
	if (!tgf) return;
}
void Main::resize(unsigned int w, unsigned int h) {
	width = w, height = h;
	if (!tgf) return;
	tgf->viewport(0, 0, width, height);
	
	tgf->world_mesh((float)w,(float)h);
}
void Main::render(float delta) {
	if (!tgf) return;
	tgf->clear(TGF_COLOR_BUFFER_BIT|TGF_DEPTH_BUFFER_BIT|TGF_STENCIL_BUFFER_BIT);
	
	tgf->begin_mesh();
	{
		srand(time(NULL));
		matrix4::rotate(mp->trans,
			M_PI / std::fmax(float(rand()%1000), 240.0f),
			M_PI / std::fmax(float(rand()%1000), 240.0f),
			M_PI / std::fmax(float(rand()%1000), 240.0f)
		);
		tgf->draw_mesh(mp);
	}
	tgf->draw_mesh(flatA);
	tgf->end_mesh();
	
}
void Main::pause() {
	if (!tgf) return;
}
void Main::destroy() {
	if (!tgf) return;
	
	tgf->delete_mesh(mp);
}