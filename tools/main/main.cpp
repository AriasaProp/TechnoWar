#include "stb_image.hpp"
#include "stb_image_write.hpp"
#include "stb_rect_pack.hpp"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

int main (int argc, char *argv[]) {
  try {
    std::cout << "Converting ... " << std::endl;
    if (argc < 3) throw "Input empty";
    if (!argv[1] || !argv[1][0] || !argv[2] || !argv[2][0])
      throw "Input empty";
    std::cout << "Input file: " << argv[1] << std::endl;
    int dat[3];
    unsigned char *inpBuffer = stbi::load (argv[1], &dat[0], &dat[1], &dat[2], stbi::channel::rgb_alpha);
    if (!inpBuffer)
      throw "Input file error";
    if (dat[2] != stbi::channel::rgb_alpha) {
      std::cout << "Channel out is: ";
      switch (dat[2]) {
      default:
      case stbi::channel::none:
        std::cout << "None?";
        break;
      case stbi::channel::grey:
        std::cout << "Grey";
        break;
      case stbi::channel::grey_alpha:
        std::cout << "Grey_Alpha";
        break;
      case stbi::channel::rgb:
        std::cout << "RGB";
        break;
      case stbi::channel::rgb_alpha:
        std::cout << "Huh?";
        break;
      }
      std::cout << std::endl;
    }
    std::cout << "Output file: " << argv[2] << std::endl;
    stbi::write::png (argv[2], dat[0], dat[1], dat[2], (void *)inpBuffer, 0);
    stbi::image_free (inpBuffer);
    std::cout << argv[2] << " completed." << std::endl;
    return EXIT_SUCCESS;
  } catch (const char *err) {
    std::cout << " Error: " << err << std::endl;
    std::cout << " STBI Error: " << stbi::failure_reason () << std::endl;
    return EXIT_FAILURE;
  }
}
