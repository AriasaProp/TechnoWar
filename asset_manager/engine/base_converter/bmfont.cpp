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

bool bmfont::ParseFont (const char *fontfile) {
  unsigned int asl;
  const char *as = (const char *)engine::asset->asset_buffer (fontfile, &asl);
  std::string buffer (as, asl);
  std::stringstream buffer_stream (buffer);
  std::string Line;
  std::string Read, Key, Value;
  std::size_t i;

  KearningInfo K;
  CharDescriptor C;

  while (!buffer_stream.eof ()) {
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
    } else */
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

bmfont::bmfont (const char *fontfile) : fcolor (0xffffffff), ftexid (nullptr), fscale (3.f) {
  int x, y;
  unsigned int datRI;
  ParseFont (fontfile);
  char *texfile = new char[strlen (fontfile)];
  memcpy (texfile, fontfile, strlen (fontfile));
  memcpy (strstr (texfile, ".fnt"), ".png", 4);
  unsigned char *tD = stbi_load (texfile, &x, &y, nullptr, STBI_rgb_alpha);
  delete[] texfile;
  stbi_image_free (tD);
}

bmfont::~bmfont () {
  Chars.clear ();
  Kearn.clear ();
}