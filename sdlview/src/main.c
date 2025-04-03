#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_opengles2.h>

#include "engine.h"
#include "core.h"
#include "manager.h"
#include "log.h"

#define SCREEN_WIDTH 2440
#define SCREEN_HEIGHT 800

#define check(X) X
#define checkCompileShader(X) glCompileShader (X)
#define checkLinkProgram(X) glLinkProgram (X)


int main(int argc, char *argv[]) {

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		LOGE("SDL failed to init %s\n", SDL_GetError());
		goto end;
	}

	//profile
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	//gl feature
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	// gl quality
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	
	SDL_Window *window = SDL_CreateWindow("Hello SDL2 OpenGL ES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
	if (!window) {
		LOGE("Failed to make sdl window: %s\n", SDL_GetError());
		goto end_sdl;
	}
	SDL_GLContext *context = SDL_GL_CreateContext(window);
	if (!context) {
		LOGE("Failed to make sdl context: %s\n", SDL_GetError());
		goto end1_sdl;
	}

	// on start, initialize
	engine_init();
	sdl_inputManager_init();
	sdl_graphicsManager_init();
	sdl_graphicsManager_resize (SCREEN_WIDTH, SCREEN_HEIGHT);
	 // flat draw
	GLint shader = 0;
	GLuint vbo = 0, ibo = 0;
  {
    shader = check (glCreateProgram ());
    GLuint vi = check (glCreateShader (GL_VERTEX_SHADER));
    const char *vt = "#version 320 es"
                     "\n#ifdef GL_FRAGMENT_PRECISION_HIGH"
                     "\n		precision highp float;"
                     "\n#else"
                     "\n		precision mediump float;"
                     "\n#endif"
                     "\nlayout (location = 0) in vec4 a_position;"
                     "\nvoid main() {"
                     "\n    gl_Position = a_position;"
                     "\n}";
    check (glShaderSource (vi, 1, &vt, 0));
    checkCompileShader (vi);
    check (glAttachShader (shader, vi));
    GLuint fi = check (glCreateShader (GL_FRAGMENT_SHADER));
    const char *ft = "#version 320 es"
                     "\n#ifdef GL_FRAGMENT_PRECISION_HIGH"
                     "\n		precision highp float;"
                     "\n#else"
                     "\n		precision mediump float;"
                     "\n#endif"
                     "\nout vec4 fragColor;"
                     "\nvoid main() {"
                     "\n    fragColor = vec4(1.0, 1.0, 1.0, 1.0);"
                     "\n}";
    check (glShaderSource (fi, 1, &ft, 0));
    checkCompileShader (fi);
    check (glAttachShader (shader, fi));
    checkLinkProgram (shader);
    check (glDeleteShader (vi));
    check (glDeleteShader (fi));
    check (glGenBuffers (2, &vbo));
    unsigned short indexs[6];
    for (unsigned short i = 0, j = 0, k = 0; i < 1; i++, j += 6) {
      indexs[j] = k++;
      indexs[j + 1] = indexs[j + 5] = k++;
      indexs[j + 2] = indexs[j + 4] = k++;
      indexs[j + 3] = k++;
    }
    // 0, 1, 2, 3, 2, 1
    check (glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ibo));
    check (glBufferData (GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof (unsigned short), (void *)indexs, GL_STATIC_DRAW));
    check (glBindBuffer (GL_ARRAY_BUFFER, vbo));
    struct vec2 verts[4] = {
      (struct vec2){0.5f, -0.5f},// Bottom-right
    	(struct vec2){0.5f, -0.5f},// Top-right
    	(struct vec2){-0.5f, 0.5f},// Bottom-left
    	(struct vec2){-0.5f, 0.5f},// Top-left
    };

    check (glBufferData (GL_ARRAY_BUFFER, 4 * sizeof (struct vec2), (void *)verts, GL_DYNAMIC_DRAW));
  }
  
	do {
		sdl_graphicsManager_preRender();
		
		Main_update(); 
		{
			check (glUseProgram (shader));
		  
		  check (glBindBuffer (GL_ARRAY_BUFFER, vbo));
		  
		  check (glEnableVertexAttribArray (0));
		  check (glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, sizeof (struct vec2), (void *)0));
		  
		  check (glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ibo));
		  check (glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL));
		  check (glUseProgram (0));
		  check (glBindBuffer (GL_ARRAY_BUFFER, 0));
		  check (glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0));
		  check (glBindTexture (GL_TEXTURE_2D, 0));
		}
		
		sdl_graphicsManager_postRender();
		
		SDL_GL_SwapWindow(window);
	} while (sdl_inputManager_update());
	Main_pause();
	Main_term();
	
	sdl_graphicsManager_term();
	sdl_inputManager_term();
	
	SDL_GL_DeleteContext(context);
end1_sdl:
	SDL_DestroyWindow(window);
end_sdl:
	SDL_Quit();
end:
	return 0;
}