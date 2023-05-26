#ifndef _Included_vulkan_graphics
#define _Included_vulkan_graphics

#include "../main_game.hpp"
#include "android_graphics.hpp"

struct vulkan_graphics : public android_graphics {
private:
  bool pause, resume, resize, relayout, destroyed;
public:
  // android
  void onResume () override;
  void onWindowInit (ANativeWindow *) override;
  void needResize () override;
  void needLayout () override;
  void render () override;
  void onWindowTerm () override;
  void onPause () override;
  // core
  float getWidth () override;
  float getHeight () override;
  void clear (const unsigned int &) override;
  void clearcolor (const float &, const float &, const float &, const float &) override;
  engine::texture_core *gen_texture (const int &, const int &, unsigned char *) override;
  void bind_texture (engine::texture_core *) override;
  void set_texture_param (const int &, const int &) override;
  void delete_texture (engine::texture_core *) override;
  void flat_render (engine::texture_core *, engine::flat_vertex *, unsigned int) override;
  engine::mesh_core *gen_mesh (engine::mesh_core::data *, unsigned int, unsigned short *, unsigned int) override;
  void mesh_render (engine::mesh_core **, const unsigned int &) override;
  void delete_mesh (engine::mesh_core *) override;

  vulkan_graphics ();
  ~vulkan_graphics ();
};
#endif //_Included_vulkan_graphics