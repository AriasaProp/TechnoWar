#ifndef Included_UISTAGE_
#define Included_UISTAGE_

#include <string>
#include "../engine.hpp"

namespace uistage {
  struct actor {
    float pos[2], size[2];
    unsigned char color[4];
    std::string texKey;
    actor(float p[2],float s[2], unsigned char c[4] = {0xff,0xff,0xff,0xff}, std::string tex = ""): pos(p), size(s), color(c), texKey(tex);
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