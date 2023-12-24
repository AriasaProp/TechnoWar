#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

#include "stb_image.hpp"

stbi_io_callbacks sic_file {
	.read = [](void *user, char *data, unsigned int size) -> int {
		std::ifstream ifile = (std::ifstream*)user;
		ifile.read(data, size);
		return ifile.gcount();
	},
	.skip = [](void *user, int n){
		std::ifstream ifile = (std::ifstream*)user;
		ifile.seekg(n, std::ios::cur);
	},
	.eof = [](void *user) -> bool {
		return ((std::ifstream*)user)->eof();
	}
};

int main(int argc, char** argv) {
  try {
  	if (argc < 3) throw  std::runtime_error("Input empty");
    if (!argv[1] || !argv[1][0] || !argv[2] || !argv[2][0])
      throw  std::runtime_error("Input empty");
    std::cout << "in " << argv[1] << " & out " << argv[2] << std::endl;
    std::ifstream ifile{argv[1], std::ios::binary};
  	std::ofstream ofile{argv[2], std::ios::binary | std::ios::out | std::ios::trunc};
  	
    if (ifile.is_open() && ofile.is_open()) {
	    int x, y, comp;
	    unsigned char *inpBuffer = stbi_load_from_callbacks(&sic_file, (void*)&ifile, &x, &y, &comp, STBI_rgb_alpha);
	    ofile.write((char*)inpBuffer, x*y*comp);
	    stbi_image_free(inpBuffer);
	    ifile.close();
	    ofile.close();
    } else throw  std::runtime_error("Could not open file input/output.");
    std::cout << "File conversion complete." << std::endl;
  } catch (std::exception err) {
    std::cout << "Error: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
