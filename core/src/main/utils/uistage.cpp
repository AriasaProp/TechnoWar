#include "../assets/stb_image.hpp"
#include "uistage.hpp"
#include "../engine.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#define TOOLTIP_DURATION 4.75f
#define TEMP_SIZE 4095

union glb_tmp {
  char char_buffer[1024]; // for 1 kB => 4 kbit
} global_temporary;

struct CharDescriptor {
  short x, y;
  short Width, Height;
  short XOffset, YOffset;
  short XAdvance;
};

struct bmfont {
private:
  short LineHeight;
public:
  short Width;
  short Height;
  short Base;
  short Outline;
  std::unordered_map<int, CharDescriptor> Chars;
  std::unordered_map<uint32_t, float> Kearn;
  uint32_t fcolor;
  engine::texture_core *ftexid;
  float fontSizeBase, fontSizeUsed;
  float fscale ();
  void SetColor (unsigned char, unsigned char, unsigned char, unsigned char);
  void setFontSize (float); // px
  float getLineHeight();
  bmfont (const char *);
  ~bmfont ();

} *font = nullptr;

struct textureAtlas {
  engine::texture_core *tex;
  uistage::texture_region region;
  uint32_t clr;
};
struct tooltip {
  float lifetime; // in period 10000 of period as delta time
  std::string message;
} tooltips[10];
static std::unordered_map<std::string, textureAtlas> regions;
// static engine::texture_core *binded = nullptr;
static std::unordered_set<uistage::actor *> actors;

static engine::flat_vertex vert[TEMP_SIZE]; //= 20 KB, approximate 1024 actors can be drawn at once
static float yList[2], vList[2], xList[2], uList[2];
enum Actor_Type : size_t { None = 0, Static, Button };

void uistage::loadBMFont (const char *fontFile) {
  font = new bmfont (fontFile);
  font->setFontSize (40.f);
}
void uistage::addTextureRegion (std::string key, engine::texture_core *tex, const uistage::texture_region &reg) {
  regions[key] = textureAtlas{tex, reg, 0xffffffff};
}
void uistage::addTextureRegion (std::string key, engine::texture_core *tex, const uistage::texture_region &reg, const uint32_t clr) {
  regions[key] = textureAtlas{tex, reg, clr};
}
void uistage::draw (float delta) {
  // draw
  for (actor *act : actors) {
    act->draw (delta);
  }
  //tooltip drawn
  {
    size_t tooltip_drawn = 0;
    engine::flat_vertex *verts = vert;
    //background
    for (size_t i = 0; i < 10; ++i) {
      tooltip &tlp = tooltips[i];
      if (tlp.lifetime <= 0.0f) {
        tlp.message = "";
        break;
      }
      auto &Chars = font->Chars;
      float width = 0;
      float F = font->fscale ();
      for (const char *t = tlp.message.c_str(); *t; ++t) {
        if (Chars.find (*t) == Chars.end ()) continue;
        width += Chars[*t].XAdvance;
        if (*(t + 1)) {
          uint16_t key[2] = {static_cast<uint16_t> (*t), static_cast<uint16_t> (*(t + 1))};
          auto &Kearn = font->Kearn;
          auto it = Kearn.find (*reinterpret_cast<uint32_t*>(key));
          if (it != Kearn.end ())
            width += it->second;
        }
      }
      width *= F;
      xList[0] = x - 5.0f; // minx
      xList[1] = x + width + 5.0f; // maxx
      yList[0] = y - font->getLineHeight() - 5.0f; // miny
      yList[1] = y + 5.0f; // maxy
      uint32_t bc = 0x88000000;
      if (tlp.lifetime < 1.65f) {
        float transitionAlpha = tlp.lifetime/1.65f;
        ((uint8_t*)&bc)[3] = static_cast<uint8_t>(static_cast<float>(((uint8_t*)&bc)[3]) * transitionAlpha);
      }
      *(verts++) = {xList[0], yList[0], bc, 0, 0};
      *(verts++) = {xList[0], yList[1], bc, 0, 0};
      *(verts++) = {xList[1], yList[0], bc, 0, 0};
      *(verts++) = {xList[1], yList[1], bc, 0, 0};
      ++tooltip_drawn;
    }
    if (tooltip_drawn)
      engine::graph->flat_render (nullptr, vert, tooltip_drawn);
    //text
    size_t text_tooltip_drawn = 0;
    verts = vert;
    for (size_t i = 0; i < 10; ++i) {
      tooltip &tlp = tooltips[i];
      if (tlp.lifetime <= 0.0f) break;
      auto &Chars = font->Chars;
      float width = 0;
      float F = font->fscale ();
      for (const char *t = tlp.message.c_str(); *t; ++t) {
        if (Chars.find (*t) == Chars.end ()) continue;
        width += Chars[*t].XAdvance;
        if (*(t + 1)) {
          uint16_t key[2] = {static_cast<uint16_t> (*t), static_cast<uint16_t> (*(t + 1))};
          auto &Kearn = font->Kearn;
          auto it = Kearn.find (*reinterpret_cast<uint32_t*>(key));
          if (it != Kearn.end ())
            width += it->second;
        }
      }
      width *= F;
      uint32_t hc = font->fcolor;
      if (tlp.lifetime < 1.65f) {
        float transitionAlpha = tlp.lifetime/1.65f;
        ((uint8_t*)&hc)[3] = static_cast<uint8_t>(static_cast<float>(((uint8_t*)&hc)[3]) * transitionAlpha);
      }
      float x = (engine::graph->getWidth() - width) *.5f;
      float y = engine::graph->getHeight() * 0.75f + (font->getLineHeight() + 10.5f) * i + 10.5f;
      
      for (const char *t = tlp.message.c_str(); *t; t++) {
        auto itf = Chars.find (*t);
        if (itf == Chars.end ()) continue;
        CharDescriptor &f = itf->second;
        xList[0] = x + (f.XOffset * F); // minx
        yList[1] = y - (f.YOffset * F); // maxy
        xList[1] = xList[0] + (f.Width * F); // maxx
        yList[0] = yList[1] - (f.Height * F); // miny
      
        uList[0] = f.x / (float)font->Width;
        vList[0] = f.y / (float)font->Height;
        uList[1] = (f.x + f.Width) / (float)font->Width;
        vList[1] = (f.y + f.Height) / (float)font->Height;
      
        *(verts++) = {xList[0], yList[0], hc, uList[0], vList[1]};
        *(verts++) = {xList[0], yList[1], hc, uList[0], vList[0]};
        *(verts++) = {xList[1], yList[0], hc, uList[1], vList[1]};
        *(verts++) = {xList[1], yList[1], hc, uList[1], vList[0]};
      
        if (*(t + 1)) {
          x += f.XAdvance * F;
          uint16_t key[2] = {static_cast<uint16_t> (*t), static_cast<uint16_t> (*(t + 1))};
          auto &Kearn = font->Kearn;
          auto it = Kearn.find (*reinterpret_cast<uint32_t*>(key));
          if (it != Kearn.end ())
            x += it->second * F;
        }
      }
      tlp.lifetime -= delta;
      text_tooltip_drawn += tlp.message.size();
    }
    if (text_tooltip_drawn)
      engine::graph->flat_render (font->ftexid, vert, text_tooltip_drawn);
  }
}
void uistage::cleartemp () {
  memset(tooltips, 0, sizeof tooltips);
}
void uistage::clear () {
  for (auto i = regions.begin (), j = regions.end (); i != j; i++) {
    delete i->second.tex;
  }
  regions.clear ();
  for (uistage::actor *a : actors)
    delete a;
  actors.clear ();
  delete font;
}

uistage::image_actor *uistage::makeImage (std::string k, Rect r) {
  uistage::image_actor *ua = new image_actor (k, r);
  actors.insert (ua);
  return ua;
}

uistage::button_actor *uistage::makeButton (std::initializer_list<std::string> k, Rect r, void (*onclick) ()) {
  if (!k.size ()) throw ("button must have a key texture");
  std::string *K = new std::string[k.size ()];
  size_t i = 0;
  const std::string *it = k.begin (), *end = k.end ();
  while (it != end) {
    K[i] = *it;
    ++it;
    ++i;
  }
  uistage::button_actor *ua = new button_actor (K, r, onclick);
  actors.insert (ua);
  return ua;
}
uistage::text_actor *uistage::makeText (float x, float y, Align a, std::string k) {
  uistage::text_actor *ua = new text_actor (x, y, a, k.c_str ());
  actors.insert (ua);
  return ua;
}
void uistage::temporaryTooltip(const char *fmt, ...) {
  if (fmt == NULL)
    return;
  size_t i = 9;
  do {
    tooltips[i].lifetime = tooltips[i-1].lifetime;
    tooltips[i].message = tooltips[i-1].message;
  } while ((--i) != 0);
  tooltips[i].lifetime = TOOLTIP_DURATION;
  
  va_list ap;
  va_start (ap, fmt);
  vsprintf (global_temporary.char_buffer, fmt, ap);
  va_end (ap);
  tooltips[i].message = global_temporary.char_buffer;
}
uistage::actor *focused_actor[100]{};
void uistage::touchDown (float x, float y, int pointer, int button) {
  for (actor *act : actors) {
    if ((act->getType () == Actor_Type::Button) && (act->getRect ().insetOf (x, y))) {
      ((button_actor *)act)->setState (1);
      focused_actor[pointer] = act;
      return;
    }
  }
  (void)button;
}
void uistage::touchMove (float x, float y, float xs, float ys, int pointer, int button) {

  (void)x;
  (void)y;
  (void)xs;
  (void)ys;
  (void)pointer;
  (void)button;
}
void uistage::touchUp (float x, float y, int pointer, int button) {
  if (focused_actor[pointer]) {
    actor *act = focused_actor[pointer];
    if (act->getType () == Actor_Type::Button) {
      button_actor *bact = (button_actor *)act;
      bact->setState (0);
      if (bact->onClick != NULL)
        bact->onClick ();
    }
    focused_actor[pointer] = nullptr;
  }
  (void)x;
  (void)y;
  (void)button;
}
void uistage::touchCanceled (float x, float y, int pointer, int button) {
  if (focused_actor[pointer]) {
    actor *act = focused_actor[pointer];
    if (act->getType () == Actor_Type::Button) {
      ((button_actor *)act)->setState (0);
    }
    focused_actor[pointer] = nullptr;
  }
  (void)x;
  (void)y;
  (void)button;
}

// define bmfont source

float bmfont::fscale () { return fontSizeUsed / fontSizeBase; }
void bmfont::SetColor (unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
  unsigned char *ucolor = reinterpret_cast<unsigned char*>(&fcolor);
  ucolor[0] = r;
  ucolor[1] = g;
  ucolor[2] = b;
  ucolor[3] = a;
}
void bmfont::setFontSize (float size) { // px
  fontSizeUsed = size;
}
float bmfont::getLineHeight() {
  return static_cast<float>(LineHeight) * fontSizeUsed / fontSizeBase;
}
bmfont::bmfont (const char *fontfile) : fcolor (0xffffffff), ftexid (nullptr) {
  // parse fnt
  {
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
        Kearn[*reinterpret_cast<uint32_t*>(id)] = amount;
      }
    }
  }
  int x, y;
  unsigned int datRI;
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
//{ redefine actor
void uistage::actor::draw (float delta) {
  if (getKey().empty()) return;
  (void)delta;
  textureAtlas &ta = regions[getKey()];
  engine::texture_core *tex = ta.tex;
  // left, top, right, bottom
  const unsigned int *split = ta.region.patch;
  size_t quadCount = 0;
  engine::flat_vertex *verts = vert;
  Rect &rectangle = getRect();
  // vertically 1
  if (split[3]) { // horizontally
    yList[0] = rectangle.ymin;
    yList[1] = rectangle.ymin + split[3];
    vList[0] = float (ta.region.pos[1] + ta.region.size[1]) / float (tex->height ());
    vList[1] = float (ta.region.pos[1] + ta.region.size[1] - split[3]) / float (tex->height ());
    if (split[0]) {
      xList[0] = rectangle.xmin;
      xList[1] = rectangle.xmin + split[0];
      uList[0] = float (ta.region.pos[0]) / float (tex->width ());
      uList[1] = float (ta.region.pos[0] + split[0]) / float (tex->width ());
      *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
      quadCount++;
    }
    xList[0] = rectangle.xmin + split[0];
    xList[1] = rectangle.xmax - split[2];
    if (xList[1] > xList[0]) {
      uList[0] = float (ta.region.pos[0] + split[0]) / float (tex->width ());
      uList[1] = float (ta.region.pos[0] + ta.region.size[0] - split[2]) / float (tex->width ());
      *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
      quadCount++;
    }
    if (split[2]) {
      xList[0] = rectangle.xmax - split[2];
      xList[1] = rectangle.xmax;
      uList[0] = float (ta.region.pos[0] + ta.region.size[0] - split[2]) / float (tex->width ());
      uList[1] = float (ta.region.pos[0] + ta.region.size[0]) / float (tex->width ());
      *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
      quadCount++;
    }
  }
  // vertically 2
  yList[0] = rectangle.ymin + split[3];
  yList[1] = rectangle.ymax - split[1];
  if (yList[1] > yList[0]) { // horizontally
    vList[0] = float (ta.region.pos[1] + ta.region.size[1] - split[3]) / float (tex->height ());
    vList[1] = float (ta.region.pos[1] + split[1]) / float (tex->height ());
    if (split[0]) {
      xList[0] = rectangle.xmin;
      xList[1] = rectangle.xmin + split[0];
      uList[0] = float (ta.region.pos[0]) / float (tex->width ());
      uList[1] = float (ta.region.pos[0] + split[0]) / float (tex->width ());
      *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
      quadCount++;
    }
    xList[0] = rectangle.xmin + split[0];
    xList[1] = rectangle.xmax - split[2];
    if (xList[1] > xList[0]) {
      uList[0] = float (ta.region.pos[0] + split[0]) / float (tex->width ());
      uList[1] = float (ta.region.pos[0] + ta.region.size[0] - split[2]) / float (tex->width ());
      *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
      quadCount++;
    }
    if (split[2]) {
      xList[0] = rectangle.xmax - split[2];
      xList[1] = rectangle.xmax;
      uList[0] = float (ta.region.pos[0] + ta.region.size[0] - split[2]) / float (tex->width ());
      uList[1] = float (ta.region.pos[0] + ta.region.size[0]) / float (tex->width ());
      *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
      quadCount++;
    }
  }
  // vertically 3
  if (split[1]) { // horizontally
    yList[0] = rectangle.ymax - split[1];
    yList[1] = rectangle.ymax;
    vList[0] = float (ta.region.pos[1] + split[1]) / float (tex->height ());
    vList[1] = float (ta.region.pos[1]) / float (tex->height ());
    if (split[0]) {
      xList[0] = rectangle.xmin;
      xList[1] = rectangle.xmin + split[0];
      uList[0] = float (ta.region.pos[0]) / float (tex->width ());
      uList[1] = float (ta.region.pos[0] + split[0]) / float (tex->width ());
      *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
      quadCount++;
    }
    xList[0] = rectangle.xmin + split[0];
    xList[1] = rectangle.xmax - split[2];
    if (xList[1] > xList[0]) {
      uList[0] = float (ta.region.pos[0] + split[0]) / float (tex->width ());
      uList[1] = float (ta.region.pos[0] + ta.region.size[0] - split[2]) / float (tex->width ());
      *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
      quadCount++;
    }
    if (split[2]) {
      xList[0] = rectangle.xmax - split[2];
      xList[1] = rectangle.xmax;
      uList[0] = float (ta.region.pos[0] + ta.region.size[0] - split[2]) / float (tex->width ());
      uList[1] = float (ta.region.pos[0] + ta.region.size[0]) / float (tex->width ());
      *(verts++) = {xList[0], yList[0], ta.clr, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.clr, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.clr, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.clr, uList[1], vList[1]};
      quadCount++;
    }
  }
  engine::graph->flat_render (tex, vert, quadCount);
}

uistage::text_actor::text_actor (float x, float y, Align a, std::string ti) : text (ti) {
  float width = 0;
  auto &Chars = font->Chars;
  for (const char *t = text.c_str(); *t; t++) {
    if (Chars.find (*t) == Chars.end ()) continue;
    width += Chars[*t].XAdvance;
  }
  rectangle = Rect (x, y, a, width * font->fscale (), font->getLineHeight());
}
Rect &uistage::text_actor::getRect () { return rectangle; }
std::string uistage::text_actor::getKey () { return ""; }
void uistage::text_actor::draw (float delta) {
  uistage::actor::draw (delta);
  float F = font->fscale ();
  auto &Chars = font->Chars;
  engine::flat_vertex *verts = vert;
  auto &Kearn = font->Kearn;
  float x = rectangle.xmin;
  for (const char *t = text.c_str(); *t; t++) {
    auto itf = Chars.find (*t);
    if (itf == Chars.end ()) continue;
    CharDescriptor &f = itf->second;
    xList[0] = x + (f.XOffset * F);              // minx
    yList[1] = rectangle.ymax - (f.YOffset * F); // maxy
    xList[1] = xList[0] + (f.Width * F);         // maxx
    yList[0] = yList[1] - (f.Height * F);        // miny

    uList[0] = f.x / (float)font->Width;
    vList[0] = f.y / (float)font->Height;
    uList[1] = (f.x + f.Width) / (float)font->Width;
    vList[1] = (f.y + f.Height) / (float)font->Height;

    *(verts++) = {xList[0], yList[0], font->fcolor, uList[0], vList[1]};
    *(verts++) = {xList[0], yList[1], font->fcolor, uList[0], vList[0]};
    *(verts++) = {xList[1], yList[0], font->fcolor, uList[1], vList[1]};
    *(verts++) = {xList[1], yList[1], font->fcolor, uList[1], vList[0]};

    if (*(t + 1)) {
      float nX = f.XAdvance;
      uint16_t key[2] = {static_cast<uint16_t> (*t), static_cast<uint16_t> (*(t + 1))};
      auto it = Kearn.find (*reinterpret_cast<uint32_t*>(key));
      if (it != Kearn.end ())
        nX += it->second;
      x += nX * F;
    }
  }
  engine::graph->flat_render (font->ftexid, vert, text.size());
}
size_t uistage::text_actor::getType () const { return Actor_Type::Static; }
void uistage::text_actor::setText(const char *fmt, ...) {
  if (fmt == NULL)
    return;
  va_list ap;
  va_start (ap, fmt);
  vsprintf (global_temporary.char_buffer, fmt, ap);
  va_end (ap);
  text = global_temporary.char_buffer;
}
uistage::text_actor::~text_actor () {}

uistage::image_actor::image_actor (std::string k, Rect r) : key (k), rectangle (r) {}
Rect &uistage::image_actor::getRect () { return rectangle; }
std::string uistage::image_actor::getKey () { return key; }
size_t uistage::image_actor::getType () const { return Actor_Type::Static; }
void uistage::image_actor::draw (float delta) {
  uistage::actor::draw (delta);
}
uistage::image_actor::~image_actor () {}

uistage::button_actor::button_actor (std::string *k, Rect r, void (*onclick) ()) : keys (k), rectangle (r), onClick (onclick) {}
Rect &uistage::button_actor::getRect () { return rectangle; }
std::string uistage::button_actor::getKey () {
  if (!keys[mstate].empty())
    return keys[mstate];
  else if (!keys[0].empty())
    return keys[0];
  else
    return "";
}
void uistage::button_actor::setState (size_t state) { mstate = state; }
size_t uistage::button_actor::getType () const { return Actor_Type::Button; }
void uistage::button_actor::draw (float delta) {
  uistage::actor::draw (delta);
}
uistage::button_actor::~button_actor () { delete[] keys; }

//}