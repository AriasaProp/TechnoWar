#include "mainListener.h"

float r = 0, g = 0, b = 0;

void Main::create() {
	r = g = b = 1;
	if (!tgf) return;
	//tgf->clearcolormask(TGF::COLOR_BUFFER_BIT|TGF::DEPTH_BUFFER_BIT|TGF::STENCIL_BUFFER_BIT, 1.f, 0.f, 0.f, 1.f);
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
	
}
void Main::pause() {
	r = 1, g = 1, b = 0;
	if (!tgf) return;
	//tgf->clearcolormask(TGF::COLOR_BUFFER_BIT|TGF::DEPTH_BUFFER_BIT|TGF::STENCIL_BUFFER_BIT, 0.f, 1.f, 0.f, 1.f);
}
void Main::destroy() {
	if (!tgf) return;
	delete tgf;
	tgf = nullptr;
}