#ifndef Included_Android_Info
#define Included_Android_Info

#include "../engine.hpp"

struct android_info : public engine::info_core {
private:
  void *instance;

public:
  android_info ();
  ~android_info ();
  const char *get_platform_info () override;
  long memory () override;
};

#endif // Included_Android_Info