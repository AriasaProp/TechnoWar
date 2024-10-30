#include "stbi/stbi_load.hpp"
#include "stbi/stbi_rectpack.hpp"
#include "stbi/stbi_write.hpp"
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

void image_rewrite (fs::path assets, fs::path converted) {
  fs::path image_path = assets / "images";
  if (!fs::exists (image_path) || !fs::is_directory (image_path))
    throw "images folder not exist!";

  // create converted directory for image result
  fs::path image_result_path = converted / "images";
  if (fs::exists (image_result_path) || !fs::create_directory (image_result_path))
    throw "error make converted dir";
  // find all subfolder inside image
  int dih[3];
  unsigned char *img_src;
  for (const fs::directory_entry &image : fs::directory_iterator (image_path)) {
    if (!fs::is_regular_file (image.status ())) continue;
    fs::path image_extension = image.path ().extension ();
    if (image_extension.compare (".9.png") ||
        image_extension.compare (".jpeg") ||
        image_extension.compare (".jpg") ||
        image_extension.compare (".png")) continue;
    img_src = stbi::load::load_from_filename (image.path ().c_str (), dih, dih + 1, dih + 2, stbi::load::channel::rgb_alpha);
    /*
    std::string image_filename = image.path ().filename().string ();
      if (!img_src) throw stbi::load::failure_reason ();
      {
        size_t lastSlashPos = image_path.find_last_of ("/\\");
        lastSlashPos = (lastSlashPos == std::string::npos) ? 0 : lastSlashPos + 1;
        size_t lastDotPos = image_path.find_last_of ('.');
        if (lastDotPos == std::string::npos || lastDotPos < lastSlashPos) {
          lastDotPos = image_path.length ();
        }
        image_path = image_path.substr (lastSlashPos, lastDotPos - lastSlashPos);
        fs::path res = image_result_path / image_path;
        res += ".png";
        stbi::write::png (res.c_str (), dih[0], dih[1], 4, img_src, 4);
      }
    */
    stbi::load::image_free (img_src);
  }
}