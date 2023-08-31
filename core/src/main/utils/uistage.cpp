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

static engine::flat_vertex vert[1024*1024]; //1024 MB
static float cList[4];
static float vList[4];
static float rList[4];
static float uList[4];
void uistage::draw (float delta) {
  (void)delta;
  //hit by touches / click
  //draw
  for (actor *act : actors) {
    std::string texKey = act->texKey();
    if (engine::inpt->onTouched() && (act->getType()==Actor_Type::Button)) {
      texKey = ((button_actor *)act)->keys[1];
    }
    textureAtlas &ta = regions[texKey];
    engine::texture_core *tex = ta.tex;
    // left, top, right, bottom
    const unsigned int *split = ta.region.patch;
    Rect rectangle = act->getRect();
    cList[0] = rectangle.ymin;
    cList[3] = rectangle.ymax;
    cList[1] = cList[0] + split[3];
    cList[2] = cList[3] - split[1];
    
    vList[0] = float(ta.region.pos[1]+ta.region.size[1])/float(tex->height());
    vList[3] = float(ta.region.pos[1])/float(tex->height());
    vList[1] = float(ta.region.pos[1]+ta.region.size[1]-split[3])/float(tex->height());
    vList[2] = float(ta.region.pos[1]+split[1])/float(tex->height());
    
    rList[0] = rectangle.xmin;
    rList[3] = rectangle.xmax;
    rList[1] = rList[0] + split[0];
    rList[2] = rList[3] - split[2];
    
    uList[0] = float(ta.region.pos[0])/float(tex->width());
    uList[3] = float(ta.region.pos[0]+ta.region.size[0])/float(tex->width());
    uList[1] = float(ta.region.pos[0]+split[0])/float(tex->width());
    uList[2] = float(ta.region.pos[0]+ta.region.size[0]-split[2])/float(tex->width());
    
    size_t quadCount = 0;
    engine::flat_vertex *verts = vert;
    for (size_t p = 0; p < 3; p++) { //vertical list
      float &ymin = cList[ p ];
      float &ymax = cList[p+1];
      if (ymax == ymin) continue;
      for (size_t q = 0; q < 3; q++) { //horizontal list
        float &xmin = rList[ q ];
        float &xmax = rList[q+1];
        if (xmax == xmin) continue;
        *(verts++) = {xmin, ymin, ta.clr, uList[ q ], vList[ p ]};
        *(verts++) = {xmin, ymax, ta.clr, uList[ q ], vList[p+1]};
        *(verts++) = {xmax, ymin, ta.clr, uList[q+1], vList[ p ]};
        *(verts++) = {xmax, ymax, ta.clr, uList[q+1], vList[p+1]};
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
