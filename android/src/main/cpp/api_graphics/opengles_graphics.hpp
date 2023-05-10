#ifndef _Included_OPENGLES_Graphics
#define _Included_OPENGLES_Graphics

#include "android_graphics.hpp"
#include "../main_game.hpp"
#include <EGL/egl.h>
#include <unordered_set>

struct opengles_graphics: public android_graphics {
private:
	ANativeWindow *window;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	EGLConfig eConfig;
	std::unordered_set<engine::texture_core*> managedTexture;
	std::unordered_set<engine::mesh_core*> managedMesh;
	EGLint width, height;
public:
	//android
	void onResume() override;
	void onWindowInit(ANativeWindow*) override;
	void needResize() override;
	void render() override;
	void onWindowTerm() override;
	void onPause() override;
	void onDestroy() override;
	//core
	float getWidth() override;
  float getHeight() override;
  void clear(const unsigned int&) override;
  void clearcolor(const float&, const float&, const float&, const float&) override;
  engine::texture_core* gen_texture(const int&, const int&, unsigned char*) override;
  void bind_texture(engine::texture_core*) override;
  void set_texture_param(const int&, const int&) override;
  void delete_texture(engine::texture_core*) override;
  void flat_render(engine::texture_core*, engine::flat_vertex*, unsigned int) override;
  engine::mesh_core* gen_mesh(engine::mesh_core::data*, unsigned int, unsigned short*, unsigned int) override;
  void mesh_render(engine::mesh_core**, const unsigned int&) override;
  void delete_mesh(engine::mesh_core*) override;
  inline void resize_viewport(const int, const int);
  
  opengles_graphics();
  ~opengles_graphics();
  
};
#endif //_Included_OPENGLES_Graphics