#include "stbi_load.hpp"
#include "stbi_rectpack.hpp"
#include "stbi_write.hpp"

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;

int main (int argc, char *argv[]) {
  try {
    std::cout << "Converting ... " << std::endl;
    if ((argc < 3) || !argv[1] || !argv[1][0] || !argv[2] || !argv[2][0])
      throw "Input empty";
    std::cout << "Input folder: " << argv[1] << std::endl;
    static const std::set<std::string> exten = {".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif"};
    for (const auto &entry : fs::directory_iterator (argv[1])) {
      if ((!fs::is_regular_file (entry.status ())) || (exten.find (entry.path ().extension ().string ()) == exten.end ())) continue;
      int dat[3];
      stbi::load::info (entry.path ().c_str (), dat, dat + 1, dat + 2);
      std::cout << "Size: " << dat[0] << " x " << dat[1] << ", Channel: ";
      switch (dat[2]) {
      default:
      case stbi::load::channel::none:
        std::cout << "None?";
        break;
      case stbi::load::channel::grey:
        std::cout << "Grey";
        break;
      case stbi::load::channel::grey_alpha:
        std::cout << "Grey_Alpha";
        break;
      case stbi::load::channel::rgb:
        std::cout << "RGB";
        break;
      case stbi::load::channel::rgb_alpha:
        std::cout << "RGBA";
        break;
      }
      std::cout << std::endl;
    }
    /*
    unsigned char *inpBuffer = stbi::load::load_from_filename (argv[1], &dat[0], &dat[1], &dat[2], stbi::load::channel::rgb_alpha);
    if (!inpBuffer)
      throw "Input file error";
    if (dat[2] != stbi::load::channel::rgb_alpha) {
      std::cout << "Channel out is: ";
      switch (dat[2]) {
      default:
      case stbi::load::channel::none:
        std::cout << "None?";
        break;
      case stbi::load::channel::grey:
        std::cout << "Grey";
        break;
      case stbi::load::channel::grey_alpha:
        std::cout << "Grey_Alpha";
        break;
      case stbi::load::channel::rgb:
        std::cout << "RGB";
        break;
      case stbi::load::channel::rgb_alpha:
        std::cout << "Huh?";
        break;
      }
      std::cout << std::endl;
    }
    std::cout << "Output file: " << argv[2] << std::endl;
    stbi::write::png (argv[2], dat[0], dat[1], dat[2], (void *)inpBuffer, 0);
    stbi::load::image_free (inpBuffer);
    std::cout << argv[2] << " completed." << std::endl;
    */
  } catch (const fs::filesystem_error &e) {
    std::cerr << "Error reading directory: " << e.what () << std::endl;
    return EXIT_FAILURE;
  } catch (const char *err) {
    std::cout << " Error: " << err << std::endl;
    std::cout << " STBI Error: " << stbi::load::failure_reason () << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
