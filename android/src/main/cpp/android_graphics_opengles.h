#ifndef Included_Android_Graphics_Opengles
#define Included_Android_Graphics_Opengles 1

#include "android_graphics.h"

struct android_graphics_opengles : public android_graphics {
	android_graphics_opengles();
	~android_graphics_opengles() override;
	
	float getWidth() override;
	float getHeight() override;
	
	void clear(const unsigned int&) override;
	void clearcolor(const float&, const float&, const float&, const float&) override;

	texture_core *gen_texture(const int&, const int&, unsigned char*) override;
	void bind_texture(texture_core *) override;
	void set_texture_param(const int&, const int&) override;
	void delete_texture(texture_core *) override;
	
	void flat_render(flat_vertex *, unsigned int) override;
	
	mesh_core *gen_mesh(mesh_core::data*,unsigned int, unsigned short*,unsigned int) override;
	void mesh_render(mesh_core**,const unsigned int&) override;
	void delete_mesh(mesh_core*) override;
	
	//for android
	
	void resize_viewport(const int, const int) override;
	// Android may lost resources
	void validate() override;
	void invalidate() override;
};

#endif // Included_Android_Graphics_Opengles
