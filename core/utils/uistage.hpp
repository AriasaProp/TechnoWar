#ifndef Included_UISTAGE_
#define Included_UISTAGE_

#include <string>
#include "../engine.hpp"

namespace uistage {
  struct actor {
    float position[2], size[2];
    unsigned char color[4] = {0xff,0xff,0xff,0xff};
    std::string texKey;
    actor();
    ~actor();
  };
  void addTextureRegion(std::string,engine::texture_core*, float,float,float,float,bool);
  void act(float);
  void draw();
}

#endif //Included_UISTAGE_