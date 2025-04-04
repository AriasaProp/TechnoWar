#include "engine.h"
#include <string.h>

static struct engine engine_instance;
struct engine *engine_init() {
  memset(&engine_instance, 0, sizeof(struct engine));
  return &engine_instance;
}
struct engine *get_engine() {
  return &engine_instance;
}