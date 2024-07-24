#include "android_info.hpp"

#include <sys/resource.h>
#include <sys/time.h>

struct rusage usage;

android_info::android_info () {
  engine::info = this;
}
android_info::~android_info () {

  engine::info = nullptr;
}

long android_info::memory () {
  if (getrusage (RUSAGE_SELF, &usage) < 0)
    return -1;
  return usage.ru_maxrss;
}