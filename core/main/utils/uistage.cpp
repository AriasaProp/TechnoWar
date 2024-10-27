#include "uistage.hpp"
#include "../engine.hpp"
#include "../qoi/qoi.hpp"
#include "../stbi/stbi_load.hpp"
#include "value.hpp"

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
#define TEMP_SIZE 65535 // 65536 - 1 = 0xffff

namespace uiskin {
engine::texture_core *tex = nullptr;
std::unordered_map<std::string, uistage::texture_region> regions;
} // namespace uiskin
engine::flat_vertex temp_vert[MAX_UI_DRAW * 4]; //= 20 KB, approximate 1024 actors can be drawn at once
char temp_char_buffer[1024];                    // for 1 kB => 4 kbit

struct CharDescriptor {
  short x, y;
  short Width, Height;
  short XOffset, YOffset;
  short XAdvance;
};

struct bmfont {
  short LineHeight;
  short Width;
  short Height;
  short Base;
  short Outline;
  color32_t fcolor;
  float fontSizeBase;
  float fontSizeUsed;
  std::unordered_map<int, CharDescriptor> Chars;
  std::unordered_map<uint32_t, float> Kearn;
  engine::texture_core *ftexid;
  float fscale ();
  bmfont (const char *);
  ~bmfont ();
} *font = nullptr;

struct tooltip {
  float lifetime, width;
  std::string message;
} tooltips[7];
static std::unordered_set<uistage::actor *> actors;

static float yList[2], vList[2], xList[2], uList[2];

void uistage::loadBMFont (const char *fontFile) {
  font = new bmfont (fontFile);
  font->fontSizeUsed = 40.f;
}
void uistage::loadUISkin (const char *uiSkin) {
  {
    sprintf (temp_char_buffer, "%s.txt", uiSkin);
    unsigned int asl;
    void *as = engine::assets::asset_buffer (temp_char_buffer, &asl);
    std::stringstream buffer_stream (std::string ((const char *)as, asl)), line_stream;
    free (as);

    std::string line_str, name;

    while (!buffer_stream.eof ()) {
      line_stream.clear ();
      std::getline (buffer_stream, line_str);
      line_stream << line_str;
      std::getline (line_stream, name, ':');
      uistage::texture_region region;
      region.rc.color = 0xffffffff;
      line_stream >> region.pos[0];
      line_stream >> region.pos[1];
      line_stream >> region.size[0];
      line_stream >> region.size[1];
      uiskin::regions.insert ({name, region});
    }
  }
  {
    sprintf (temp_char_buffer, "%s.qoi", uiSkin);
    qoi_desc d;
    unsigned int l;
    void *user = engine::assets::asset_buffer (temp_char_buffer, &l);
    unsigned char *tex_px = qoi_decode ((const unsigned char *)user, l, &d, 4);
    free (user);
    uiskin::tex = engine::graphics::gen_texture (d.width, d.height, tex_px);
    delete[] tex_px;
  }
}
void uistage::draw (float delta) {
  static color32_t clr;
  // draw
  for (actor *act : actors) {
    act->draw (delta);
  }
  size_t to_drawn = 7;
  engine::flat_vertex *verts = temp_vert;
  // touch pointer care
  {
    // background
    clr.color = 0xff999999;
    *(verts++) = {435.5f, engine::graphics::getHeight () - 155.0f, clr, 0, 1};
    *(verts++) = {435.5f, engine::graphics::getHeight () - 095.0f, clr, 0, 0};
    *(verts++) = {805.5f, engine::graphics::getHeight () - 155.0f, clr, 1, 1};
    *(verts++) = {805.5f, engine::graphics::getHeight () - 095.0f, clr, 1, 0};
    // ptr 1
    clr.color = engine::input::isTouched (0) ? 0xff00ff00 : 0xff0000ff;
    *(verts++) = {440.0f, engine::graphics::getHeight () - 150.0f, clr, 0, 1};
    *(verts++) = {440.0f, engine::graphics::getHeight () - 100.0f, clr, 0, 0};
    *(verts++) = {490.0f, engine::graphics::getHeight () - 150.0f, clr, 1, 1};
    *(verts++) = {490.0f, engine::graphics::getHeight () - 100.0f, clr, 1, 0};
    // ptr 2
    clr.color = engine::input::isTouched (1) ? 0xff00ff00 : 0xff0000ff;
    *(verts++) = {500.0f, engine::graphics::getHeight () - 150.0f, clr, 0, 1};
    *(verts++) = {500.0f, engine::graphics::getHeight () - 100.0f, clr, 0, 0};
    *(verts++) = {550.0f, engine::graphics::getHeight () - 150.0f, clr, 1, 1};
    *(verts++) = {550.0f, engine::graphics::getHeight () - 100.0f, clr, 1, 0};
    // ptr 3
    clr.color = engine::input::isTouched (2) ? 0xff00ff00 : 0xff0000ff;
    *(verts++) = {560.0f, engine::graphics::getHeight () - 150.0f, clr, 0, 1};
    *(verts++) = {560.0f, engine::graphics::getHeight () - 100.0f, clr, 0, 0};
    *(verts++) = {610.0f, engine::graphics::getHeight () - 150.0f, clr, 1, 1};
    *(verts++) = {610.0f, engine::graphics::getHeight () - 100.0f, clr, 1, 0};
    // ptr 4
    clr.color = engine::input::isTouched (3) ? 0xff00ff00 : 0xff0000ff;
    *(verts++) = {620.0f, engine::graphics::getHeight () - 150.0f, clr, 0, 1};
    *(verts++) = {620.0f, engine::graphics::getHeight () - 100.0f, clr, 0, 0};
    *(verts++) = {670.0f, engine::graphics::getHeight () - 150.0f, clr, 1, 1};
    *(verts++) = {670.0f, engine::graphics::getHeight () - 100.0f, clr, 1, 0};
    // ptr 5
    clr.color = engine::input::isTouched (4) ? 0xff00ff00 : 0xff0000ff;
    *(verts++) = {680.0f, engine::graphics::getHeight () - 150.0f, clr, 0, 1};
    *(verts++) = {680.0f, engine::graphics::getHeight () - 100.0f, clr, 0, 0};
    *(verts++) = {730.0f, engine::graphics::getHeight () - 150.0f, clr, 1, 1};
    *(verts++) = {730.0f, engine::graphics::getHeight () - 100.0f, clr, 1, 0};
    // ptr 6
    clr.color = engine::input::isTouched (5) ? 0xff00ff00 : 0xff0000ff;
    *(verts++) = {740.0f, engine::graphics::getHeight () - 150.0f, clr, 0, 1};
    *(verts++) = {740.0f, engine::graphics::getHeight () - 100.0f, clr, 0, 0};
    *(verts++) = {790.0f, engine::graphics::getHeight () - 150.0f, clr, 1, 1};
    *(verts++) = {790.0f, engine::graphics::getHeight () - 100.0f, clr, 1, 0};
  }
  // tooltip background drawn
  float F = font->fscale ();
  {
    // background
    clr.rgba = {0x00, 0x00, 0x00, 0x88};
    for (size_t i = 0; i < 7; ++i) {
      tooltip &tlp = tooltips[i];
      if (tlp.lifetime <= 0.0f) break;
      float x = (engine::graphics::getWidth () - tlp.width) * .5f;
      float y = engine::graphics::getHeight () * 0.75f + ((static_cast<float> (font->LineHeight) * F) + 10.5f) * i + 10.5f;

      xList[0] = x - 5.0f;                                               // minx
      xList[1] = x + tlp.width + 5.0f;                                   // maxx
      yList[0] = y - (static_cast<float> (font->LineHeight) * F) - 5.0f; // miny
      yList[1] = y + 5.0f;                                               // maxy
      if (tlp.lifetime < 1.65f)
        clr.rgba.a = (uint8_t)round (136.0f * tlp.lifetime / 1.65f);
      else
        clr.rgba.a = 0x88;
      *(verts++) = {xList[0], yList[0], clr, 0, 0};
      *(verts++) = {xList[0], yList[1], clr, 0, 0};
      *(verts++) = {xList[1], yList[0], clr, 0, 0};
      *(verts++) = {xList[1], yList[1], clr, 0, 0};
      ++to_drawn;
    }
  }
  if (to_drawn)
    engine::graphics::flat_render (nullptr, temp_vert, to_drawn);
  to_drawn = 0;
  verts = temp_vert;
  {
    // text tooltip
    for (size_t i = 0; i < 7; ++i) {
      tooltip &tlp = tooltips[i];
      if (tlp.lifetime <= 0.0f) break;
      clr = font->fcolor;
      if (tlp.lifetime < 1.65f)
        clr.rgba.a = (uint8_t)round (255.0f * tlp.lifetime / 1.65f);
      float x = (engine::graphics::getWidth () - tlp.width) * .5f;
      float y = engine::graphics::getHeight () * 0.75f + ((static_cast<float> (font->LineHeight) * font->fscale ()) + 10.5f) * i + 10.5f;

      auto &Chars = font->Chars;
      for (const char *t = tlp.message.c_str (); *t; t++) {
        auto itf = Chars.find (*t);
        if (itf == Chars.end ()) continue;
        CharDescriptor &f = itf->second;
        xList[0] = x + (f.XOffset * F);       // minx
        yList[1] = y - (f.YOffset * F);       // maxy
        xList[1] = xList[0] + (f.Width * F);  // maxx
        yList[0] = yList[1] - (f.Height * F); // miny

        uList[0] = f.x / (float)font->Width;
        vList[0] = f.y / (float)font->Height;
        uList[1] = (f.x + f.Width) / (float)font->Width;
        vList[1] = (f.y + f.Height) / (float)font->Height;

        *(verts++) = {xList[0], yList[0], clr, uList[0], vList[1]};
        *(verts++) = {xList[0], yList[1], clr, uList[0], vList[0]};
        *(verts++) = {xList[1], yList[0], clr, uList[1], vList[1]};
        *(verts++) = {xList[1], yList[1], clr, uList[1], vList[0]};

        if (*(t + 1)) {
          x += f.XAdvance * F;
          uint16_t key[2] = {static_cast<uint16_t> (*t), static_cast<uint16_t> (*(t + 1))};
          auto &Kearn = font->Kearn;
          auto it = Kearn.find (*reinterpret_cast<uint32_t *> (key));
          if (it != Kearn.end ())
            x += it->second * F;
        }
      }
      tlp.lifetime -= delta;
      to_drawn += tlp.message.size ();
    }
  }
  if (to_drawn)
    engine::graphics::flat_render (font->ftexid, temp_vert, to_drawn);
}
void uistage::clear () {
  memset (tooltips, 0, sizeof tooltips);
  if (uiskin::tex) engine::graphics::delete_texture (uiskin::tex);
  uiskin::regions.clear ();
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
  std::copy (k.begin (), k.end (), K);
  uistage::button_actor *ua = new button_actor (K, r, onclick);
  actors.insert (ua);
  return ua;
}
uistage::text_actor *uistage::makeText (Vector2 pos, const Align &a, std::string k) {
  uistage::text_actor *ua = new text_actor (pos, a, k.c_str ());
  actors.insert (ua);
  return ua;
}
void uistage::temporaryTooltip () {
  std::unordered_map<std::string, uistage::texture_region>::iterator it = uiskin::regions.begin ();
  temporaryTooltip (it->first.c_str ());
}
void uistage::temporaryTooltip (const char *fmt, ...) {
  if (fmt == NULL)
    return;
  size_t i = 6;
  do {
    tooltips[i] = tooltips[i - 1];
    tooltips[i].lifetime -= 0.2f;
  } while (--i);
  tooltips[i].lifetime = TOOLTIP_DURATION;
  va_list ap;
  va_start (ap, fmt);
  vsprintf (temp_char_buffer, fmt, ap);
  va_end (ap);
  tooltips[i].message = temp_char_buffer;
  auto &Chars = font->Chars;
  float width = 0;
  for (const char *t = tooltips[i].message.c_str (); *t; ++t) {
    if (Chars.find (*t) == Chars.end ()) continue;
    width += Chars[*t].XAdvance;
    if (*(t + 1)) {
      uint16_t key[2] = {static_cast<uint16_t> (*t), static_cast<uint16_t> (*(t + 1))};
      auto &Kearn = font->Kearn;
      auto it = Kearn.find (*reinterpret_cast<uint32_t *> (key));
      if (it != Kearn.end ())
        width += it->second;
    }
  }
  width *= font->fscale ();
  tooltips[i].width = width;
}
uistage::actor *focused_actor[100]{};
void uistage::touchDown (float x, float y, int pointer, int button) {
  for (actor *act : actors) {
    button_actor *button_act = dynamic_cast<button_actor *> (act);
    if (button_act && act->getRect ().insetOf (x, y)) {
      button_act->setState (1);
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
  actor *act = focused_actor[pointer];
  if (act) {
    button_actor *button_act = dynamic_cast<button_actor *> (act);
    if (button_act) {
      button_act->setState (0);
      if (button_act->onClick != NULL)
        button_act->onClick ();
    }
    focused_actor[pointer] = nullptr;
  }
  (void)x;
  (void)y;
  (void)button;
}
void uistage::touchCanceled (float x, float y, int pointer, int button) {
  actor *act = focused_actor[pointer];
  if (act) {
    button_actor *button_act = dynamic_cast<button_actor *> (act);
    if (button_act) button_act->setState (0);
    focused_actor[pointer] = nullptr;
  }
  (void)x;
  (void)y;
  (void)button;
}

// define bmfont source

float bmfont::fscale () { return fontSizeUsed / fontSizeBase; }

bmfont::bmfont (const char *fontfile) {
  // parse fnt
  fcolor.color = 0xffffffff;
  {
    unsigned int asl;
    void *as = engine::assets::asset_buffer (fontfile, &asl);
    std::stringstream buffer_stream (std::string ((const char *)as, asl));
    free (as);
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
        Kearn[*reinterpret_cast<uint32_t *> (id)] = amount;
      }
    }
  }
  int x, y;
  unsigned int datRI;
  char *texfile = new char[strlen (fontfile)];
  memcpy (texfile, fontfile, strlen (fontfile));
  memcpy (strstr (texfile, ".fnt"), ".png", 4);
  void *datR = engine::assets::asset_buffer (texfile, &datRI);
  delete[] texfile;
  unsigned char *tD = stbi::load::load_from_memory ((unsigned char const *)datR, (int)datRI, &x, &y, nullptr, stbi::load::channel::rgb_alpha);
  free (datR);
  ftexid = engine::graphics::gen_texture (x, y, tD);
  stbi::load::image_free (tD);
}

bmfont::~bmfont () {
  Chars.clear ();
  Kearn.clear ();
  engine::graphics::delete_texture (ftexid);
}
// redefine actor
void uistage::actor::draw (float delta) {
  if (getKey ().empty ()) return;
  (void)delta;
  uistage::texture_region &ta = uiskin::regions[getKey ()];
  // left, top, right, bottom
  const unsigned int *split = ta.patch;
  size_t quadCount = 0;
  engine::flat_vertex *verts = temp_vert;
  float rectangle[4];
  getRect ().getFloats (rectangle);
  // vertically 1
  if (split[3]) { // horizontally
    yList[0] = rectangle[1];
    yList[1] = rectangle[1] + split[3];
    vList[0] = float (ta.pos[1] + ta.size[1]) / float (uiskin::tex->height ());
    vList[1] = float (ta.pos[1] + ta.size[1] - split[3]) / float (uiskin::tex->height ());
    if (split[0]) {
      xList[0] = rectangle[0];
      xList[1] = rectangle[0] + split[0];
      uList[0] = float (ta.pos[0]) / float (uiskin::tex->width ());
      uList[1] = float (ta.pos[0] + split[0]) / float (uiskin::tex->width ());
      *(verts++) = {xList[0], yList[0], ta.rc, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.rc, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.rc, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.rc, uList[1], vList[1]};
      quadCount++;
    }
    xList[0] = rectangle[0] + split[0];
    xList[1] = rectangle[2] - split[2];
    if (xList[1] > xList[0]) {
      uList[0] = float (ta.pos[0] + split[0]) / float (uiskin::tex->width ());
      uList[1] = float (ta.pos[0] + ta.size[0] - split[2]) / float (uiskin::tex->width ());
      *(verts++) = {xList[0], yList[0], ta.rc, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.rc, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.rc, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.rc, uList[1], vList[1]};
      quadCount++;
    }
    if (split[2]) {
      xList[0] = rectangle[2] - split[2];
      xList[1] = rectangle[2];
      uList[0] = float (ta.pos[0] + ta.size[0] - split[2]) / float (uiskin::tex->width ());
      uList[1] = float (ta.pos[0] + ta.size[0]) / float (uiskin::tex->width ());
      *(verts++) = {xList[0], yList[0], ta.rc, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.rc, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.rc, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.rc, uList[1], vList[1]};
      quadCount++;
    }
  }
  // vertically 2
  yList[0] = rectangle[1] + split[3];
  yList[1] = rectangle[3] - split[1];
  if (yList[1] > yList[0]) { // horizontally
    vList[0] = float (ta.pos[1] + ta.size[1] - split[3]) / float (uiskin::tex->height ());
    vList[1] = float (ta.pos[1] + split[1]) / float (uiskin::tex->height ());
    if (split[0]) {
      xList[0] = rectangle[0];
      xList[1] = rectangle[0] + split[0];
      uList[0] = float (ta.pos[0]) / float (uiskin::tex->width ());
      uList[1] = float (ta.pos[0] + split[0]) / float (uiskin::tex->width ());
      *(verts++) = {xList[0], yList[0], ta.rc, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.rc, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.rc, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.rc, uList[1], vList[1]};
      quadCount++;
    }
    xList[0] = rectangle[0] + split[0];
    xList[1] = rectangle[2] - split[2];
    if (xList[1] > xList[0]) {
      uList[0] = float (ta.pos[0] + split[0]) / float (uiskin::tex->width ());
      uList[1] = float (ta.pos[0] + ta.size[0] - split[2]) / float (uiskin::tex->width ());
      *(verts++) = {xList[0], yList[0], ta.rc, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.rc, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.rc, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.rc, uList[1], vList[1]};
      quadCount++;
    }
    if (split[2]) {
      xList[0] = rectangle[2] - split[2];
      xList[1] = rectangle[2];
      uList[0] = float (ta.pos[0] + ta.size[0] - split[2]) / float (uiskin::tex->width ());
      uList[1] = float (ta.pos[0] + ta.size[0]) / float (uiskin::tex->width ());
      *(verts++) = {xList[0], yList[0], ta.rc, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.rc, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.rc, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.rc, uList[1], vList[1]};
      quadCount++;
    }
  }
  // vertically 3
  if (split[1]) { // horizontally
    yList[0] = rectangle[3] - split[1];
    yList[1] = rectangle[3];
    vList[0] = float (ta.pos[1] + split[1]) / float (uiskin::tex->height ());
    vList[1] = float (ta.pos[1]) / float (uiskin::tex->height ());
    if (split[0]) {
      xList[0] = rectangle[0];
      xList[1] = rectangle[0] + split[0];
      uList[0] = float (ta.pos[0]) / float (uiskin::tex->width ());
      uList[1] = float (ta.pos[0] + split[0]) / float (uiskin::tex->width ());
      *(verts++) = {xList[0], yList[0], ta.rc, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.rc, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.rc, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.rc, uList[1], vList[1]};
      quadCount++;
    }
    xList[0] = rectangle[0] + split[0];
    xList[1] = rectangle[2] - split[2];
    if (xList[1] > xList[0]) {
      uList[0] = float (ta.pos[0] + split[0]) / float (uiskin::tex->width ());
      uList[1] = float (ta.pos[0] + ta.size[0] - split[2]) / float (uiskin::tex->width ());
      *(verts++) = {xList[0], yList[0], ta.rc, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.rc, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.rc, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.rc, uList[1], vList[1]};
      quadCount++;
    }
    if (split[2]) {
      xList[0] = rectangle[2] - split[2];
      xList[1] = rectangle[2];
      uList[0] = float (ta.pos[0] + ta.size[0] - split[2]) / float (uiskin::tex->width ());
      uList[1] = float (ta.pos[0] + ta.size[0]) / float (uiskin::tex->width ());
      *(verts++) = {xList[0], yList[0], ta.rc, uList[0], vList[0]};
      *(verts++) = {xList[0], yList[1], ta.rc, uList[0], vList[1]};
      *(verts++) = {xList[1], yList[0], ta.rc, uList[1], vList[0]};
      *(verts++) = {xList[1], yList[1], ta.rc, uList[1], vList[1]};
      quadCount++;
    }
  }
  engine::graphics::flat_render (uiskin::tex, temp_vert, quadCount);
}

uistage::text_actor::text_actor (Vector2 _pos, const Align &a, std::string ti) : text (ti) {
  float width = 0;
  auto &Chars = font->Chars;
  auto &Kearn = font->Kearn;
  for (const char *t = text.c_str (); *t; t++) {
    if (Chars.find (*t) == Chars.end ()) continue;
    width += Chars[*t].XAdvance;
    if (*(t + 1)) {
      uint16_t key[2] = {static_cast<uint16_t> (*t), static_cast<uint16_t> (*(t + 1))};
      auto it = Kearn.find (*reinterpret_cast<uint32_t *> (key));
      if (it != Kearn.end ())
        width += it->second;
    }
  }
  float out[2];
  _pos.getFloats (out);
  rectangle = Rect (out[0], out[1], width * font->fscale (), (static_cast<float> (font->LineHeight) * font->fscale ()), ALIGN_BOTTOM_LEFT, a);
}
Rect &uistage::text_actor::getRect () { return rectangle; }
std::string uistage::text_actor::getKey () { return ""; }
void uistage::text_actor::draw (float delta) {
  uistage::actor::draw (delta);
  float F = font->fscale ();
  auto &Chars = font->Chars;
  engine::flat_vertex *verts = temp_vert;
  auto &Kearn = font->Kearn;
  float r[4];
  rectangle.getFloats (r);
  float x = r[0];
  for (const char *t = text.c_str (); *t; t++) {
    auto itf = Chars.find (*t);
    if (itf == Chars.end ()) continue;
    CharDescriptor &f = itf->second;
    xList[0] = x + (f.XOffset * F);       // minx
    yList[1] = r[3] - (f.YOffset * F);    // maxy
    xList[1] = xList[0] + (f.Width * F);  // maxx
    yList[0] = yList[1] - (f.Height * F); // miny

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
      auto it = Kearn.find (*reinterpret_cast<uint32_t *> (key));
      if (it != Kearn.end ())
        nX += it->second;
      x += nX * F;
    }
  }
  engine::graphics::flat_render (font->ftexid, temp_vert, text.size ());
}
void uistage::text_actor::setText (const char *fmt, ...) {
  if (fmt == NULL)
    return;
  va_list ap;
  va_start (ap, fmt);
  vsprintf (temp_char_buffer, fmt, ap);
  va_end (ap);
  text = temp_char_buffer;
}
uistage::text_actor::~text_actor () {}

uistage::image_actor::image_actor (std::string k, Rect r) : key (k), rectangle (r) {}
Rect &uistage::image_actor::getRect () { return rectangle; }
std::string uistage::image_actor::getKey () { return key; }
void uistage::image_actor::draw (float delta) {
  uistage::actor::draw (delta);
}
uistage::image_actor::~image_actor () {}

uistage::button_actor::button_actor (std::string *k, Rect r, void (*onclick) ()) : keys (k), rectangle (r), onClick (onclick) {}
Rect &uistage::button_actor::getRect () { return rectangle; }
std::string uistage::button_actor::getKey () {
  if (!keys[mstate].empty ())
    return keys[mstate];
  else if (!keys[0].empty ())
    return keys[0];
  else
    return "";
}
void uistage::button_actor::setState (size_t state) { mstate = state; }
void uistage::button_actor::draw (float delta) {
  uistage::actor::draw (delta);
}
uistage::button_actor::~button_actor () { delete[] keys; }
