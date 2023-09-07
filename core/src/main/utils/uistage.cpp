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
  size_t mstate = 0;
  Rect mRect;
  
  button_actor(std::string *k, Rect r): keys(k), mRect(r) {}
  
  Rect &getRect() override {
    return mRect;
  }
  void setState(size_t state) {
    mstate = state;
  }
  std::string texKey() override {
    return keys[mstate];
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
static float yList[2], vList[2], xList[2], uList[2];
void uistage::draw (float delta) {
  (void)delta;
  //hit by touches / click
  //draw
  for (actor *act : actors) {
    textureAtlas &ta = regions[act->texKey()];
    engine::texture_core *tex = ta.tex;
    // left, top, right, bottom
    const unsigned int *split = ta.region.patch;
    Rect rectangle = act->getRect();
    size_t quadCount = 0;
    engine::flat_vertex *verts = vert;
    //vertically 1
    if (split[3]) { //horizontally
      yList[0] = rectangle.ymin;
      yList[1] = rectangle.ymin + split[3];
      vList[0] = float(ta.region.pos[1]+ta.region.size[1])/float(tex->height());
      vList[1] = float(ta.region.pos[1]+ta.region.size[1]-split[3])/float(tex->height());
      if (split[0]) {
        xList[0] = rectangle.xmin;
        xList[1] = rectangle.xmin + split[0];
        uList[0] = float(ta.region.pos[0])/float(tex->width());
        uList[1] = float(ta.region.pos[0]+split[0])/float(tex->width());
        *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
        *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
        *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
        *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
        quadCount++;
      }
      xList[0] = rectangle.xmin + split[0];
      xList[1] = rectangle.xmax - split[2];
      if (xList[1] > xList[0]) {
        uList[0] = float(ta.region.pos[0]+split[0])/float(tex->width());
        uList[1] = float(ta.region.pos[0]+ta.region.size[0]-split[2])/float(tex->width());
        *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
        *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
        *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
        *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
        quadCount++;
      }
      if (split[2]) {
        xList[0] = rectangle.xmax - split[2];
        xList[1] = rectangle.xmax;
        uList[0] = float(ta.region.pos[0]+ta.region.size[0]-split[2])/float(tex->width());
        uList[1] = float(ta.region.pos[0]+ta.region.size[0])/float(tex->width());
        *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
        *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
        *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
        *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
        quadCount++;
      }
    }
    //vertically 2
    yList[0] = rectangle.ymin + split[3];
    yList[1] = rectangle.ymax - split[1];
    if (yList[1] > yList[0]) { //horizontally
      vList[0] = float(ta.region.pos[1]+ta.region.size[1]-split[3])/float(tex->height());
      vList[1] = float(ta.region.pos[1]+split[1])/float(tex->height());
      if (split[0]) {
        xList[0] = rectangle.xmin;
        xList[1] = rectangle.xmin + split[0];
        uList[0] = float(ta.region.pos[0])/float(tex->width());
        uList[1] = float(ta.region.pos[0]+split[0])/float(tex->width());
        *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
        *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
        *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
        *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
        quadCount++;
      }
      xList[0] = rectangle.xmin + split[0];
      xList[1] = rectangle.xmax - split[2];
      if (xList[1] > xList[0]) {
        uList[0] = float(ta.region.pos[0]+split[0])/float(tex->width());
        uList[1] = float(ta.region.pos[0]+ta.region.size[0]-split[2])/float(tex->width());
        *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
        *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
        *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
        *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
        quadCount++;
      }
      if (split[2]) {
        xList[0] = rectangle.xmax - split[2];
        xList[1] = rectangle.xmax;
        uList[0] = float(ta.region.pos[0]+ta.region.size[0]-split[2])/float(tex->width());
        uList[1] = float(ta.region.pos[0]+ta.region.size[0])/float(tex->width());
        *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
        *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
        *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
        *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
        quadCount++;
      }
    }
    //vertically 3
    if (split[1]) { //horizontally
      yList[0] = rectangle.ymax - split[1];
      yList[1] = rectangle.ymax;
      vList[0] = float(ta.region.pos[1]+split[1])/float(tex->height());
      vList[1] = float(ta.region.pos[1])/float(tex->height());
      if (split[0]) {
        xList[0] = rectangle.xmin;
        xList[1] = rectangle.xmin + split[0];
        uList[0] = float(ta.region.pos[0])/float(tex->width());
        uList[1] = float(ta.region.pos[0]+split[0])/float(tex->width());
        *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
        *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
        *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
        *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
        quadCount++;
      }
      xList[0] = rectangle.xmin + split[0];
      xList[1] = rectangle.xmax - split[2];
      if (xList[1] > xList[0]) {
        uList[0] = float(ta.region.pos[0]+split[0])/float(tex->width());
        uList[1] = float(ta.region.pos[0]+ta.region.size[0]-split[2])/float(tex->width());
        *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
        *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
        *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
        *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
        quadCount++;
      }
      if (split[2]) {
        xList[0] = rectangle.xmax - split[2];
        xList[1] = rectangle.xmax;
        uList[0] = float(ta.region.pos[0]+ta.region.size[0]-split[2])/float(tex->width());
        uList[1] = float(ta.region.pos[0]+ta.region.size[0])/float(tex->width());
        *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
        *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
        *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
        *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
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

uistage::actor *focused_actor[100]{};
void uistage::touchDown(float x, float y, int pointer, int button) {
  for (actor *act : actors) {
    if ((act->getType()==Actor_Type::Button) && (act->getRect().insetOf(x, y))) {
      ((button_actor *)act)->setState(1);
      focused_actor[pointer] = act;
      return;
    }
  }
  (void)button;
}
void uistage::touchMove(float x, float y, float xs, float ys, int pointer, int button) {
  
  (void)x;
  (void)y;
  (void)xs;
  (void)ys;
  (void)pointer;
  (void)button;
}
void uistage::touchUp(float x, float y, int pointer, int button) {
  if (focused_actor[pointer]) {
    actor *act = focused_actor[pointer];
    if (act->getType()==Actor_Type::Button) {
      ((button_actor *)act)->setState(0);
    }
    focused_actor[pointer] = nullptr;
  }
  (void)x;
  (void)y;
  (void)button;
}
void uistage::touchCanceled(float x, float y, int pointer, int button) {
  (void)x;
  (void)y;
  (void)pointer;
  (void)button;
}
