#include "android_asset.hpp"

#include "system/stb_image.hpp"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <cstdio>

AAssetManager *assetmanager;

struct a_asset: public engine::asset_core {
	AAsset *asset = nullptr;
	a_asset(const char *filename) {
    asset = AAssetManager_open(assetmanager, filename, AASSET_MODE_STREAMING);
	}
	unsigned int read(void *buff, unsigned int size) override {
		if (!asset) return 0;
		AAsset_read(asset, buff, size);
	}
	void seek(int n) override {
		if (!asset) return;
  	AAsset_seek(asset, n, SEEK_CUR);
	}
	bool eof() override {
		if (!asset) return true;
		return AAsset_getRemainingLength(asset) == 0;
	}
	~a_asset() {
		AAssetManager_close(asset);
	}
};

engine::asset_core *android_asset::open_asset(const char *filename) {
	return new a_asset(filename);
}
void android_asset::close_asset(engine::asset_core *a) {
	delete(static_cast<a_asset*>(a));
}

android_asset::android_asset(AAssetManager *mngr) {
	assetmanager = mngr;
	engine::asset = this;
}
android_asset::~android_asset() {
	
	engine::asset = nullptr;
}