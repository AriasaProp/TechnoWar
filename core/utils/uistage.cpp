#include "uistage.hpp"
#include <unordered_map>
#include <unordered_set>

namespace uistage {
  struct textureAtlas {
    bool isNinePatch;
    engine::texture_core *tex;
    float x1, y1, x2, y2; //offset 0,1
  };
  static std::unordered_map<std::string, textureAtlas> regions;
  static std::unordered_set<actor*> actors;
  actor::actor() {
    actors.insert(this);
  }
  actor::~actor() {
    auto it = actros.find(this);
    if (it != actors.end())
      actors.erase(it);
  }
  
  void addTextureRegion(std::string key, engine::texture_core *tex, float x1, float y1, float x2, float y2, bool ninePatch = false) {
    regions[key] = textureAtlas{ninePatch, tex, x1,y1,x2,y2};
  }
  void act (float delta) {
    //nothing to do
  }
  void draw () {
    
    engine::graph->flat_render(/*tex*/,/**/,/**/)
  }
}
