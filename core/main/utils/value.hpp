#ifndef VALUE_
#define VALUE_

#include <cstdint>

//common value type
union color32_t {
	struct {
    uint8_t r, g, b, a;
  } rgba;
  uint32_t color;
};



#endif //VALUE_