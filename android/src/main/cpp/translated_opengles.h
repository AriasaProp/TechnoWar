#ifndef Included_TGLES
#define Included_TGLES

#include "translatedGraphicsFunction.h"

class tgf_gles : public TranslatedGraphicsFunction {
public:
	tgf_gles();
	~tgf_gles() override;
	const char *renderer() override;
	void clearcolormask(unsigned int, float, float, float, float) override;
	void viewport(unsigned int, unsigned int, unsigned int, unsigned int) override;
	unsigned int gen_shader(const char*, const char*) override;
	void delete_shader(const unsigned int) override;
	
	unsigned int gen_buffer() override;
	void bind_buffer(int, const unsigned int) override;
	void buffer_data(int, const unsigned int, const unsigned char *, int) override;
	unsigned int gen_vertex_array() override;
	void bind_vertex_array(const unsigned int) override;
};

#endif // Included_TGLES
