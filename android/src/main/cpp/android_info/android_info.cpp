#include "android_info.hpp"

#include <sys/resource.h>
#include <sys/time.h>
#include <string>

struct ty {
	struct rusage usage;
	int sdk_version;
  std::string tmp;
};

android_info::android_info (int sdk) {
  engine::info = this;
  instance = (void*)new ty;
  ((ty*)instance)->sdk_version = sdk;
}
android_info::~android_info () {
  delete (ty*)instance;
  engine::info = nullptr;
}
const char *android_info::get_platform_info() {
  ((ty*)instance)->tmp = "Android SDK " + std::to_string(sdk_version);
	return ((ty*)instance)->tmp.c_str();
}

long android_info::memory () {
  if (getrusage (RUSAGE_SELF, &(((ty*)instance)->usage)) < 0)
    return -1;
  return usage.ru_maxrss;
}