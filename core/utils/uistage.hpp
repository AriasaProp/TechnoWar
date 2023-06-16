#ifndef Included_UISTAGE_
#define Included_UISTAGE_

#include <string>
#include "../engine.hpp"
#include "math.hpp"

namespace uistage {
  struct actor {
    Rect rectangle; 
    unsigned char color[4];
    std::string texKey;
  };
  struct texture_region {
    unsigned int pos[2], size[2];
    unsigned int patch[4];
  };
  //void addBMFont();
  void addTextureRegion(std::string,engine::texture_core*, const texture_region &);
  void act(float);
  void draw();
  void clear();
  
  //all actor types
  actor *makeImage(std::string,Rect);
}

#endif //Included_UISTAGE_