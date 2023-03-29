#ifndef Included_Android_Asset
#define Included_Android_Asset

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "../engine.hpp"

struct android_asset: public engine::assets_core {
	android_asset(AAssetManager*);
	~android_asset();
	engine::asset_core *open_asset(const char *) override;
	unsigned int read_asset(engine::asset_core*, void*, unsigned int) override;
  void seek_asset(engine::asset_core*, int) override;
  bool eof_asset(engine::asset_core*) override;
	void close_asset(engine::asset_core *) override;
};

#endif //Included_Android_Asset