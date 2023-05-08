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

bool BMFont::ParseFont(const char *fontfile ) {
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

int BMFont::GetKerningPair (int first, int second) {
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

float BMFont::GetStringWidth (const char *string) {
  float total = 0;
  CharDescriptor *f;
  for (size_t i = 0, n = strlen (string); i < n; i++) {
    f = &Chars[string[i]];
    total += f->XAdvance;
  }
  return total * fscale;
}

void BMFont::Print (float x, float y, const char *fmt, ...) {
  float CurX = (float)x;
  float CurY = (float)y;
  float DstX = 0.0;
  float DstY = 0.0;
  int Flen;
  float adv = (float)1.0 / Width; // Font texture atlas spacing.
  char text[512] = "";            // Holds Our String
  CharDescriptor *f;              // Pointer to font.
  va_list ap;                     // Pointer To List Of Arguments
  if (fmt == NULL)                // If There's No Text
    return;                       // Do Nothing
  va_start (ap, fmt);             // Parses The String For Variables
  vsprintf (text, fmt, ap);       // And Converts Symbols To Actual Numbers
  va_end (ap);

  y = y + LineHeight;
  Flen = strlen (text);

  for (int i = 0; i != Flen; ++i) {

    f = &Chars[text[i]];

    CurX = x + f->XOffset;
    CurY = y - f->YOffset;
    DstX = CurX + f->Width;
    DstY = CurY - f->Height;

    // 0,1 Texture Coord
    texlst[i * 4].u = adv * f->x;
    texlst[i * 4].v = (float)1.0 - (adv * (f->y + f->Height));
    texlst[i * 4].x = (float)fscale * CurX;
    texlst[i * 4].y = (float)fscale * DstY;
    memcpy (texlst[i * 4].color, &fcolor, 4 * sizeof (unsigned char));

    // 0,0 Texture Coord
    texlst[(i * 4) + 1].u = adv * f->x;
    texlst[(i * 4) + 1].v = (float)1.0 - (adv * f->y);
    texlst[(i * 4) + 1].x = (float)fscale * CurX;
    texlst[(i * 4) + 1].y = (float)fscale * CurY;
    memcpy (texlst[(i * 4) + 3].color, &fcolor, 4 * sizeof (unsigned char));

    // 1,1 Texture Coord
    texlst[(i * 4) + 2].u = adv * (f->x + f->Width);
    texlst[(i * 4) + 2].v = (float)1.0 - (adv * (f->y + f->Height));
    texlst[(i * 4) + 2].x = (float)fscale * DstX;
    texlst[(i * 4) + 2].y = (float)fscale * DstY;
    memcpy (texlst[(i * 4) + 2].color, &fcolor, 4 * sizeof (unsigned char));

    // 1,0 Texture Coord
    texlst[(i * 4) + 3].u = adv * (f->x + f->Width);
    texlst[(i * 4) + 3].v = (float)1.0 - (adv * f->y);
    texlst[(i * 4) + 3].x = (float)fscale * DstX;
    texlst[(i * 4) + 3].y = (float)fscale * CurY;
    memcpy (texlst[(i * 4) + 3].color, &fcolor, 4 * sizeof (unsigned char));

    // Only check kerning if there is greater then 1 character and
    // if the check character is 1 less then the end of the string.
    if (Flen > 1 && i < Flen) {
      x += GetKerningPair (text[i], text[i + 1]);
    }

    x += f->XAdvance;
  }
  engine::graph->flat_render (ftexid, texlst, strlen (text));
}

void BMFont::PrintCenter (float y, const char *string) {
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
BMFont::BMFont (const char *fontfile) : fcolor (0xffffffff), ftexid (nullptr), fscale (1.0), fblend (0) {
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

BMFont::~BMFont () {
  Chars.clear ();
  Kearn.clear ();
  engine::graph->delete_texture (ftexid);
}