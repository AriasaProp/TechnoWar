#ifndef MANAGER_
#define MANAGER_

extern void sdl_inputManager_init();
extern int sdl_inputManager_update();
extern void sdl_inputManager_term ();

extern void sdl_graphicsManager_init();
extern void sdl_graphicsManager_resize(unsigned int, unsigned int);
extern void sdl_graphicsManager_preRender();
extern void sdl_graphicsManager_postRender();
extern void sdl_graphicsManager_term();

/*
extern void sdl_assetManager_init();
extern void sdl_assetManager_term();
*/
#endif // MANAGER_