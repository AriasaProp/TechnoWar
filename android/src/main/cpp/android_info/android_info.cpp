#include "android_info.hpp"

#include <sys/resource.h>
#include <sys/time.h>

struct rusage usage;

long android_info::memory() {
	if (getrusage(RUSAGE_SELF, &usage) < 0)
		return -1;
	return usage.ru_maxrss;
}