#ifndef Included_Android_Info
#define Included_Android_Info

#include "../engine.hpp"

struct android_info : public engine::info_core {
private:
public:
  long memory () override;
};

#endif // Included_Android_Info