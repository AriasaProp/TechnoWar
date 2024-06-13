#include "stb_image.hpp"
#include "stb_image_write.hpp"
#include "stb_rect_pack.hpp"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

int main (int argc, char *argv[]) {
  try {
    std::cout << "Converting ... ";
    if (argc < 3) throw std::runtime_error ("Input empty");
    if (!argv[1] || !argv[1][0] || !argv[2] || !argv[2][0])
      throw std::runtime_error ("Input empty");
    std::cout << argv[1];
    int dat[3];
    unsigned char *inpBuffer = stbi::load (argv[1], &dat[0], &dat[1], &dat[2], stbi::channel::rgb_alpha);
    if (!inpBuffer) throw std::runtime_error (stbi::failure_reason ());
    if (dat[2] != stbi::channel::rgb_alpha) throw std::runtime_error ("failed convert channels");

    stbi::write::png (argv[2], dat[0], dat[1], dat[2], (void *)inpBuffer, 0);
    stbi::image_free (inpBuffer);
    std::cout << argv[2] << " completed.";
    std::cout << std::endl;
    return EXIT_SUCCESS;
  } catch (std::exception err) {
    std::cout << " Error: " << err.what ();
    std::cout << std::endl;
    return EXIT_FAILURE;
  }
  
}
