//#include "qoi/qoi.hpp"
#include "stbi/stbi_load.hpp"
#include "stbi/stbi_rectpack.hpp"
#include "stbi/stbi_write.hpp"
#include "utils/uistage.hpp"
#include "utils/value.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

void uiskin_packer (fs::path assets, fs::path converted) {
  fs::path uiskin_path = assets / "uiskin";
  if (!fs::exists (uiskin_path) || !fs::is_directory (uiskin_path))
    throw std::string ("uiskin folder not exist!");

  // create converted directory for uiskin result
  fs::path uiskin_result_path = converted / "uiskin";
  if (fs::exists (uiskin_result_path) || !fs::create_directory (uiskin_result_path))
    throw std::string ("error make converted dir");
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
        throw std::string (stbi::load::failure_reason ());
      image_rects.push_back ({(unsigned int)dih[0], (unsigned int)dih[1], image_path, 0, 0, 0});
    }

    // packing
    if (!stbi::rectpack::pack_rects (PACK_SIZE, PACK_SIZE, image_rects.data (), image_rects.size ()))
      throw std::string ("cannot pack image for size PACK_SIZE x PACK_SIZE!");
    // write packed result
    fs::path outfile = uiskin_part_path / skin.path ().filename ();
    fs::path outfile_txt = outfile;
    outfile_txt += ".txt";
    std::ofstream atlas_out (outfile_txt.c_str (), std::ios::out | std::ios::trunc);
    if (!atlas_out.is_open ()) throw std::string ("fail stream atlas text");
    uint32_t *outBuffer = new uint32_t[PACK_SIZE * PACK_SIZE];
    for (const stbi::rectpack::rect &r : image_rects) {
      unsigned char *image_buffer = stbi::load::load_from_filename (r.id.c_str (), dih, dih + 1, dih + 2, stbi::load::channel::rgb_alpha);
      if (!image_buffer) throw std::string (stbi::load::failure_reason ());
      for (size_t y = 0; y < r.h; y++) {
        memcpy ((void *)(outBuffer + (r.y + y) * PACK_SIZE + r.x), (void *)(image_buffer + y * r.w * 4), r.w * 4);
      }
      stbi::load::image_free (image_buffer);

      {
        size_t lastSlashPos = r.id.find_last_of ("/\\");
        lastSlashPos = (lastSlashPos == std::string::npos) ? 0 : lastSlashPos + 1;

        size_t lastDotPos = r.id.find_last_of ('.');
        if (lastDotPos == std::string::npos || lastDotPos < lastSlashPos) {
          lastDotPos = r.id.length ();
        }

        atlas_out << r.id.substr (lastSlashPos, lastDotPos - lastSlashPos);
      }
      atlas_out << ":" << r.x << " " << r.y << " " << r.w << " " << r.h << std::endl;
    }
    atlas_out.close ();
    fs::path outfile_png = outfile;
    outfile_png += ".png";
    stbi::write::png (outfile_png.c_str (), PACK_SIZE, PACK_SIZE, 4, outBuffer, 0);
    delete[] outBuffer;
  }
}