#include "uistage.hpp"
#include "../engine.hpp"
#include "math.hpp"
#include "../assets/stb_image.hpp"

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

struct CharDescriptor;

struct bmfont {
  short LineHeight;
  short Base;
  short Width;
  short Height;
  short Outline;
  std::unordered_map<int, CharDescriptor> Chars;
  std::unordered_map<uint32_t, float> Kearn;
  uint32_t fcolor;
  engine::texture_core *ftexid;
  float fontSizeBase, fontSizeUsed;
  float fscale ();
  bool ParseFont (const char *);
  void SetColor (unsigned char, unsigned char, unsigned char, unsigned char);
  void setFontSize (float);//px
  float GetHeight ();
  //void draw_text (float, float, Align, const char *);
  //void draw_text (float, float, Align, const char *, ...);
  bmfont (const char *);
  ~bmfont ();

} *font = nullptr;
//BMFONT STATE end
struct textureAtlas {
  engine::texture_core *tex;
  uistage::texture_region region;
  uint32_t clr;
};
static std::unordered_map<std::string, textureAtlas> regions;
//static engine::texture_core *binded = nullptr;
static std::unordered_set<uistage::actor*> actors;
static engine::flat_vertex vert[1024]; //= 20 KB, approximate 1024 actors can be drawn at once
static float yList[2], vList[2], xList[2], uList[2];
enum Actor_Type: size_t{
  None = 0,
  Static,
  Button
};

struct text_actor: public uistage::actor {
  std::string stext;
  float xPos, yPos;
  Align align;
  Rect mRect;
  
  text_actor(float x, float y, Align a, std::string t): stext(t), xPos(x), yPos(y), align(a) {}
  
  Rect &getRect() override {
    return mRect;
  }
  void draw(float delta, engine::flat_vertex *vert) override {
    if (!font) return;
    const char *text = stext.c_str();
    unsigned char xpivot = align & 3;
    float x = xPos;
    float y = yPos;
    float F = font->fscale();
    auto &Chars = font->Chars;
    switch (xpivot) {
    default: // left
      break;
    case 1: { // center
      float total = 0;
      for (const char *t = text; *t; t++) {
        if (Chars.find (*t) == Chars.end ()) continue;
        total += Chars[*t].XAdvance;
      }
      x -= total * 0.5f * F;
      break;
    }
    case 2: { // right
      float total = 0;
      for (const char *t = text; *t; t++) {
        if (Chars.find (*t) == Chars.end ()) continue;
        total += Chars[*t].XAdvance;
      }
      x -= total * F;
      break;
    }
    }
    float &LineHeight = font->LineHeight;
    unsigned char ypivot = (align >> 2);
    switch (ypivot) {
    default: // top
      break;
    case 1: // center
      y += LineHeight * F * 0.5f;
      break;
    case 2: // bottom
      y += LineHeight * F;
      break;
    }
    engine::flat_vertex *cur_tex = vert;
    auto &Kearn = font->Kearn;
    for (const char *t = text; *t; t++) {
      auto itf= Chars.find (*t);
      if (itf == Chars.end ()) continue;
      CharDescriptor &f = itf->second;
      xList[0] = x + (f.XOffset * F); // minx
      yList[0] = y - (f.YOffset * F); // maxy
      xList[1] = xList[0] + (f.Width * F);  // maxx
      yList[1] = yList[0] - (f.Height * F); // miny
      
      uList[0] = f.x / (float)Width;
      vList[0] = f.y / (float)Height;
      uList[1] = (f.x + f.Width) / (float)Width;
      vList[1] = (f.y + f.Height) / (float)Height;
    
      *(cur_tex++) = {xList[0],yList[1],fcolor,uList[0],vList[1]};
      *(cur_tex++) = {xList[0],yList[0],fcolor,uList[0],vList[0]};
      *(cur_tex++) = {xList[1],yList[1],fcolor,uList[1],vList[1]};
      *(cur_tex++) = {xList[1],yList[0],fcolor,uList[1],vList[0]};
      
      if (*(t + 1)) {
        float nX = f.XAdvance;
        uint16_t key[2] = {*t, *(t + 1)};
        auto it = Kearn.find (*(uint32_t *)key));
        if (it != Kearn.end ())
          nX += it->second;
        x += nX * F;
      }
  }
    engine::graph->flat_render (font->ftexid, vert, strlen(text));
  }
  size_t getType() const override { return Actor_Type::Static; }
  ~image_actor() override {}
};
struct image_actor: public uistage::actor {
  std::string key;
  Rect rectangle;
  
  image_actor(std::string k, Rect r): key(k), mRect(r) {}
  
  Rect &getRect() override {
    return rectangle;
  }
  size_t getType() const override { return Actor_Type::Static; }
  void draw(float delta, engine::flat_vertex *vert) override {
    (void)delta;
    textureAtlas &ta = regions[key];
    engine::texture_core *tex = ta.tex;
    // left, top, right, bottom
    const unsigned int *split = ta.region.patch;
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
  ~image_actor() override {}
};
struct button_actor: public uistage::actor {
  std::string *keys;
  size_t mstate = 0;
  void (*onClick)();
  Rect rectangle;
  
  button_actor(std::string *k, Rect r): keys(k), mRect(r) {}
  
  Rect &getRect() override {
    return rectangle;
  }
  void setState(size_t state) {
    mstate = state;
  }
  size_t getType() const override { return Actor_Type::Button; }
  
  void draw(float delta, engine::flat_vertex *vert) override {
    (void)delta;
    textureAtlas &ta = regions[keys[mstate]];
    engine::texture_core *tex = ta.tex;
    // left, top, right, bottom
    const unsigned int *split = ta.region.patch;
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
  
  ~button_actor() override {delete[] keys;}
};

void uistage::loadBMFont(const char *fontFile) {
  font = new bmfont(fontFile);
  font->setFontSize(40.f);
}
void uistage::addTextureRegion(std::string key, engine::texture_core *tex, const uistage::texture_region &reg) {
  regions[key] = textureAtlas{tex, reg, 0xffffffff};
}
void uistage::addTextureRegion(std::string key, engine::texture_core *tex, const uistage::texture_region &reg, const uint32_t clr) {
  regions[key] = textureAtlas{tex, reg, clr};
}
void uistage::draw (float delta) {
  //hit by touches / click
  //draw
  for (actor *act : actors) {
    act->draw(delta, vert);
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
  delete font;
}

uistage::actor *uistage::makeImage(std::string k, Rect r) {
  uistage::actor *ua = new image_actor(k, r);
  actors.insert(ua);
  return ua;
}

uistage::actor *uistage::makeButton(std::initializer_list<std::string> k, Rect r, void(*onclick)()) {
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
uistage::actor *uistage::makeText(float x, float y, Align a, std::string k) {
  
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
      button_actor *bact = (button_actor *)act;
      bact->setState(0);
      if (bact->onClick!=NULL)
        bact->onClick();
    }
    focused_actor[pointer] = nullptr;
  }
  (void)x;
  (void)y;
  (void)button;
}
void uistage::touchCanceled(float x, float y, int pointer, int button) {
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


//define bmfont source

struct CharDescriptor {
  short x, y;
  short Width, Height;
  short XOffset, YOffset;
  short XAdvance;
};

// Todo: Add buffer overflow checking.
//#define MAX_BUFFER 256

bool bmfont::ParseFont (const char *fontfile) {
  unsigned int asl;
  const char *as = (const char *)engine::asset->asset_buffer (fontfile, &asl);
  std::string buffer (as, asl);
  std::stringstream buffer_stream (buffer);
  std::string Line, Read, Key, Value;
  unsigned int i;

  CharDescriptor C;

  while (!buffer_stream.eof ()) {
    std::stringstream LineStream;
    std::getline (buffer_stream, Line);
    LineStream << Line;

    // read the line's type
    LineStream >> Read;
    if (Read == "info") {
      // this holds info data
      while (!LineStream.eof ()) {
        std::stringstream Converter;
        LineStream >> Read;
        i = Read.find ('=');
        Key = Read.substr (0, i);
        Value = Read.substr (i + 1);

        // assign the correct value
        Converter << Value;
        if (Key == "size") {
          Converter >> fontSizeBase;
          fontSizeUsed = fontSizeBase;
        }
        
      }
    } else if (Read == "common") {
      // this holds common data
      while (!LineStream.eof ()) {
        std::stringstream Converter;
        LineStream >> Read;
        i = Read.find ('=');
        Key = Read.substr (0, i);
        Value = Read.substr (i + 1);

        // assign the correct value
        Converter << Value;
        if (Key == "lineHeight") {
          Converter >> LineHeight;
        } else if (Key == "base") {
          Converter >> Base;
        } else if (Key == "scaleW") {
          Converter >> Width;
        } else if (Key == "scaleH") {
          Converter >> Height;
        } else if (Key == "outline") {
          Converter >> Outline;
        }
      }
    } else if (Read == "chars") { // only 1 value
      std::stringstream Converter;
      LineStream >> Read;
      i = Read.find ('=');
      Key = Read.substr (0, i);
      Value = Read.substr (i + 1);
      Converter << Value;
      if (Key == "count") {
        int CharCount;
        Converter >> CharCount;
        Chars.reserve (CharCount);
      }
    } else if (Read == "char") {
      int CharID = 0;
      while (!LineStream.eof ()) {
        std::stringstream Converter;
        LineStream >> Read;
        i = Read.find ('=');
        Key = Read.substr (0, i);
        Value = Read.substr (i + 1);
        // Assign the correct value
        Converter << Value;
        if (Key == "id")
          Converter >> CharID;
        else if (Key == "x")
          Converter >> C.x;
        else if (Key == "y")
          Converter >> C.y;
        else if (Key == "width")
          Converter >> C.Width;
        else if (Key == "height")
          Converter >> C.Height;
        else if (Key == "xoffset")
          Converter >> C.XOffset;
        else if (Key == "yoffset")
          Converter >> C.YOffset;
        else if (Key == "xadvance")
          Converter >> C.XAdvance;
      }
      Chars[CharID] = C;
    } else if (Read == "kernings") { // only 1 value
      std::stringstream Converter;
      LineStream >> Read;
      i = Read.find ('=');
      Key = Read.substr (0, i);
      Value = Read.substr (i + 1);
      Converter << Value;
      if (Key == "count") {
        int KernCount;
        Converter >> KernCount;
        Kearn.reserve (KernCount);
      }
    } else if (Read == "kerning") {
      uint16_t id[2];
      float amount;
      while (!LineStream.eof ()) {
        std::stringstream Converter;
        LineStream >> Read;
        i = Read.find ('=');
        Key = Read.substr (0, i);
        Value = Read.substr (i + 1);
        Converter << Value;
        if (Key == "first")
          Converter >> id[0];
        else if (Key == "second")
          Converter >> id[1];
        else if (Key == "amount")
          Converter >> amount;
      }
      Kearn[*((uint32_t*)id)] = amount;
    }
  }
  return true;
}
/*
void bmfont::draw_text (float x, float y, Align align, const char *fmt, ...) {
  if (fmt == NULL)          // If There's No Text
    return;                 // Do Nothing
  va_list ap;               // Pointer To List Of Arguments
  va_start (ap, fmt);       // Parses The String For Variables
  vsprintf (text, fmt, ap); // And Converts Symbols To Actual Numbers
  va_end (ap);
  unsigned char xpivot = align & 3;
  float F = fscale();
  switch (xpivot) {
  default: // left
    break;
  case 1: { // center
    float total = 0;
    for (const char *t = text; *t; t++) {
      if (Chars.find (*t) == Chars.end ()) continue;
      total += Chars[*t].XAdvance;
    }
    x -= total * 0.5f * F;
    break;
  }
  case 2: { // right
    float total = 0;
    for (const char *t = text; *t; t++) {
      if (Chars.find (*t) == Chars.end ()) continue;
      total += Chars[*t].XAdvance;
    }
    x -= total * F;
    break;
  }
  }
  unsigned char ypivot = (align >> 2);
  switch (ypivot) {
  default: // top
    break;
  case 1: // center
    y += LineHeight * F * 0.5f;
    break;
  case 2: // bottom
    y += LineHeight * F;
    break;
  }
  float x1, y1, x2, y2, u1, v1, u2, v2;
  engine::flat_vertex *cur_tex = temp_vertex;
  for (const char *t = text; *t; t++) {
    auto itf= Chars.find (*t);
    if (itf == Chars.end ()) continue;
    CharDescriptor &f = itf->second;
    x1 = x + (f.XOffset * F); // minx
    y1 = y - (f.YOffset * F); // maxy
    x2 = x1 + (f.Width * F);  // maxx
    y2 = y1 - (f.Height * F); // miny
    
    u1 = f.x / (float)Width;
    v1 = f.y / (float)Height;
    u2 = (f.x + f.Width) / (float)Width;
    v2 = (f.y + f.Height) / (float)Height;

    // 0,1 Texture Coord, minxy
    *(cur_tex++) = {x1,y2,fcolor,u1,v2};
    // 0,0 Texture Coord, minx maxy
    *(cur_tex++) = {x1,y1,fcolor,u1,v1};
    // 1,1 Texture Coord, maxx miny
    *(cur_tex++) = {x2,y2,fcolor,u2,v2};
    // 1,0 Texture Coord, maxxy
    *(cur_tex++) = {x2,y1,fcolor,u2,v1};
    
    if (*(t + 1)) {
      float nX = f.XAdvance;
      short key[2] = {*t, *(t + 1)};
      auto it = Kearn.find (*((unsigned int *)key));
      if (it != Kearn.end ())
        nX += it->second;
      x += nX * F;
    }
  }
  engine::graph->flat_render (ftexid, temp_vertex, strlen(text));
}
*/
float bmfont::fscale() { return fontSizeUsed/fontSizeBase;}
void bmfont::SetColor (unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
  memcpy (&fcolor, (unsigned char[]){r, g, b, a}, sizeof (unsigned char) * 4);
}
void bmfont::setFontSize (float size) { //px
  fontSizeUsed = size;
}
float bmfont::GetHeight () {
  return (float)LineHeight * fscale();
}
bmfont::bmfont (const char *fontfile) : fcolor (0xffffffff), ftexid (nullptr) {
  int x, y;
  unsigned int datRI;
  ParseFont (fontfile);
  char *texfile = new char[strlen (fontfile)];
  memcpy (texfile, fontfile, strlen (fontfile));
  memcpy (strstr (texfile, ".fnt"), ".png", 4);
  void *datR = engine::asset->asset_buffer (texfile, &datRI);
  delete[] texfile;
  unsigned char *tD = stbi_load_from_memory ((unsigned char const *)datR, (int)datRI, &x, &y, nullptr, STBI_rgb_alpha);
  free (datR);
  ftexid = engine::graph->gen_texture (x, y, tD);
  stbi_image_free (tD);
}

bmfont::~bmfont () {
  Chars.clear ();
  Kearn.clear ();
  engine::graph->delete_texture (ftexid);
}