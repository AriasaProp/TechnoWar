#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

char buffer[2048];
int main(int argc, char** argv) {
  try {
  	if (argc < 3) throw  std::runtime_error("Input empty");
    const std::string ifn = std::string(argv[1]);
    const std::string ofn = std::string(argv[2]);
    if (ifn.empty() || ofn.empty())
      throw  std::runtime_error("Input empty");
    std::ifstream ifile(ifn, std::ios::binary);
  	std::ofstream ofile(ofn, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!ifile.is_open())
      throw  std::runtime_error("Could not open input file.");
    if (!ofile.is_open())
      throw  std::runtime_error("Could not open output file.");
    while (!ifile.eof()) {
      ifile.read(buffer, sizeof(buffer));
      ofile.write(buffer, ifile.gcount());
    }
    ifile.close();
    ofile.close();
    std::cout << "in " << ifn << " & out " << ofn << std::endl;
    std::cout << "File conversion complete." << std::endl;
  } catch (std::exception err) {
    std::cout << "Error: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
