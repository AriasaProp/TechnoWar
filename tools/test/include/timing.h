#ifndef TIMING_INCLUDED_
#define TIMING_INCLUDED_

#include <time.h>

struct timing {
  clock_t p;
};

extern struct timing start_timing();
extern float end_timing(struct timing);

#endif // TIMING_INCLUDED_