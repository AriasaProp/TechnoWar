#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

char buffer[4096];
int main(int argc, char** argv) {
  try {
  	if (argc < 3) throw  std::runtime_error("Input empty");
    std::string inputFileName = std::string(argv[1]);
    std::string outputFileName = std::string(argv[2]);
    
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
  
    std::cout << "File conversion complete." << std::endl;
  } catch (std::exception err) {
    std::cout << "Error: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
