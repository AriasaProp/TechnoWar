#include "translated_opengles.h"

// make opengles lastest possible version
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

void tgf_gles::clearcolormask(unsigned int m, float r, float g, float b, float a) {
		glClearColor(r, g, b, a);
		glClear(m);
}

void tgf_gles::viewport(unsigned int x, unsigned int y, unsigned int w, unsigned int h) {
		glViewport(x, y, w, h);
}

