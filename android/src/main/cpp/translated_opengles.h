#ifndef Included_TGLES
#define Included_TGLES 1

#include "translatedGraphicsFunction.h"

struct tgf_gles : public TranslatedGraphicsFunction {
	tgf_gles();
	~tgf_gles() override;
	const char *renderer() override;
	void clearcolor(const float&, const float&, const float&, const float&) override;
	void clear(const unsigned int&) override;
	void viewport(const int&, const int&, const int&, const int&) override;

	texture_core *gen_texture(const int&, const int&, unsigned char*) override;
	void bind_texture(texture_core *) override;
	void set_texture_param(const int&, const int&) override;
	void delete_texture(texture_core *) override;
	
	mesh_core *gen_mesh(mesh_core::data*,unsigned int, unsigned short*,unsigned int) override;
	void world_mesh(float,float) override;
	void begin_mesh() override;
	void draw_mesh(mesh_core*) override;
	void end_mesh() override;
	void delete_mesh(mesh_core*) override;

	// Android may lost resources
	void validate();
	void invalidate();
};

#endif // Included_TGLES
