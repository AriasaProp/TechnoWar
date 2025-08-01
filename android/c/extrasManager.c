#include "manager.h"
#include "engine.h"

#include <sys/resource.h>
#include <time.h>

static void *androidExtras_timeStart() {
  return (void*) clock();
}
static float androidExtras_timeEnd(void *d) {
  return (float)(clock() - (clock_t)d) / (float)CLOCKS_PER_SEC;
}

void androidExtras_init (void) {
  global_engine.e.time_start_sec = androidExtras_timeStart;
  global_engine.e.time_end_sec = androidExtras_timeEnd;
}
void androidExtras_term (void) {
  memset(&global_engine.e, 0, sizeof(struct engine_extras));
}