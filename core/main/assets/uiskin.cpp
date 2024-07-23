#include "uiskin.hpp"

#include <cstdio>
#include <memory>

// check file state
static inline bool readFile (FILE *file, char *buffer, size_t size) {
  for (size_t bytesRead = 0, result; bytesRead < size;) {
    result = fread (buffer + bytesRead, 1, size - bytesRead, file);
    if (result > 0) {
      bytesRead += result;
    } else {
      if (feof (file)) {
        return false;
      }
      if (ferror (file)) {
        clearerr (file); // Menghapus kesalahan dan mencoba lagi
      }
    }
  }
  return true;
}

// constructor
uiskin::uiskin (const char *f) {
  filename = std::string (f);
  uiskns.insert (filename);
  FILE *atlas_pack = fopen (filename, "rb");
  if (!atlas_pack) return;
  try {

    // read regions
    for (char reading = 0; (reading = std::getc (atlas_pack)) != '\$';) {
      if (reading == '\n') continue; // skip char
      if (reading != '\"') throw "file invalid!";
      uiskin::region reg;
      // get id
      while ((reading = std::getc (atlas_pack)) != '\"') {
        reg.id += reading;
      }
      if ((reading = std::getc (atlas_pack)) != ':') throw "file invalid!";

      if (!readFile (atlas_pack, (char *)(&reg.x), 16)) // 16 bytes -> 4 * 32 bit
        throw "file invalid";
    }

  } catch (...) {
  }
  fclose (atlas_pack);
}

// destructor
uiskin::~uiskin () {
  uiskns.erase (filename);
}

size_t uiskin::region::hash::operator() (const uiskin::region &r) const {
  return std::hash<std::string> () (r.id);
}
size_t uiskin::region::hash::operator() (const char *r) const {
  return std::hash<std::string> () (std::string (r));
}
