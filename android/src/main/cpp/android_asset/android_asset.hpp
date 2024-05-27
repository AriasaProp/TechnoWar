#ifndef Included_Android_Asset
#define Included_Android_Asset

#include "../engine.hpp"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

struct android_asset : public engine::assets_core {
private:
  AAssetManager *assetmanager;

public:
  android_asset (AAssetManager *);
  ~android_asset ();
  engine::asset_core *open_asset (const char *) override;
  void *asset_buffer (const char *, unsigned int *) override;
};

#endif // Included_Android_Asset