#include "bmfont.hpp"
#include "stb_image.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

struct KearningInfo {
	short First, Second, Amount;
};

struct CharDescriptor {
	short x = 0, y = 0;
	short Width = 0;
	short Height = 0;
	short XOffset = 0, YOffset = 0;
	short XAdvance = 0;
	short Page = 0;
};

// Todo: Add buffer overflow checking.
//#define MAX_BUFFER 256

bool bmfont::ParseFont(const char *fontfile ) {
  unsigned int asl;
  const char *as = (const char*) engine::asset->asset_buffer(fontfile, &asl);
  std::string buffer(as,asl);
	std::stringstream buffer_stream(buffer);
	std::string Line;
	std::string Read, Key, Value;
	std::size_t i; 

  KearningInfo K;
  CharDescriptor C;

  while (!buffer_stream.eof()) {
    std::stringstream LineStream;
    std::getline (buffer_stream, Line);
    LineStream << Line;

    // read the line's type
    LineStream >> Read;
    /*if (Read == "info") {
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
          short fontSize;
          Converter >> fontSize;
          fscale = fontSize;
        }
      }
    } else */if (Read == "common") {
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
        } else if (Key == "pages") {
          Converter >> Pages;
        } else if (Key == "outline") {
          Converter >> Outline;
        }
      }
    } else if (Read == "char") {
      // This is data for each specific character.
      int CharID = 0;

      while (!LineStream.eof ()) {
        std::stringstream Converter;
        LineStream >> Read;
        i = Read.find ('=');
        Key = Read.substr (0, i);
        Value = Read.substr (i + 1);

        // Assign the correct value
        Converter << Value;
        if (Key == "id") {
          Converter >> CharID;
        } else if (Key == "x") {
          Converter >> C.x;
        } else if (Key == "y") {
          Converter >> C.y;
        } else if (Key == "width") {
          Converter >> C.Width;
        } else if (Key == "height") {
          Converter >> C.Height;
        } else if (Key == "xoffset") {
          Converter >> C.XOffset;
        } else if (Key == "yoffset") {
          Converter >> C.YOffset;
        } else if (Key == "xadvance") {
          Converter >> C.XAdvance;
        } else if (Key == "page") {
          Converter >> C.Page;
        }
      }
      Chars[CharID] = C;
    } else if (Read == "kernings") {
      while (!LineStream.eof ()) {
        std::stringstream Converter;
        LineStream >> Read;
        i = Read.find ('=');
        Key = Read.substr (0, i);
        Value = Read.substr (i + 1);

        // assign the correct value
        Converter << Value;
        if (Key == "count") {
          int KernCount;
          Converter >> KernCount;
          Kearn.reserve (KernCount);
        }
      }
    } else if (Read == "kerning") {
      while (!LineStream.eof ()) {
        std::stringstream Converter;
        LineStream >> Read;
        i = Read.find ('=');
        Key = Read.substr (0, i);
        Value = Read.substr (i + 1);

        // assign the correct value
        Converter << Value;
        if (Key == "first")
          Converter >> K.First;
        else if (Key == "second")
          Converter >> K.Second;
        else if (Key == "amount")
          Converter >> K.Amount;
      }
      // wrlog("Done with this pass");
      Kearn.push_back (K);
    }
  }
  return true;
}

int bmfont::GetKerningPair (int first, int second) {
  if (!Kearn.empty ())
    for (const KearningInfo &ki : Kearn)
      if (ki.First == first && ki.Second == second)
        return ki.Amount;
  return 0;
}

float bmfont::GetStringWidth (const char *text) {
  float total = 0;
  while (*text) {
    if (Chars.find(*text) == Chars.end()) continue;
    total += Chars[*text].XAdvance;
    text++;
  }
  return total * fscale;
}

void bmfont::draw_text (float x, float y, Align align, const char *fmt, ...) {
  if (fmt == NULL)                // If There's No Text
    return;                       // Do Nothing
  va_list ap;                     // Pointer To List Of Arguments
  va_start (ap, fmt);             // Parses The String For Variables
  char text[1024] = "";            // Holds Our String
  vsprintf (text, fmt, ap);       // And Converts Symbols To Actual Numbers
  va_end (ap);

  (void)align;
  float x1,y1,x2,y2, u1, v1, u2, v2;
  y += LineHeight;
  const size_t n = strlen(text);
  engine::flat_vertex *texlst = (engine::flat_vertex*)alloca(n * 4 * sizeof(engine::flat_vertex));
  engine::flat_vertex *cur_tex = texlst;
  for (size_t i = 0; i < n; i++) {
    if (Chars.find(text[i]) == Chars.end()) continue;
    const CharDescriptor &f = Chars[text[i]];
    // max, min
    x1 = x + (f.XOffset * fscale); //minx
    y1 = y - (f.YOffset * fscale);  //maxy
    x2 = x1 + (f.Width * fscale); //maxx
    y2 = y1 - (f.Height * fscale);  //miny
    u1 = f.x / (float)Width;
    v1 = f.y / (float)Height;
    u2 = (f.x + f.Width) / (float)Width;
    v2 = (f.y + f.Height) / (float)Height;

    // 0,1 Texture Coord, minxy
    cur_tex->u = u1;
    cur_tex->v = v2;
    cur_tex->x = x1;
    cur_tex->y = y2;
    memcpy (cur_tex->color, &fcolor, 4 * sizeof (unsigned char));
    
    cur_tex++;
    // 0,0 Texture Coord, minx maxy
    cur_tex->u = u1;
    cur_tex->v = v1;
    cur_tex->x = x1;
    cur_tex->y = y1;
    memcpy (cur_tex->color, &fcolor, 4 * sizeof (unsigned char));

    cur_tex++;
    // 1,1 Texture Coord, maxxy
    cur_tex->u = u2;
    cur_tex->v = v2;
    cur_tex->x = x2;
    cur_tex->y = y2;
    memcpy (cur_tex->color, &fcolor, 4 * sizeof (unsigned char));

    cur_tex++;
    // 1,0 Texture Coord, maxx miny
    cur_tex->u = u2;
    cur_tex->v = v1;
    cur_tex->x = x2;
    cur_tex->y = y1;
    memcpy (cur_tex->color, &fcolor, 4 * sizeof (unsigned char));

    cur_tex++;
    // Only check kerning if there is greater then 1 character and
    // if the check character is 1 less then the end of the string.
    if (n > 1) {
      x += GetKerningPair (text[i], text[i + 1]) * fscale;
    }
    x += f.XAdvance * fscale;
  }
  engine::graph->flat_render(ftexid, texlst, n);
}
void bmfont::draw_text_center (float y, const char *t) {
  float x = 0;
  for (const char *text = t; *text; text++) {
    if (Chars.find(*text)==Chars.end()) continue;
    if (*(text+1)) {
      x += (float)GetKerningPair (*text, *(text+1));
    }
    x += (float)Chars[*text].XAdvance;
    text++;
  }
  x *= fscale * 0.5f;
  draw_text ((engine::graph->getWidth () * 0.5f) - x, y, ALIGN_TOP_LEFT, t);
}
bmfont::bmfont(const char *fontfile) : fcolor (0xffffffff), ftexid (nullptr), fscale (3.f) {
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

bmfont::~bmfont() {
  Chars.clear ();
  Kearn.clear ();
  engine::graph->delete_texture (ftexid);
}