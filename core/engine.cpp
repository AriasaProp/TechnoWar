#include "engine.h"

namespace engine {
	//graphics
  float (*getWidth)() = 0;
  float (*getHeight)() = 0;
  void (*clear)(const unsigned int&) = 0;
  void (*clearcolor)(const float&, const float&, const float&, const float&) = 0;
  texture_core *(*gen_texture)(const int&, const int&, unsigned char*) = 0;
  void (*bind_texture)(texture_core*) = 0;
  void (*set_texture_param)(const int&, const int&) = 0;
  void (*delete_texture)(texture_core*) = 0;
  void (*flat_render)(flat_vertex*, unsigned int) = 0;
  mesh_core *(*gen_mesh)(mesh_core::data*, unsigned int, unsigned short*, unsigned int) = 0;
  void (*mesh_render)(mesh_core**, const unsigned int&) = 0;
  void (*delete_mesh)(mesh_core*) = 0;
  //input
	float *(*getSensorValue)(const char*) = 0;
	int (*getX)(unsigned int) = 0;
	int (*getDeltaX)(unsigned int) = 0;
	int (*getY)(unsigned int) = 0;
	int (*getDeltaY)(unsigned int) = 0;
	bool (*justTouched)() = 0;
	bool (*isTouched)(unsigned int) = 0;
	float (*getPressure)(unsigned int) = 0;
	bool (*isButtonPressed)(int button) = 0;
	bool (*isButtonJustPressed)(int button) = 0;
	bool (*isKeyPressed)(int key) = 0;
	bool (*isKeyJustPressed)(int key) = 0;
	void (*process_event)() = 0;
}