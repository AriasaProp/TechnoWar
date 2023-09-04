#include "uistage.hpp"
#include <unordered_map>
#include <unordered_set>

enum Actor_Type: size_t{
  None = 0,
  Image,
  Button
};

struct image_actor: public uistage::actor {
  std::string key;
  Rect mRect;
  
  image_actor(std::string k, Rect r): key(k), mRect(r) {}
  
  Rect &getRect() override {
    return mRect;
  }
  std::string texKey() override {
    return key;
  }
  size_t getType() const override { return Actor_Type::Image; }
  ~image_actor() override {}
};
struct button_actor: public uistage::actor {
  std::string *keys;
  Rect mRect;
  
  button_actor(std::string *k, Rect r): keys(k), mRect(r) {}
  
  Rect &getRect() override {
    return mRect;
  }
  std::string texKey() override {
    return keys[0];
  }
  size_t getType() const override { return Actor_Type::Button; }
  ~button_actor() override {delete[] keys;}
};

struct textureAtlas {
  engine::texture_core *tex;
  uistage::texture_region region;
  uint32_t clr;
};
static std::unordered_map<std::string, textureAtlas> regions;
static std::unordered_set<uistage::actor*> actors;

void uistage::addTextureRegion(std::string key, engine::texture_core *tex, const uistage::texture_region &reg) {
  regions[key] = textureAtlas{tex, reg, 0xffffffff};
}
void uistage::addTextureRegion(std::string key, engine::texture_core *tex, const uistage::texture_region &reg, const uint32_t clr) {
  regions[key] = textureAtlas{tex, reg, clr};
}

static engine::flat_vertex vert[1024]; //= 20 KB, approximate 1024 actors can be drawn at once
static float cList[4], vList[4], xList[2], uList[2];
static float touch_pos[2];
void uistage::draw (float delta) {
  (void)delta;
  //hit by touches / click
  //draw
  for (actor *act : actors) {
    std::string texKey = act->texKey();
    if (engine::inpt->isTouched(0) && (act->getType()==Actor_Type::Button)) {
      engine::inpt->getPointerPos(touch_pos, 0);
      if (act->getRect().insetOf(touch_pos[0], touch_pos[1])) {
        texKey = ((button_actor *)act)->keys[1];
      }
    }
    textureAtlas &ta = regions[texKey];
    engine::texture_core *tex = ta.tex;
    // left, top, right, bottom
    const unsigned int *split = ta.region.patch;
    Rect rectangle = act->getRect();
    cList[0] = rectangle.ymin;
    cList[1] = rectangle.ymin + split[3];
    cList[2] = rectangle.ymax - split[1];
    cList[3] = rectangle.ymax;
    
    vList[0] = float(ta.region.pos[1]+ta.region.size[1])/float(tex->height());
    vList[3] = float(ta.region.pos[1])/float(tex->height());
    vList[1] = float(ta.region.pos[1]+ta.region.size[1]-split[3])/float(tex->height());
    vList[2] = float(ta.region.pos[1]+split[1])/float(tex->height());
    
    size_t quadCount = 0;
    engine::flat_vertex *verts = vert;
    for (size_t p = 0; p < 3; p++) { //vertical list
      float &ymin = cList[ p ];
      float &ymax = cList[p+1];
      if (ymax > ymin) { //horizontally
        float &vmin = vList[ p ];
        float &vmax = vList[p+1];
        if (split[0]) {
          xList[0] = rectangle.xmin;
          xList[1] = rectangle.xmin + split[0];
          uList[0] = float(ta.region.pos[0])/float(tex->width());
          uList[1] = float(ta.region.pos[0]+split[0])/float(tex->width());
          *(verts++) = {xList[0], ymin, ta.clr, uList[0], vmin};
          *(verts++) = {xList[0], ymax, ta.clr, uList[0], vmax};
          *(verts++) = {xList[1], ymin, ta.clr, uList[1], vmin};
          *(verts++) = {xList[1], ymax, ta.clr, uList[1], vmax};
          quadCount++;
        }
        xList[0] = rectangle.xmin + split[0];
        xList[1] = rectangle.xmax - split[2];
        if (xList[1] > xList[0]) {
          uList[0] = float(ta.region.pos[0]+split[0])/float(tex->width());
          uList[1] = float(ta.region.pos[0]+ta.region.size[0]-split[2])/float(tex->width());
          *(verts++) = {xList[0], ymin, ta.clr, uList[0], vmin};
          *(verts++) = {xList[0], ymax, ta.clr, uList[0], vmax};
          *(verts++) = {xList[1], ymin, ta.clr, uList[1], vmin};
          *(verts++) = {xList[1], ymax, ta.clr, uList[1], vmax};
          quadCount++;
        }
        if (split[2]) {
          xList[0] = rectangle.xmax - split[2];
          xList[1] = rectangle.xmax;
          uList[0] = float(ta.region.pos[0]+ta.region.size[0]-split[2])/float(tex->width());
          uList[1] = float(ta.region.pos[0]+ta.region.size[0])/float(tex->width());
          *(verts++) = {xList[0], ymin, ta.clr, uList[0], vmin};
          *(verts++) = {xList[0], ymax, ta.clr, uList[0], vmax};
          *(verts++) = {xList[1], ymin, ta.clr, uList[1], vmin};
          *(verts++) = {xList[1], ymax, ta.clr, uList[1], vmax};
          quadCount++;
        }
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

uistage::actor *uistage::makeImage(std::string k, Rect r) {
  uistage::actor *ua = new image_actor(k, r);
  actors.insert(ua);
  return ua;
}

uistage::actor *uistage::makeButton(std::initializer_list<std::string> k, Rect r) {
  if (!k.size()) throw("button must have a key texture");
  std::string *K = new std::string[k.size()];
  size_t i = 0;
  const std::string *it = k.begin(), *end = k.end();
  while (it != end) {
    K[i] = *it;
    ++it;
    ++i;
  }
  uistage::actor *ua = new button_actor(K,r);
  actors.insert(ua);
  return ua;
}
