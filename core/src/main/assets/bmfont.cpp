#include "bmfont.hpp"
#include "stb_image.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

struct CharDescriptor {
  short x = 0, y = 0;
  short Width = 0;
  short Height = 0;
  short XOffset = 0, YOffset = 0;
  short XAdvance = 0;
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
      short id[2];
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
      Kearn[*((unsigned int *)id)] = amount;
    }
  }
  return true;
}

void bmfont::draw_text (float x, float y, Align align, const char *fmt, ...) {
  if (fmt == NULL)          // If There's No Text
    return;                 // Do Nothing
  va_list ap;               // Pointer To List Of Arguments
  va_start (ap, fmt);       // Parses The String For Variables
  char text[1024] = "";     // Holds Our String
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
    ;
    break;
  }
  case 2: { // right
    float total = 0;
    for (const char *t = text; *t; t++) {
      if (Chars.find (*t) == Chars.end ()) continue;
      total += Chars[*t].XAdvance;
    }
    x -= total * F;
    ;
    break;
  }
  }
  unsigned char ypivot = (align >> 2);
  switch (ypivot) {
  default:
    break;
  case 1:
    y += LineHeight * F * 0.5f;
    break;
  case 2:
    y += LineHeight * F;
    break;
  }
  float x1, y1, x2, y2, u1, v1, u2, v2;
  unsigned int n = strlen (text);
  engine::flat_vertex *texlst = new engine::flat_vertex[n * 4];
  engine::flat_vertex *cur_tex = texlst;
  for (const char *t = text; *t; t++) {
    if (Chars.find (*t) == Chars.end ()) continue;
    const CharDescriptor &f = Chars[*t];
    // max, min
    x1 = x + (f.XOffset * F); // minx
    y1 = y - (f.YOffset * F); // maxy
    x2 = x1 + (f.Width * F);  // maxx
    y2 = y1 - (f.Height * F); // miny
    u1 = f.x / (float)Width;
    v1 = f.y / (float)Height;
    u2 = (f.x + f.Width) / (float)Width;
    v2 = (f.y + f.Height) / (float)Height;

    // 0,1 Texture Coord, minxy
    cur_tex->x = x1;
    cur_tex->y = y2;
    memcpy (&cur_tex->color, &fcolor, 4 * sizeof (unsigned char));
    cur_tex->u = u1;
    cur_tex->v = v2;

    cur_tex++;
    
    // 1,0 Texture Coord, maxx miny
    cur_tex->x = x2;
    cur_tex->y = y1;
    memcpy (&cur_tex->color, &fcolor, 4 * sizeof (unsigned char));
    cur_tex->u = u2;
    cur_tex->v = v1;

    cur_tex++;
    
    // 0,0 Texture Coord, minx maxy
    cur_tex->x = x1;
    cur_tex->y = y1;
    memcpy (&cur_tex->color, &fcolor, 4 * sizeof (unsigned char));
    cur_tex->u = u1;
    cur_tex->v = v1;

    cur_tex++;
    
    // 1,1 Texture Coord, maxxy
    cur_tex->x = x2;
    cur_tex->y = y2;
    memcpy (&cur_tex->color, &fcolor, 4 * sizeof (unsigned char));
    cur_tex->u = u2;
    cur_tex->v = v2;

    cur_tex++;
    
    if (*(t + 1)) {
      float nX = f.XAdvance;
      short key[2] = {*t, *(t + 1)};
      auto it = Kearn.find (*((unsigned int *)key));
      if (it != Kearn.end ())
        nX += it->second;
      x += nX * F;
    }
  }
  engine::graph->flat_render (nullptr/*ftexid*/, texlst, n);
}
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