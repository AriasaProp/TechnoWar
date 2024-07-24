#include "uiskin.hpp"
#include "../engine.hpp"

#include <cstdio>
#include <memory>

// constructor
uiskin::uiskin (const char *f) {
  engine::asset_core *ast = engine::assets->open_asset (f);
  try {

    // read regions
    char reading;
    while (!ast->eof ()) {
      if (ast->read ((void *)&reading, 1)) continue;
      if (reading == '\n') continue; // skip char
      if (reading != '\"') throw "file invalid!";
      uiskin::region reg;
      // get id
      while (ast->read ((void *)&reading, 1) && (reading != '\"')) {
        reg.id += reading;
      }
      if ((ast->read((void*)&reading, 1) && (reading != ':')) throw "file invalid!";

      if (ast->read((void *)(&reg.x), 16) != 16) // 16 bytes -> 4 * 32 bit
        throw "file invalid";
    }

  } catch (...) {
  }
  delete ast;
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
