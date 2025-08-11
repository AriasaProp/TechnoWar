#include <android/asset_manager.h>

#include "common.h"
#include "engine.h"

static AAssetManager *mngr = NULL;

static AAsset *reading[MAX_ASSET_READING] = {0};

void assetBuffer(const char *filename, void *buf, int *len) {
  if (!mngr) return;
  AAsset *reading = AAssetManager_open(mngr, filename, AASSET_MODE_BUFFER);
  *len = AAsset_getLength(reading);
  memcpy(buf, AAsset_getBuffer(reading), *len);
  AAsset_close(reading);
}

static int openAsset(const char *filename) {
  for (size_t i = 0; i < MAX_ASSET_READING; ++i) {
    if (!reading[i]) {
      reading[i] = AAssetManager_open(mngr, filename, AASSET_MODE_STREAMING);
      return i;
    }
  }
  return -1;
}
static int assetRead(int a, void *buf, size_t count) {
  return AAsset_read(reading[a], buf, count);
}
static size_t assetLength(int a) {
  return AAsset_getRemainingLength(reading[a]);
}
static void assetClose(int a) {
  AAsset_close(reading[a]);
  reading[a] = NULL;
}

void androidAssetManager_init(void *m) {
  mngr = (AAssetManager *)m;

  global_engine.assetBuffer = assetBuffer;
  global_engine.openAsset = openAsset;
  global_engine.assetRead = assetRead;
  global_engine.assetLength = assetLength;
  global_engine.assetClose = assetClose;
}
void androidAssetManager_term() {
  mngr = NULL;
}