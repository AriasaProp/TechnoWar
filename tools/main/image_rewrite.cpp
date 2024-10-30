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
  unsigned char *img_src;
  int dih[3];
  for (const fs::directory_entry &image : fs::directory_iterator (image_path)) {
    if (!fs::is_regular_file (image.status ())) continue;
    std::string image_path = image.path ().string ();
    if (!(
            image_path.ends_with (".9.png") ||
            image_path.ends_with (".jpeg") ||
            image_path.ends_with (".jpg") ||
            image_path.ends_with (".png"))) continue;
    img_src = stbi::load::load_from_filename (image_path.c_str (), dih, dih + 1, dih + 2, stbi::load::channel::rgb_alpha);
    if (!img_src) throw stbi::load::failure_reason ();
    /*
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