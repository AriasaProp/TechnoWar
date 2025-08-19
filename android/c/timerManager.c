#include "common.h"
#include "engine.h"

#include <string.h>
#include <time.h>

static struct {
  size_t counted_frame, lastFPS;
  clock_t deltaTime, lastSecond, lastFrame, tempClock;
} src;

static const float deltaConst = 1000.f / CLOCKS_PER_SEC;
static float getDeltaTimeMs(void) {
  return (float)src.deltaTime * deltaConst;
}
static size_t getFPS(void) {
  return src.lastFPS;
}

void androidTimerManager_init(void) {
  memset(&src, 0, sizeof(src));
  global_engine.getDeltaTimeMs = getDeltaTimeMs;
  global_engine.getFPS = getFPS;

  src.lastFrame = src.lastSecond = clock();
}

void androidTimerManager_onFrame(void) {
  src.tempClock = clock();
  // frame count
  ++src.counted_frame;
  if ((src.tempClock - src.lastSecond) >= CLOCKS_PER_SEC) {
    src.lastSecond = src.tempClock;
    src.lastFPS = src.counted_frame;
    src.counted_frame = 0;
  }
  // delta time
  src.deltaTime = src.tempClock - src.lastFrame;

  src.lastFrame = src.tempClock;
}

void androidTimerManager_term(void) {
  memset(&src, 0, sizeof(src));
}
