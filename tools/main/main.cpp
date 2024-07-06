#include "stbi_load.hpp"
#include "stbi_rectpack.hpp"
#include "stbi_write.hpp"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

int main (int argc, char *argv[]) {
  struct stat file_folder_info;
  try {
    std::cout << "Converting Assets" << std::endl;
    // path of source files
    fs::path assets = "assets";
    if (!fs::exists (assets)) throw "assets didn't exist!";
    // create converted directory for result
    fs::path converted = "converted";
    fs::create_directory (converted);
    // packing uiskins
    try {
      std::cout << "Converting UISkin" << std::endl;
      fs::path uiskin_path = assets / "uiskin";

      if (!fs::exists (uiskin_path) || !fs::is_directory (uiskin_path))
        throw "UISkins path isn't right!";

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
        // safe total area and made square with that size
        unsigned int rectpacked_size = 0;

        // as temporary data receive as data image holder (dih)
        int dih[3];

        // get all image files inside uiskin sub folder
        for (const fs::directory_entry &image : fs::directory_iterator (skin.path ())) {
          if (!fs::is_regular_file (image.status ())) continue;
          std::string image_path = image.path ().string ();
          if (!(image_path.ends_with (".9.png") || image_path.ends_with (".png"))) continue;
          if (!stbi::load::info (image_path.c_str (), dih, dih + 1, dih + 2)) continue;
          image_rects.push_back ({(unsigned int)dih[0], (unsigned int)dih[1], image_path, 0, 0, 0});
          rectpacked_size += dih[0] * dih[1];
        }

        // square root to get dimension of rect packer
        {
          // logically should bigger than 1
          assert (rectpacked_size > 1);
          const double n = static_cast<double> (rectpacked_size) * 1.15;
          double x;
          double root = n;
          do {
            x = root;
            root = 0.5 * (x + n / x);
          } while (std::abs (root - x) > 0.0);
          root = std::ceil (root);
          rectpacked_size = static_cast<unsigned int> (root) + 5;
        }
        // packing
        if (!stbi::rectpack::pack_rects (rectpacked_size, rectpacked_size, image_rects.data (), image_rects.size ()))
          std::cout << "Warning: All not packed!" << std::endl;
        // write packed result
        std::ofstream atlas_map((uiskin_part_path/"map.txt").c_str());
        atlas_map << "size " << rectpacked_size << " , " << rectpacked_size << std::endl;
        uint32_t outBuffer[rectpacked_size * rectpacked_size] = {0};
        for (const stbi::rectpack::rect &r : image_rects) {
          if (!r.was_packed) continue;
          unsigned char *image_buffer = stbi::load::load_from_filename (r.id.c_str(), dih, dih + 1, dih + 2, stbi::load::channel::rgb_alpha);
          if (!image_buffer) continue;
          for (size_t y = 0; y < r.h; y++) {
            memcpy ((void *)(outBuffer + ((r.y + y) * rectpacked_size) + r.x), (void *)(image_buffer + (y * r.w * 4)), r.w * 4);
          }
          stbi::load::image_free (image_buffer);
          atlas_map << r.id << ": " << r.x << " , " << r.y << " , " << r.w << " , " << r.h << std::endl;
        }
        atlas_map.close ();

        // create output directory skin name
        fs::path outfile = uiskin_part_path / "pack.png";
        stbi::write::png (outfile.c_str (), rectpacked_size, rectpacked_size, stbi::load::channel::rgb_alpha, (void *)outBuffer, 0);
        std::cout << "Output: " << outfile.c_str () << " completed." << std::endl;
      }
    } catch (const fs::filesystem_error &e) {
      std::cerr << "Error filesystem: " << e.what () << std::endl;
    } catch (const char *err) {
      std::cerr << "UISkin Conversion Error: " << err << std::endl;
    }
  } catch (const fs::filesystem_error &e) {
    std::cerr << "Error file: " << e.what () << std::endl;
    return EXIT_FAILURE;
  } catch (const char *err) {
    std::cout << " Error: " << err << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
