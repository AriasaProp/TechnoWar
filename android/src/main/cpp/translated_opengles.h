#ifndef Included_TGLES
#define Included_TGLES

#include "translatedGraphicsFunction.h"

class tgf_gles : public TranslatedGraphicsFunction {
public:
	~tgf_gles() {}
	const char *renderer() override;
	void clearcolormask(unsigned int, float, float, float, float) override;
	void viewport(unsigned int, unsigned int, unsigned int, unsigned int) override;

	void gen_shader(unsigned int*, const char*, const char*) override;
	void bind_shader(unsigned int*) override;
	void delete_shader(unsigned int*) override;

	void gen_buffer(unsigned int*) override;
	void bind_buffer(int, unsigned int*) override;
	void buffer_data(int, unsigned int, const unsigned char *, int) override;
	void delete_buffer(unsigned int*) override;

	void gen_vertex_array(unsigned int*) override;
	void bind_vertex_array(unsigned int*) override;
	void delete_vertex_array(unsigned int*) override;

	void vertex_attrib_pointer(unsigned int, unsigned int, int type, bool, void *) override;
	void enable_vertex_attrib_array(unsigned int) override;
	void draw_elements(int, unsigned int, int, unsigned int) override;
};

#endif // Included_TGLES
