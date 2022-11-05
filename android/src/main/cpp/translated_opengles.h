#ifndef Included_TGLES
#define Included_TGLES

#include "translatedGraphicsFunction.h"

class tgf_gles : public TranslatedGraphicsFunction {
public:
	tgf_gles();
	~tgf_gles() override;
	const char *renderer() override;
	void clearcolormask(const unsigned int&, const float&, const float&, const float&, const float&) override;
	void viewport(const unsigned int&, const unsigned int&, const unsigned int&, const unsigned int&) override;

	void gen_shader(unsigned int&, const char*, const char*) override;
	void bind_shader(const unsigned int&) override;
	void delete_shader(unsigned int&) override;

	void gen_buffer(unsigned int*) override;
	void bind_buffer(unsigned int, unsigned int*) override;
	void buffer_data(unsigned int, long, const void*, unsigned int) override;
	void delete_buffer(unsigned int*) override;

	void gen_vertex_array(unsigned int*) override;
	void bind_vertex_array(unsigned int*) override;
	void delete_vertex_array(unsigned int*) override;

	void vertex_attrib_pointer(unsigned int, int, unsigned int type, bool, int, const void *) override;
	void enable_vertex_attrib_array(unsigned int) override;
	void draw_elements(int, unsigned int, int, const void *) override;
};

#endif // Included_TGLES
