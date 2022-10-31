#include "mainListener.h"

float r = 0, g = 0, b = 0;

void Main::create() {
	if (!tgf) return;
	r = g = b = 1;
}
void Main::resume() {
	if (!tgf) return;
	r = 1, g = b = 0;
}
void Main::resize(unsigned int width, unsigned int height) {
	if (!tgf) return;
	//r = g = 0, b = 1;
	
}
void Main::render(float delta) {
	if (!tgf) return;
	tgf->clearcolormask(TGF::COLOR_BUFFER_BIT|TGF::DEPTH_BUFFER_BIT|TGF::STENCIL_BUFFER_BIT, r, g, b, 1.f);
}
void Main::pause() {
	r = 1, g = 1, b = 0;
	if (!tgf) return;
}
void Main::destroy() {
	if (!tgf) return;
	delete tgf;
	tgf = nullptr;
}