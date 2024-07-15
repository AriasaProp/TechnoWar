#include "uiskin.hpp"

#include <cstdint>
#include <cstdio>
#include <memory>
// constructor
uiskin::uiskin () {
}
// destructor
uiskin::~uiskin () {}

// check file state
static bool readFile (FILE *file, char *buffer, size_t size) {
  for (size_t bytesRead = 0, result; bytesRead < size;) {
    result = fread (buffer + bytesRead, 1, size - bytesRead, file);
    if (result > 0) {
      bytesRead += result;
    } else {
      if (feof (file)) {
        std::cerr << "End of file reached.\n";
        return false;
      }
      if (ferror (file)) {
        std::cerr << "Error reading file.\n";
        clearerr (file); // Menghapus kesalahan dan mencoba lagi
      }
    }
  }
  return true;
}

static uiskin uiskin::read_from_filename (const char *filename) {
  try {
    FILE *atlas_pack = fopen (filename, "rb");

    char *buffer = (char *)malloc (64);
    readFile (buffer, buffer, 8); // 64 bit -> 8 bytes
    {
        uint64_t

    }

    free (buffer);

    fclose (atlas_pack);
  } catch (const char *err) {
  }
}
