#include "uistage.hpp"
#include <unordered_map>
#include <unordered_set>

struct textureAtlas {
  engine::texture_core *tex;
  uistage::texture_region region;
};
static std::unordered_map<std::string, textureAtlas> regions;
static std::unordered_set<uistage::actor*> actors;

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
static engine::flat_vertex v_;
static float cList[4];
static float vList[4];
static float rList[4];
static float uList[4];
void uistage::draw () {
  for (actor *act : actors) {
    engine::flat_vertex *verts = vert;
    textureAtlas &ta = regions[act->texKey];
    engine::texture_core *tex = ta.tex;
    // left, top, right, bottom
    const unsigned int *split = ta.region.patch;
    memcpy(&v_.color, act->color, sizeof(v_.color));
    cList[0] = act->rectangle.ymin;
    cList[3] = act->rectangle.ymax;
    cList[1] = cList[0] + split[3];
    cList[2] = cList[3] - split[1];
    
    vList[0] = float(ta.region.pos[1]+ta.region.size[1])/float(tex->height());
    vList[3] = float(ta.region.pos[1])/float(tex->height());
    vList[1] = float(ta.region.pos[1]+ta.region.size[1]-split[3])/float(tex->height());
    vList[2] = float(ta.region.pos[1]+split[1])/float(tex->height());
    
    rList[0] = act->rectangle.xmin;
    rList[3] = act->rectangle.xmax;
    rList[1] = rList[0] + split[0];
    rList[2] = rList[3] - split[2];
    
    uList[0] = float(ta.region.pos[0])/float(tex->width());
    uList[3] = float(ta.region.pos[0]+ta.region.size[0])/float(tex->width());
    uList[1] = float(ta.region.pos[0]+split[0])/float(tex->width());
    uList[2] = float(ta.region.pos[0]+ta.region.size[0]-split[2])/float(tex->width());
    
    size_t quadCount = 0;
    for (size_t p = 0; p < 3; p++) { //vertical list
      float &ymin  = cList[p];
      float &ymax = cList[p+1];
      if (ymax == ymin) continue;
      float &vmin = vList[p];
      float &vmax = vList[p+1];
      for (size_t q = 0; q < 3; q++) { //horizontal list
        float &xmin = rList[q];
        float &xmax = rList[q+1];
        if (xmax == xmin) continue;
        float &umin = uList[q];
        float &umax = uList[q+1];
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
        quadCount++;
      }
    }
    engine::graph->flat_render(tex,vert,quadCount);
  }
}
void uistage::clear() {
  for (auto i = regions.begin(), j = regions.end(); i != j; i++) {
    delete i->second.tex;
  }
  regions.clear();
  for (uistage::actor *a : actors)
    delete a;
  actors.clear();
}

uistage::actor makeImage(std::string texKey, const Rect &r) {
  actors.insert(new actor{
    .rectangle = r,
    .texKey = texKey
  });
}
