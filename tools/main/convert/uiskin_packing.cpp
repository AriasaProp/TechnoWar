#include "assets/uiskin.hpp"
#include "qoi/qoi.hpp"
#include "stbi/stbi_load.hpp"
#include "stbi/stbi_rectpack.hpp"
#include "stbi/stbi_write.hpp"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

void uiskin_packing (fs::path assets, fs::path converted) {
  fs::path uiskin_path = assets / "uiskin";
  if (!fs::exists (uiskin_path) || !fs::is_directory (uiskin_path))
    throw "uiskin folder not exist!";

  // create converted directory for uiskin result
  fs::path uiskin_result_path = converted / "uiskin";
  fs::create_directory (uiskin_result_path);

  // find all subfolder inside uiskin
  for (const fs::directory_entry &skin : fs::directory_iterator (uiskin_path)) {
    // skip non directory
    if (!fs::is_directory (skin.status ())) continue;
    // make part skin
    fs::path uiskin_part_path = uiskin_result_path / skin.path ().filename ();
    fs::create_directory (uiskin_part_path);

    // bin rects for hold rects
    std::vector<stbi::rectpack::rect> image_rects;

    // as temporary data receive as data image holder (dih)
    int dih[3];

    // get all image files inside uiskin sub folder
    for (const fs::directory_entry &image : fs::directory_iterator (skin.path ())) {
      if (!fs::is_regular_file (image.status ())) continue;
      std::string image_path = image.path ().string ();
      if (!(image_path.ends_with (".9.png") || image_path.ends_with (".png"))) continue;
      if (!stbi::load::info (image_path.c_str (), dih, dih + 1, dih + 2))
      	throw stbi::load::failure_reason ();
      image_rects.push_back ({(unsigned int)dih[0], (unsigned int)dih[1], image_path, 0, 0, 0});
    }

    // packing
    if (!stbi::rectpack::pack_rects (PACK_SIZE, PACK_SIZE, image_rects.data (), image_rects.size ()))
      throw "cannot pack image for size PACK_SIZE x PACK_SIZE!";
    // write packed result
    fs::path outfile = uiskin_part_path / skin.path ().filename ();
    outfile += ".pack";
    FILE *atlas_out = fopen (outfile.c_str (), "wb");
    if (atlas_out == NULL) {
    	throw strerror (errno);
    }
    uint32_t outBuffer[PACK_SIZE * PACK_SIZE] = {0};
    for (const stbi::rectpack::rect &r : image_rects) {
      unsigned char *image_buffer = stbi::load::load_from_filename (r.id.c_str (), dih, dih + 1, dih + 2, stbi::load::channel::rgb_alpha);
      if (!image_buffer)
      	throw stbi::load::failure_reason ();

      for (size_t y = 0; y < r.h; y++) {
        memcpy ((void *)(outBuffer + ((r.y + y) * PACK_SIZE) + r.x), (void *)(image_buffer + (y * r.w * 4)), r.w * 4);
      }
      stbi::load::image_free (image_buffer);
      std::string name = r.id;
      {
        size_t lastSlashPos = name.find_last_of ("/\\");
        lastSlashPos = (lastSlashPos == std::string::npos) ? 0 : lastSlashPos + 1;

        size_t lastDotPos = name.find_last_of ('.');
        if (lastDotPos == std::string::npos || lastDotPos < lastSlashPos) {
          lastDotPos = name.length ();
        }

        name = name.substr (lastSlashPos, lastDotPos - lastSlashPos);
      }
      fwrite ("\"", sizeof (char), 1, atlas_out);
      fwrite (name.data (), sizeof (char), name.size (), atlas_out);
      fwrite ("\":", sizeof (char), 2, atlas_out);
      uint32_t tempWrite[4]{r.x, r.y, r.w, r.h};
      fwrite (tempWrite, sizeof (tempWrite), 1, atlas_out);
      fwrite ("\n", sizeof (char), 1, atlas_out);
    }
    fwrite ("$", sizeof (char), 1, atlas_out);
    // create output directory skin name
    {
      qoi_desc desc{PACK_SIZE, PACK_SIZE, 4, 1};
      int size;
      unsigned char *encoded = qoi_encode ((unsigned char *)outBuffer, &desc, &size);
      fwrite (encoded, 1, size, atlas_out);
      delete[] encoded;
    }
    fflush (atlas_out);
    if (ferror (atlas_out)) throw strerror(errno);
    fclose (atlas_out);
  }
}