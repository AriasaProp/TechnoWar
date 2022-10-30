#include "mainListener.h"

void Main::create() {
	if (!tgf) return;
	tgf->clearcolormask(TGF::COLOR_BUFFER_BIT|TGF::DEPTH_BUFFER_BIT|TGF::STENCIL_BUFFER_BIT, 1.f, 0.f, 0.f, 1.f);
}
void Main::resume() {
	if (!tgf) return;
	tgf->clearcolormask(TGF::COLOR_BUFFER_BIT|TGF::DEPTH_BUFFER_BIT|TGF::STENCIL_BUFFER_BIT, 0.f, 1.f, 0.f, 1.f);
	
}
void Main::resize(unsigned int width, unsigned int height) {
	if (!tgf) return;
	
}
void Main::render(float delta) {
	if (!tgf) return;
	
}
void Main::pause() {
	if (!tgf) return;
	//tgf->clearcolormask(TGF::COLOR_BUFFER_BIT|TGF::DEPTH_BUFFER_BIT|TGF::STENCIL_BUFFER_BIT, 0.f, 1.f, 0.f, 1.f);
}
void Main::destroy() {
	if (!tgf) return;
	delete tgf;
	tgf = nullptr;
}