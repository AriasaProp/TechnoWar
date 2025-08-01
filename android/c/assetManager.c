#include <android/asset_manager.h>

#include "common.h"
#include "engine.h"

static AAssetManager *mngr = NULL;

static AAsset *reading[MAX_ASSET_READING] = {0};

static int assetBuffer(const char *filename, const void **buf, int *len) {
  for (size_t i = 0; i < MAX_ASSET_READING; ++i) {
    if (!reading[i]) {
      reading[i] = AAssetManager_open(mngr, filename, AASSET_MODE_BUFFER);
      *len = AAsset_getLength(reading[i]);
      *buf = AAsset_getBuffer(reading[i]);
      return i;
    }
  }
  return -1;
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

  global_engine.a.assetBuffer = assetBuffer;
  global_engine.a.openAsset = openAsset;
  global_engine.a.assetRead = assetRead;
  global_engine.a.assetLength = assetLength;
  global_engine.a.assetClose = assetClose;
}
void androidAssetManager_term() {
  mngr = NULL;

  global_engine.a.assetBuffer = NULL;
  global_engine.a.openAsset = NULL;
  global_engine.a.assetRead = NULL;
  global_engine.a.assetLength = NULL;
  global_engine.a.assetClose = NULL;
}