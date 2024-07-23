#include "uiskin.hpp"
#include "../engine.hpp"

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
	engine::asset_core *ast = engine::assets->open_asset(f);
  try {

    // read regions
    char reading;
    while (!ast->eof()) {
    	if (ast->read((void*)&reading, 1)) continue;
      if (reading == '\n') continue; // skip char
      if (reading != '\"') throw "file invalid!";
      uiskin::region reg;
      // get id
      while ((ast->read((void*)&reading, 1) && (reading != '\"')) {
        reg.id += reading;
      }
      while ((ast->read((void*)&reading, 1) && (reading != ':')) throw "file invalid!";

      if (!ast->read((void *)(&reg.x), 16) != 16) // 16 bytes -> 4 * 32 bit
        throw "file invalid";
    }

  } catch (...) {
  }
  fclose (atlas_pack);
}

// destructor
uiskin::~uiskin () {
}

size_t uiskin::region::hash::operator() (const uiskin::region &r) const {
  return std::hash<std::string> () (r.id);
}
size_t uiskin::region::hash::operator() (const char *r) const {
  return std::hash<std::string> () (std::string (r));
}


