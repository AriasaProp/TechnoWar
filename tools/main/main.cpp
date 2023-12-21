#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

char buffer[4096];
int main(int argc, char** argv) {
  try {
    std::string inputFileName;
    std::string outputFileName;
    
    for (int i = 1; i < (argc - 1); i += 2) {
      std::string a = std::string(argv[i]);
      if (a == "-i") {
        inputFileName = std::string(argv[i + 1]);
      } else if (a == "-o") {
        outputFileName = std::string(argv[i + 1]);
      } else {
        throw std::runtime_error("Unknow arguments of " + a);
      }
    }
    
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
