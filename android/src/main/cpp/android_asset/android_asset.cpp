#include "android_asset.hpp"

#include <cstdio>
#include <cstring>
#include <memory>

struct a_asset : public engine::asset_core {
  AAsset *asset;
  a_asset (AAsset *a) : asset (a) {}
  int read (void *buff, unsigned int len) override {
    return AAsset_read (asset, buff, len);
  }
  void seek (int n) override {
    AAsset_seek (asset, n, SEEK_CUR);
  }
  bool eof () override {
    return AAsset_getRemainingLength (asset) <= 0;
  }
  ~a_asset () {
    AAsset_close (asset);
  }
};
engine::asset_core *android_asset::open_asset (const char *filename) {
  return new a_asset (AAssetManager_open (assetmanager, filename, AASSET_MODE_STREAMING));
}
void *android_asset::asset_buffer (const char *filename, unsigned int *o) {
  AAsset *asset = AAssetManager_open (assetmanager, filename, AASSET_MODE_BUFFER);
  unsigned int *outLen = (o ? o : new unsigned int);
  *outLen = AAsset_getLength (asset);
  void *result = malloc (*outLen);
  memcpy (result, AAsset_getBuffer (asset), *outLen);
  AAsset_close (asset);
  if (!o) delete outLen;
  return result;
}
android_asset::android_asset (AAssetManager *mngr) : assetmanager (mngr) {
  engine::asset = this;
}
android_asset::~android_asset () {
  engine::asset = nullptr;
}