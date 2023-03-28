#ifndef Included_Android_Asset
#define Included_Android_Asset

#include "../engine.hpp"

struct android_asset: public engine::asset_core {
	android_asset(AAssetManager*);
	~android_asset();
	engine::texture_core *texture_from_asset(const char *) override;
};

#endif //Included_Android_Asset