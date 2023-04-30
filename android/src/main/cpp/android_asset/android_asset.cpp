#include "android_asset.hpp"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <cstdio>

struct a_asset: public engine::asset_core {
	AAsset *asset;
	a_asset(AAsset *a): asset(a) {}
	int read(void *buff, unsigned int len) override {
	  return AAsset_read(asset, buff, len);
	}
  void seek(int n) override {
	  AAsset_seek(asset, n, SEEK_CUR);
  }
  bool eof() override {
	  return AAsset_getRemainingLength(asset) == 0;
  }
	~a_asset() {
	  if (!eof()) AAsset_close(asset);
	}
};
engine::asset_core *android_asset::open_asset(const char *filename) {
  AAsset *a = AAssetManager_open(assetmanager, filename, AASSET_MODE_UNKNOWN);
	return new a_asset(a);
}
android_asset::android_asset(AAssetManager *mngr): assetmanager(mngr) {
	engine::asset = this;
}
android_asset::~android_asset() {
	engine::asset = nullptr;
}