#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <stdexcept>

#include "stb_image.hpp"

int main(int argc, char* argv[]) {
	std::cout << "Converting ... ";
  try {
  	if (argc < 3) throw  std::runtime_error("Input empty");
    if (!argv[1] || !argv[1][0] || !argv[2] || !argv[2][0])
      throw  std::runtime_error("Input empty");
		std::cout << argv[1];
    int dat[3];
    unsigned char *inpBuffer = stbi::load(argv[1], &dat[0], &dat[1], &dat[2], stbi::channel::rgb_alpha);
    if (!inpBuffer) throw std::runtime_error(stbi::failure_reason());
  	if (dat[2] != stbi::channel::rgb_alpha) throw std::runtime_error("failed convert channels");
  	
  	std::ofstream ofile(argv[2], std::ios::binary | std::ios::out | std::ios::trunc);
    if (ofile.is_open()) {
	    ofile.write((char*)dat, sizeof(int)*2);
	    ofile.write((char*)inpBuffer, dat[0]*dat[1]*dat[2]);
	    ofile.close();
    } else throw  std::runtime_error("Could not open file input/output.");
    stbi::image_free(inpBuffer);
    std::cout << argv[2] << " completed.";
  } catch (std::exception err) {
    std::cout << " Error: " << err.what();
    std::cout << std::endl;
    //return EXIT_FAILURE;
  }
  std::cout << std::endl;
  return EXIT_SUCCESS;
}
