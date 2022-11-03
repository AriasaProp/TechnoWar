#include "mainListener.h"

float r = 0, g = 0, b = 0;
unsigned int sp;

void Main::create() {
	if (!tgf) return;
	r = g = b = 1;
}
void Main::resume() {
	if (!tgf) return;
	r = 1, g = b = 0;
	const char *vShaderSrc = "#version 300 es"
		"\nlayout(location = 0) in vec4 a_position;"
		"\nlayout(location = 1) in vec4 a_color;"
		"\nout vec4 v_color;"
		"\nvoid main() {"
		"\n    v_color = a_color;"
		"\n    gl_Position = a_position;"
		"\n}", 
	*fShaderSrc = "#version 300 es"
		"\nprecision mediump float;"
		"\nin vec4 v_color;"
		"\nout vec4 o_fragColor;"
		"\nvoid main() {"
		"\n    o_fragColor = v_color;"
		"\n}";
	tgf->gen_shader(&sp, vShaderSrc, fShaderSrc);
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
	if (!tgf) return;
	r = 1, g = 1, b = 0;
	tgf->delete_shader(sp);
}
void Main::destroy() {
	if (!tgf) return;
	delete tgf;
	tgf = 0;
}