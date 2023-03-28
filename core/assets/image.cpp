#include "image.hpp"
#include "../system/stb_image.hpp"

image::image() {
	unsigned char data[16] {
		0xff, 0xff, 0xff, 0xff,
		0x00, 0x00, 0x00, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0x00, 0x00, 0x00, 0xff
	};
	core = engine::graph->gen_texture(2,2,data);
}
image::image(const char *filename) {
	engine::asset_core *ac = engine::asset->open_asset(filename);
	const stbi_io_callbacks io {
		[](void *user, char *data, int size) -> int {
			return static_cast<engine::asset_core*>(user)->read(data, size);
		},
		[](void *user, int n) -> void {
			static_cast<engine::asset_core*>(user)->seek(n);
		},
		[](void *user) -> bool {
			return static_cast<engine::asset_core*>(user)->eof();
		}
	};
	int width, height;
	unsigned char *data = stbi_load_from_callbacks(io, &ac, &width, &height, nullptr, STBI_rgb_alpha);
	engine::asset->close_asset(ac);
	core = engine::graph->gen_texture(width, height, data);
	stbi_image_free(data);
}
engine::texture_core *image::getCore() {
	return core;
}
image::~image() {
	engine::graph->delete_texture(core);
}
