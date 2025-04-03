#include <SDL2/SDL.h>
#include "util.h"
#include "engine.h"

static struct inputManager {
	int arr[20];
} *im;


// core implementation
static struct vec2 getTouch (size_t p) {
	(void)p;
	struct vec2 r = {0,0};
  return r;
}


void sdl_inputManager_init() {
	im = (struct inputManager*)malloc(sizeof(struct inputManager));
	memset(im->arr, 0, 20*sizeof(int));
	struct engine *e = get_engine ();
	e->i.getTouch = getTouch;
}

int sdl_inputManager_update() {
	int ret = 1;
	static SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
			case SDL_QUIT:
				ret = 0;
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEMOTION:
			case SDL_MOUSEBUTTONUP:
				break;
			default:
				break;
		}
	}
	return ret;
}

void sdl_inputManager_term () {
	free (im);
}