#ifndef Included_TGLES
#define Included_TGLES 1

#include "translatedGraphicsFunction.h"

struct tgf_gles : public TranslatedGraphicsFunction {
private:
	bool valid = false;
	int *temp = 0;
	unsigned int *utemp = 0;
	char *msg = 0;
public:
	tgf_gles();
	~tgf_gles() override;
	const char *renderer() override;
	void clearcolormask(const unsigned int&, const float&, const float&, const float&, const float&) override;
	void viewport(const int&, const int&, const int&, const int&) override;

	void ui_draw_funct() override;
	
	shader_core *gen_shader(const char*, const char*) override;
	void bind_shader(shader_core*) override;
	void delete_shader(shader_core*) override;
	int get_shader_uloc(shader_core*, const char *) override;
	void u_matrix4fv(const int&,const int&, const bool&, const float *) override;

	void gen_buffer(unsigned int&) override;
	void bind_buffer(unsigned int, const unsigned int) override;
	void buffer_data(unsigned int, long, const void*, unsigned int) override;
	void delete_buffer(unsigned int&) override;

	texture_core *gen_texture(const int&, const int&, unsigned char*) override;
	void bind_texture(texture_core *) override;
	void set_texture_param(const int&, const int&) override;
	void delete_texture(texture_core *) override;
	
	void gen_vertex_array(unsigned int&) override;
	void bind_vertex_array(const unsigned int) override;
	void delete_vertex_array(unsigned int&) override;
	
	mesh_core *gen_mesh(mesh_core::data*,unsigned int, unsigned short*,unsigned int) override;
	void update_mesh(mesh_core*, mesh_core::data*,unsigned int, unsigned short*,unsigned int) override;
	void draw_mesh(mesh_core*) override;
	void delete_mesh(mesh_core*) override;

	void vertex_attrib_pointer(unsigned int, int, unsigned int type, bool, int, const void *) override;
	void enable_vertex_attrib_array(const unsigned int) override;
	void draw_elements(int, unsigned int, int, const void *) override;
	
	// Android may lost resources
	void validate();
	void invalidate();
};

#endif // Included_TGLES
