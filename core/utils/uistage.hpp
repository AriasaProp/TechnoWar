#ifndef Included_UISTAGE_
#define Included_UISTAGE_

#include <string>
#include "../engine.hpp"

namespace uistage {
  struct actor {
    float pos[2], size[2];
    unsigned char color[4] = {0xff,0xff,0xff,0xff};
    std::string texKey;
    actor(float[2],float[2], unsigned char[4], std::string);
    ~actor();
  };
  struct texture_region {
    unsigned int pos[2], size[2];
    unsigned int patch[4];
  };
  void addTextureRegion(std::string,engine::texture_core*, const texture_region &);
  void act(float);
  void draw();
  void clear();
}

#endif //Included_UISTAGE_