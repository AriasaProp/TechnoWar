#ifndef Included_UISTAGE_
#define Included_UISTAGE_

#include <string>
#include "../engine.hpp"
#include "math.hpp"

namespace uistage {
  struct actor {
    Point<float,2> position, size;
    float color = 0xffffffff;
    std::string texKey;
    actor();
    ~actor();
  };
  void addTextureRegion(std::string,engine::texture_core*, float,float,float,float,bool);
  void act(float);
  void draw();
}

#endif //Included_UISTAGE_