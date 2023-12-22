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
    std::string ifn = std::string(argv[1]);
    std::string ofn = std::string(argv[2]);
    /*
    if (inputFileName.empty() || outputFileName.empty())
      throw  std::runtime_error("Input empty");
    std::ifstream inputFile(inputFileName, std::ios::binary);
    if (!inputFile.is_open())
      throw  std::runtime_error("Could not open input file.");
  	std::ofstream outputFile(outputFileName, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!outputFile.is_open())
      throw  std::runtime_error("Could not open output file.");
    while (!inputFile.eof()) {
      inputFile.read(buffer, sizeof(buffer));
      outputFile.write(buffer, inputFile.gcount());
    }
  
    inputFile.close();
    outputFile.close();
  	*/
    std::cout << "in " << ifn << " & out " << ofn << std::endl;
    std::cout << "File conversion complete." << std::endl;
    return EXIT_FAILURE;
  } catch (std::exception err) {
    std::cout << "Error: " << err.what() << std::endl;
    //return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
