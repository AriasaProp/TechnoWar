#include "engine.h"

static struct engine engine_instance;

struct engine *get_engine() {
	return &engine_instance;
}