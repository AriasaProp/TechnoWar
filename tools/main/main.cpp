#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>

char buffer[4096];
int main(int argc, char** argv) {
  try {
    std::string inputFileName;
    std::string outputFileName;
    
    std::string a, b;
    for (int i = 1; i < (argc - 1); ++i) {
      a = argv[i];
      if (a == "-i") {
        inputFileName = argv[++i];
      } else if (a == "-o") {
        outputFileName = argv[++i];
      } else {
        throw "Unknow arguments of " + a;
      }
    }
    if (inputFileName.empty() || outputFileName.empty())
      throw "Usage: " + std::string(argv[0]) + " -i <input file> -o <output file>";
  
    std::ifstream inputFile(inputFileName, std::ios::binary); // Open input file in binary mode
    if (!inputFile.is_open())
      throw "Could not open input file.";
  
    std::ofstream outputFile(outputFileName, std::ios::binary | std::ios::out | std::ios::trunc); // Open output file in binary mode
    if (!outputFile.is_open())
      throw "Could not open output file.";
    
    while (!inputFile.eof()) {
      inputFile.read(buffer, sizeof(buffer));
      outputFile.write(buffer, inputFile.gcount());
    }
  
    inputFile.close();
    outputFile.close();
  
    std::cout << "File conversion complete." << std::endl;
  } catch (std::string err) {
    std::cout << "Error: " << err << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cout << "Something error" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
