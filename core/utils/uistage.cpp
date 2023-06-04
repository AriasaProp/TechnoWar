#include "uistage.hpp"
#include "math.hpp"
#include <unordered_map>
#include <unordered_set>

struct textureAtlas {
  engine::texture_core *tex;
  uistage::texture_region region;
};
static std::unordered_map<std::string, textureAtlas> regions;
static std::unordered_set<actor*> actors;

uistage::actor::actor(float p[2],float s[2], unsigned char c[4], std::string tex = ""): pos(p), size(s), color(c), texKey(tex) {
  actors.insert(this);
}
uistage::actor::~actor() {
  auto it = actors.find(this);
  if (it != actors.end())
    actors.erase(it);
}

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
    unsigned int *split = a.patch;
    x1 = a.pos[0];
    y1 = a.pos[1];
    x2 = x1 + a.size[0];
    y2 = y1 + a.size[1];
    
    const textureAtlas &ta = regions[a.texKey];
    const engine::texture_core *tex = ta.tex;
    // left , top, right, bottom
    unsigned int *split = ta.region.patch;
    
    bool patched = (split[0] || split[1] || split[2] || split[3]);
    //bottom - left
    *(verts++) = {x1, y1, {0xff, 0xff, 0xff, 0xff}, 0, 1};
    if (patched) {
      *(verts++) = {x1, y1+split[3], {0xff, 0xff, 0xff, 0xff}, 0, 1-float(split[3]/tex->height())};
      *(verts++) = {x1+split[0], y1, {0xff, 0xff, 0xff, 0xff}, float(split[0]/tex->width()), 1};
      *(verts++) = {x1+split[0], y1+split[3], {0xff, 0xff, 0xff, 0xff}, float(split[0]/tex->width()), 1-float(split[3]/tex->height())};
    //center - left
      *(verts++) = {x1, y1+split[3], {0xff, 0xff, 0xff, 0xff}, 0, 1-float(split[3]/tex->height())};
      *(verts++) = {x1, y2-split[1], {0xff, 0xff, 0xff, 0xff}, 0, float(split[1]/tex->height())};
      *(verts++) = {x1+split[0], y1+split[3], {0xff, 0xff, 0xff, 0xff}, float(split[0]/tex->width()), 1-float(split[3]/tex->height())};
      *(verts++) = {x1+split[0], y2-split[1], {0xff, 0xff, 0xff, 0xff}, float(split[0]/tex->width()), float(split[1]/tex->height())};
    //top - left
      *(verts++) = {x1, y2-split[1], {0xff, 0xff, 0xff, 0xff}, 0, float(split[1]/tex->height())};
    }
    *(verts++) = {x1, y2, {0xff, 0xff, 0xff, 0xff}, 0, 0};
    if (patched) {
      *(verts++) = {x1+split[0], y2-split[1], {0xff, 0xff, 0xff, 0xff}, float(split[0]/tex->width()), float(split[1]/tex->height())};
      *(verts++) = {x1+split[0], y2, {0xff, 0xff, 0xff, 0xff}, float(split[0]/tex->width()), 0};
    //bottom - center
      *(verts++) = {x1+split[0], y1, {0xff, 0xff, 0xff, 0xff}, float(split[0]/tex->width()), 1};
      *(verts++) = {x1+split[0], y1+split[3], {0xff, 0xff, 0xff, 0xff}, float(split[0]/tex->width()), 1-float(split[3]/tex->height())};
      *(verts++) = {x2-split[2], y1, {0xff, 0xff, 0xff, 0xff}, 1-float(split[1]/tex->width()), 1};
      *(verts++) = {x2-split[2], y1+split[3], {0xff, 0xff, 0xff, 0xff}, 1-float(split[1]/tex->width()), 1-float(split[3]/tex->height())};
    //center
      *(verts++) = {x1+split[0], y1+split[3], {0xff, 0xff, 0xff, 0xff}, float(split[0]/tex->width()), 1-float(split[3]/tex->height())};
      *(verts++) = {x1+split[0], y2-split[1], {0xff, 0xff, 0xff, 0xff}, float(split[0]/tex->width()), float(split[1]/tex->height())};
      *(verts++) = {x2-split[2], y1+split[3], {0xff, 0xff, 0xff, 0xff}, 1-float(split[2]/tex->width()), 1-float(split[3]/tex->height())};
      *(verts++) = {x2-split[2], y2-split[1], {0xff, 0xff, 0xff, 0xff}, 1-float(split[2]/tex->width()), float(split[1]/tex->height())};
    //top - center
      *(verts++) = {x1+split[0], y2-split[1], {0xff, 0xff, 0xff, 0xff}, float(split[0]/tex->width()), float(split[1]/tex->height())};
      *(verts++) = {x1+split[0], y2, {0xff, 0xff, 0xff, 0xff}, float(split[0]/tex->width()), 0};
      *(verts++) = {x2-split[2], y2-split[1], {0xff, 0xff, 0xff, 0xff}, 1-float(split[2]/tex->width()), float(split[1]/tex->height())};
      *(verts++) = {x2-split[2], y2, {0xff, 0xff, 0xff, 0xff}, 1-float(split[2]/tex->width()), 0};
    //bottom - right
      *(verts++) = {x2-split[2], y1, {0xff, 0xff, 0xff, 0xff}, 1-float(split[1]/tex->width()), 1};
      *(verts++) = {x2-split[2], y1+split[3], {0xff, 0xff, 0xff, 0xff}, 1-float(split[1]/tex->width()), 1-float(split[3]/tex->height())};
    }
    *(verts++) = {x2, y1, {0xff, 0xff, 0xff, 0xff}, 1, 1};
    if (patched) {
      *(verts++) = {x2, y1+split[3], {0xff, 0xff, 0xff, 0xff}, 1, 1-float(split[3]/tex->height())};
    //center - right
      *(verts++) = {x2-split[2], y1+split[3], {0xff, 0xff, 0xff, 0xff}, 1-float(split[1]/tex->width()), 1-float(split[3]/tex->height())};
      *(verts++) = {x2-split[2], y2-split[1], {0xff, 0xff, 0xff, 0xff}, 1-float(split[1]/tex->width()), float(split[1]/tex->height())};
      *(verts++) = {x2, y1+split[3], {0xff, 0xff, 0xff, 0xff}, 1, 1-float(split[3]/tex->height())};
      *(verts++) = {x2, y2-split[1], {0xff, 0xff, 0xff, 0xff}, 1, float(split[1]/tex->height())};
    //top - right
      *(verts++) = {x2-split[2], y2-split[1], {0xff, 0xff, 0xff, 0xff}, 1-float(split[1]/tex->width()), float(split[1]/tex->height())};
      *(verts++) = {x2-split[2], y2, {0xff, 0xff, 0xff, 0xff}, 1-float(split[1]/tex->width()), 0};
      *(verts++) = {x2, y2-split[1], {0xff, 0xff, 0xff, 0xff}, 1, float(split[1]/tex->height())};
    }
    *(verts++) = {x2, y2, {0xff, 0xff, 0xff, 0xff}, 1, 0};
    
    engine::graph->flat_render(tex,verts,patched?9:1)
  }
}
void uistage::clear() {
  for (auto i = regions.begin(), j = regions.end(); i != j; i++) {
    delete i->seconds.tex;
  }
  regions.clear();
}
