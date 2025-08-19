#include "common.h"
#include "engine.h"
#include <android/asset_manager.h>
#include <string.h>

static AAssetManager *mngr = NULL;

static void *assetBuffer(const char *filename, const void **buf, size_t *len) {
  AAsset *reading = AAssetManager_open(mngr, filename, AASSET_MODE_BUFFER);
  *len = (size_t)AAsset_getLength(reading);
  *buf = AAsset_getBuffer(reading);
  return (void *)reading;
}
static void *openAsset(const char *filename) {
  return (void *)AAssetManager_open(mngr, filename, AASSET_MODE_STREAMING);
}
static int assetRead(void *a, void *buf, size_t count) {
  return AAsset_read((AAsset *)a, buf, count);
}
static void assetSeek(void *a, int l) {
  AAsset_seek((AAsset *)a, l, SEEK_CUR);
}
static size_t assetLength(void *a) {
  return (size_t)AAsset_getRemainingLength64((AAsset *)a);
}
static void assetClose(void *a) {
  AAsset_close((AAsset *)a);
}

void androidAssetManager_init(void *m) {
  mngr = (AAssetManager *)m;

  global_engine.assetBuffer = assetBuffer;
  global_engine.openAsset = openAsset;
  global_engine.assetRead = assetRead;
  global_engine.assetSeek = assetSeek;
  global_engine.assetLength = assetLength;
  global_engine.assetClose = assetClose;
}
void androidAssetManager_term(void) {
  mngr = NULL;
}