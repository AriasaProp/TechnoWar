#ifndef _Included_OPENGLES_Graphics
#define _Included_OPENGLES_Graphics

#include "android_graphics.h"
#include "../main_game.h"
#include <unordered_set>

struct ui_batch {
	bool dirty_projection;
	int shader;
	int u_projection;
	unsigned int vao, vbo, ibo;
	float ui_projection[16];
};
struct world_btch {
	bool dirty_worldProj;
	int shader;
	int u_worldProj;
	int u_transProj;
	float worldProj[16];
};

struct opengles_graphics: public android_graphics {
private:
	std::unordered_set<engine::texture_core*> managedTexture;
	std::unordered_set<engine::mesh_core*> managedMesh;
	bool valid = false;
	int *temp;
	unsigned int *utemp;
	char *msg;
	float width, height;
	ui_batch *ubatch;
	world_btch *ws;
	ANativeWindow *window;
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  EGLConfig eConfig; 
public:
	//android
	void onResume() override;
	void onWindowInit(ANativeWindow*) override;
	void needResize() override;
	void render() override;
	void onWindowTerm() override;
	void onPause() override;
	void onDestroy() override;
	~opengles_graphics() override;
	opengles_graphics();
	//core
	float getWidth() override;
  float getHeight() override;
  void clear(const unsigned int&) override;
  void clearcolor(const float&, const float&, const float&, const float&) override;
  texture_core* gen_texture(const int&, const int&, unsigned char*) override;
  void bind_texture(texture_core*) override;
  void set_texture_param(const int&, const int&) override;
  void delete_texture(texture_core*) override;
  void flat_render(flat_vertex*, unsigned int) override;
  mesh_core* gen_mesh(mesh_core::data*, unsigned int, unsigned short*, unsigned int) override;
  void mesh_render(mesh_core**, const unsigned int&) override;
  void delete_mesh(mesh_core*) override;
  
  opengles_graphics();
  ~opengles_graphics();
};
#endif //_Included_OPENGLES_Graphics