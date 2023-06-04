#ifndef _Included_OPENGLES_Graphics
#define _Included_OPENGLES_Graphics 1

#include "android_graphics.hpp"

struct opengles_graphics : public android_graphics {
private:
  struct gl_data *mgl_data;
public:
  // android
  void preRender (ANativeWindow*, unsigned int&) override;
  void postRender (bool) override;
  void onWindowTerm () override;
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
  inline void update_layout ();

  opengles_graphics ();
  ~opengles_graphics ();
};
#endif //_Included_OPENGLES_Graphics