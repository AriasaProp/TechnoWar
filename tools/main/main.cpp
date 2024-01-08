#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

#include "stb_image.hpp"

int main(int argc, char* argv[]) {
  try {
  	if (argc < 3) throw  std::runtime_error("Input empty");
    if (!argv[1] || !argv[1][0] || !argv[2] || !argv[2][0])
      throw  std::runtime_error("Input empty");
    int x, y, comp;
    unsigned char *inpBuffer = stbi::load_from_file(argv[1], &x, &y, &comp, stbi::channel::rgb_alpha);
    if (!inpBuffer) throw std::runtime_error(stbi::failure_reason());
  	if (comp != stbi::channel::rgb_alpha) throw std::runtime_error("failed convert channels");
  	
  	std::ofstream ofile{argv[2], std::ios::binary | std::ios::out | std::ios::trunc};
    if (ofile.is_open()) {
	    if () {
	    	int dat[2] {x, y};
		    ofile.write((char*)dat, sizeof(dat));
		    ofile.write((char*)inpBuffer, x*y*comp);
		    stbi::image_free(inpBuffer);
	    }
	    ofile.close();
    } else throw  std::runtime_error("Could not open file input/output.");
    std::cout << "File conversion complete." << std::endl;
  } catch (std::exception err) {
    std::cout << "Error: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
