#include "uistage.hpp"
#include "math.hpp"
#include <unordered_map>
#include <unordered_set>

struct textureAtlas {
  engine::texture_core *tex;
  uistage::texture_region region;
};
static std::unordered_map<std::string, textureAtlas> regions;
static std::unordered_set<uistage::actor*> actors;

uistage::actor::actor(){
  actors.insert(this);
}
uistage::actor::~actor() {
  auto it = actors.find(this);
  if (it != actors.end())
    actors.erase(it);
}

/*
void uistage::loadoutSkin(const char *filename) {
  
}
*/

void uistage::addTextureRegion(std::string key, engine::texture_core *tex, const uistage::texture_region &reg) {
  regions[key] = textureAtlas{tex, reg};
}

void uistage::act (float) {
  //nothing to do
}
static engine::flat_vertex vert[1024];
void uistage::draw () {
  float x1, y1, x2, y2;
  for (auto i = actors.begin(), j = actors.end(); i != j; i++) {
    engine::flat_vertex *verts = vert;
    actor &a = **i;
    x1 = a.pos[0];
    y1 = a.pos[1];
    x2 = x1 + a.size[0];
    y2 = y1 + a.size[1];
    
    textureAtlas &ta = regions[a.texKey];
    engine::texture_core *tex = ta.tex;
    
    // left , top, right, bottom
    const unsigned int *split = ta.region.patch;
    
    bool patched = (split[0] || split[1] || split[2] || split[3]);
    unsigned char cl[4]{0xff, 0xff,patched?0xff:0x00  ,0xff};
    //bottom - left
    *(verts++) = {x1, y1, {cl[0], cl[1], cl[2], cl[3]}, 0, 1};
    if (patched) {
      *(verts++) = {x1, y1+split[3], {cl[0], cl[1], cl[2], cl[3]}, 0, 1-float(split[3]/ta.region.size[1])};
      *(verts++) = {x1+split[0], y1, {cl[0], cl[1], cl[2], cl[3]}, float(split[0]/ta.region.size[0]), 1};
      *(verts++) = {x1+split[0], y1+split[3], {cl[0], cl[1], cl[2], cl[3]}, float(split[0]/ta.region.size[0]), 1-float(split[3]/ta.region.size[1])};
    //center - left
      *(verts++) = {x1, y1+split[3], {cl[0], cl[1], cl[2], cl[3]}, 0, 1-float(split[3]/ta.region.size[1])};
      *(verts++) = {x1, y2-split[1], {cl[0], cl[1], cl[2], cl[3]}, 0, float(split[1]/ta.region.size[1])};
      *(verts++) = {x1+split[0], y1+split[3], {cl[0], cl[1], cl[2], cl[3]}, float(split[0]/ta.region.size[0]), 1-float(split[3]/ta.region.size[1])};
      *(verts++) = {x1+split[0], y2-split[1], {cl[0], cl[1], cl[2], cl[3]}, float(split[0]/ta.region.size[0]), float(split[1]/ta.region.size[1])};
    //top - left
      *(verts++) = {x1, y2-split[1], {cl[0], cl[1], cl[2], cl[3]}, 0, float(split[1]/ta.region.size[1])};
    }
    *(verts++) = {x1, y2, {cl[0], cl[1], cl[2], cl[3]}, 0, 0};
    if (patched) {
      *(verts++) = {x1+split[0], y2-split[1], {cl[0], cl[1], cl[2], cl[3]}, float(split[0]/ta.region.size[0]), float(split[1]/ta.region.size[1])};
      *(verts++) = {x1+split[0], y2, {cl[0], cl[1], cl[2], cl[3]}, float(split[0]/ta.region.size[0]), 0};
    //bottom - center
      *(verts++) = {x1+split[0], y1, {cl[0], cl[1], cl[2], cl[3]}, float(split[0]/ta.region.size[0]), 1};
      *(verts++) = {x1+split[0], y1+split[3], {cl[0], cl[1], cl[2], cl[3]}, float(split[0]/ta.region.size[0]), 1-float(split[3]/ta.region.size[1])};
      *(verts++) = {x2-split[2], y1, {cl[0], cl[1], cl[2], cl[3]}, 1-float(split[1]/ta.region.size[0]), 1};
      *(verts++) = {x2-split[2], y1+split[3], {cl[0], cl[1], cl[2], cl[3]}, 1-float(split[1]/ta.region.size[0]), 1-float(split[3]/ta.region.size[1])};
    //center
      *(verts++) = {x1+split[0], y1+split[3], {cl[0], cl[1], cl[2], cl[3]}, float(split[0]/ta.region.size[0]), 1-float(split[3]/ta.region.size[1])};
      *(verts++) = {x1+split[0], y2-split[1], {cl[0], cl[1], cl[2], cl[3]}, float(split[0]/ta.region.size[0]), float(split[1]/ta.region.size[1])};
      *(verts++) = {x2-split[2], y1+split[3], {cl[0], cl[1], cl[2], cl[3]}, 1-float(split[2]/ta.region.size[0]), 1-float(split[3]/ta.region.size[1])};
      *(verts++) = {x2-split[2], y2-split[1], {cl[0], cl[1], cl[2], cl[3]}, 1-float(split[2]/ta.region.size[0]), float(split[1]/ta.region.size[1])};
    //top - center
      *(verts++) = {x1+split[0], y2-split[1], {cl[0], cl[1], cl[2], cl[3]}, float(split[0]/ta.region.size[0]), float(split[1]/ta.region.size[1])};
      *(verts++) = {x1+split[0], y2, {cl[0], cl[1], cl[2], cl[3]}, float(split[0]/ta.region.size[0]), 0};
      *(verts++) = {x2-split[2], y2-split[1], {cl[0], cl[1], cl[2], cl[3]}, 1-float(split[2]/ta.region.size[0]), float(split[1]/ta.region.size[1])};
      *(verts++) = {x2-split[2], y2, {cl[0], cl[1], cl[2], cl[3]}, 1-float(split[2]/ta.region.size[0]), 0};
    //bottom - right
      *(verts++) = {x2-split[2], y1, {cl[0], cl[1], cl[2], cl[3]}, 1-float(split[1]/ta.region.size[0]), 1};
      *(verts++) = {x2-split[2], y1+split[3], {cl[0], cl[1], cl[2], cl[3]}, 1-float(split[1]/ta.region.size[0]), 1-float(split[3]/ta.region.size[1])};
    }
    *(verts++) = {x2, y1, {cl[0], cl[1], cl[2], cl[3]}, 1, 1};
    if (patched) {
      *(verts++) = {x2, y1+split[3], {cl[0], cl[1], cl[2], cl[3]}, 1, 1-float(split[3]/ta.region.size[1])};
    //center - right
      *(verts++) = {x2-split[2], y1+split[3], {cl[0], cl[1], cl[2], cl[3]}, 1-float(split[1]/ta.region.size[0]), 1-float(split[3]/ta.region.size[1])};
      *(verts++) = {x2-split[2], y2-split[1], {cl[0], cl[1], cl[2], cl[3]}, 1-float(split[1]/ta.region.size[0]), float(split[1]/ta.region.size[1])};
      *(verts++) = {x2, y1+split[3], {cl[0], cl[1], cl[2], cl[3]}, 1, 1-float(split[3]/ta.region.size[1])};
      *(verts++) = {x2, y2-split[1], {cl[0], cl[1], cl[2], cl[3]}, 1, float(split[1]/ta.region.size[1])};
    //top - right
      *(verts++) = {x2-split[2], y2-split[1], {cl[0], cl[1], cl[2], cl[3]}, 1-float(split[1]/ta.region.size[0]), float(split[1]/ta.region.size[1])};
      *(verts++) = {x2-split[2], y2, {cl[0], cl[1], cl[2], cl[3]}, 1-float(split[1]/ta.region.size[0]), 0};
      *(verts++) = {x2, y2-split[1], {cl[0], cl[1], cl[2], cl[3]}, 1, float(split[1]/ta.region.size[1])};
    }
    *(verts++) = {x2, y2, {cl[0], cl[1], cl[2], cl[3]}, 1, 0};
    
    engine::graph->flat_render(tex,vert,patched?9:1);
  }
}
void uistage::clear() {
  for (auto i = regions.begin(), j = regions.end(); i != j; i++) {
    delete i->second.tex;
  }
  regions.clear();
}
