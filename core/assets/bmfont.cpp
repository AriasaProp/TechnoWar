#include "bmfont.hpp"
#include "stb_image.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

// Todo: Add buffer overflow checking.
//#define MAX_BUFFER 256

engine::flat_vertex texlst[2048 * 4];

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
    if (Read == "common") {
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
    }

    else if (Read == "char") {
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

      Chars.insert (std::map<int, CharDescriptor>::value_type (CharID, C));

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
    }

    else if (Read == "kerning") {
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
  if (!Kearn.empty ()) { // Only process if there actually is kerning information
    // Kearning is checked for every character processed. This is expensive in terms of processing time.
    for (size_t i = 0, n = Kearn.size (); i < n; i++) {
      if (Kearn[i].First == first && Kearn[i].Second == second) {
        return Kearn[i].Amount;
      }
    }
  }
  return 0;
}

float bmfont::GetStringWidth (const char *string) {
  float total = 0;
  CharDescriptor *f;
  for (size_t i = 0, n = strlen (string); i < n; i++) {
    f = &Chars[string[i]];
    total += f->XAdvance;
  }
  return total * fscale;
}

void bmfont::Print (float x, float y, const char *fmt, ...) {
  float x1, y1,x2, y2;
  float adv = 1.0 / (float)Width; // Font texture atlas spacing.
  char text[512] = "";            // Holds Our String
  CharDescriptor *f;              // Pointer to font.
  va_list ap;                     // Pointer To List Of Arguments
  if (fmt == NULL)                // If There's No Text
    return;                       // Do Nothing
  va_start (ap, fmt);             // Parses The String For Variables
  vsprintf (text, fmt, ap);       // And Converts Symbols To Actual Numbers
  va_end (ap);

  y += LineHeight;

  for (size_t i = 0, n = strlen(text); i < n; i++) {
    f = &Chars[text[i]];
    // max, min
    x1 = x + f->XOffset; //minx
    y1 = y - f->YOffset; //maxy
    x2 = x1 + f->Width; //maxx
    y2 = y1 - f->Height; //miny

    // 0,1 Texture Coord, minxy
    texlst[i * 4].u = adv * f->x;
    texlst[i * 4].v = adv * (f->y + f->Height);
    texlst[i * 4].x = fscale * x1;
    texlst[i * 4].y = fscale * y2;
    memcpy (texlst[i * 4].color, &fcolor, 4 * sizeof (unsigned char));

    // 0,0 Texture Coord, minx maxy
    texlst[(i * 4) + 1].u = adv * f->x;
    texlst[(i * 4) + 1].v = adv * f->y;
    texlst[(i * 4) + 1].x = fscale * x1;
    texlst[(i * 4) + 1].y = fscale * y1;
    memcpy (texlst[(i * 4) + 3].color, &fcolor, 4 * sizeof (unsigned char));

    // 1,1 Texture Coord, maxxy
    texlst[(i * 4) + 2].u = adv * (f->x + f->Width);
    texlst[(i * 4) + 2].v = adv * (f->y + f->Height);
    texlst[(i * 4) + 2].x = fscale * x2;
    texlst[(i * 4) + 2].y = fscale * y1;
    memcpy (texlst[(i * 4) + 2].color, &fcolor, 4 * sizeof (unsigned char));

    // 1,0 Texture Coord, maxx miny
    texlst[(i * 4) + 3].u = adv * (f->x + f->Width);
    texlst[(i * 4) + 3].v = adv * f->y;
    texlst[(i * 4) + 3].x = fscale * x2;
    texlst[(i * 4) + 3].y = fscale * y2;
    memcpy (texlst[(i * 4) + 3].color, &fcolor, 4 * sizeof (unsigned char));

    // Only check kerning if there is greater then 1 character and
    // if the check character is 1 less then the end of the string.
    if (n > 1) {
      x += GetKerningPair (text[i], text[i + 1]);
    }

    x += f->XAdvance;
  }
  engine::graph->flat_render (ftexid, texlst, strlen (text));
}

void bmfont::PrintCenter (float y, const char *string) {
  int x = 0;
  CharDescriptor *f;
  for (size_t i = 0, n = strlen (string); i < n; i++) {
    f = &Chars[string[i]];
    if (n > 1) {
      x += GetKerningPair (string[i], string[i + 1]);
    }
    x += f->XAdvance;
  }

  Print ((engine::graph->getWidth () / 2.f) - (x / 2), y, string);
}
bmfont::bmfont(const char *fontfile) : fcolor (0xffffffff), ftexid (nullptr), fscale (1.0), fblend (0) {
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