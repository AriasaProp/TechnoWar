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
  float xmin, ymin, xmax, ymax;
  float umin, vmin, umax, vmax;
  engine::flat_vertex v_;
  for (auto i = actors.begin(), j = actors.end(); i != j; i++) {
    engine::flat_vertex *verts = vert;
    actor &a = **i;
    textureAtlas &ta = regions[a.texKey];
    engine::texture_core *tex = ta.tex;
    // left, top, right, bottom
    const unsigned int *split = ta.region.patch;
    
    memcpy(&v_.color, a.color, sizeof(v_.color));
    ymin = a.pos[1], ymax = ymin+a.size[1];
    xmax = xmin+a.size[0];
    umax = float(ta.region.pos[0]+ta.region.size[0])/float(tex->width());
    vmin = float(ta.region.pos[1]+ta.region.size[1])/float(tex->height());
    vmax = float(ta.region.pos[1])/float(tex->height());
    
    float columnList[3]{ymin+split[3], ymax-split[1], ymax};
    float vList[3]{vmin-float(split[3])/float(tex->height()), vmax+float(split[1])/float(tex->height()), vmax};
    float rowList[3]{xmin+split[0], xmax-split[2], xmax};
    float uList[3]{umin+float(split[0])/float(tex->width()), umax-float(split[2])/float(tex->width()), umax};
    size_t quadCount = 0;
    for (size_t i = 0; i < 3; i++) { //vertical list
      if (columnList[i] == ymin) continue;
      ymax = columnList[i];
      vmax = vList[i];
      xmin = a.pos[0], umin = float(ta.region.pos[0])/float(tex->width());
      for (size_t j = 0; j < 3; j++) { //horizontal list
        if (rowList[j] == xmin) continue;
        xmax = rowList[j];
        umax = uList[j];
        v_.x = xmin, v_.u = umin;
        v_.y = ymin, v_.v = vmin;
        memcpy(verts++, &v_, sizeof(v_));
        v_.y = ymax, v_.v = vmax;
        memcpy(verts++, &v_, sizeof(v_));
        v_.x = xmax, v_.u = umax;
        v_.y = ymin, v_.v = vmin;
        memcpy(verts++, &v_, sizeof(v_));
        v_.y = ymax, v_.v = vmax;
        memcpy(verts++, &v_, sizeof(v_));
        xmin = xmax, umin = umax;
        quadCount++;
      }
      ymin = ymax, vmin = vmax;
    }
    engine::graph->flat_render(tex,vert,quadCount);
  }
}
void uistage::clear() {
  for (auto i = regions.begin(), j = regions.end(); i != j; i++) {
    delete i->second.tex;
  }
  regions.clear();
}
