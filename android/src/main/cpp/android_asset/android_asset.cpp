#include "android_asset.hpp"

#include "system/stb_image.hpp"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <cstdio>

struct a_asset: public engine::asset_core {
	AAsset *asset;
	a_asset(AAsset *a): asset(a) {}
	~a_asset() { AAsset_close(asset); }
};
engine::asset_core *android_asset::open_asset(const char *filename) {
	return new a_asset(AAssetManager_open(assetmanager, filename, AASSET_MODE_UNKNOWN));
}
unsigned int android_asset::read_asset(engine::asset_core *a, void *buff, unsigned int len)  {
	if (!a) return 0;
	return AAsset_read(static_cast<a_asset*>(a)->asset, buff, len);
}
void android_asset::seek_asset(engine::asset_core *a, int n)  {
	if (!a) return;
	AAsset_seek(static_cast<a_asset*>(a)->asset, n, SEEK_CUR);
}
bool android_asset::eof_asset(engine::asset_core *a)  {
	if (!a) return true;
	return AAsset_getRemainingLength(static_cast<a_asset*>(a)->asset) == 0;
}
void android_asset::close_asset(engine::asset_core *a) {
	delete(static_cast<a_asset*>(a));
}
android_asset::android_asset(AAssetManager *mngr): assetmanager(mngr) {
	engine::asset = this;
}
android_asset::~android_asset() {
	engine::asset = nullptr;
}