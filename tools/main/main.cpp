#include <iostream>
#include <fstream>
#include <string>

char buffer[2048];
int main(int argc, char** argv) {
  try {
    std::string inputFileName;
    std::string outputFileName;
    
    char **args = argv;
    char *name = args;
    while (args) {
      if (i + 1 < argc) {
        std::string a(argv[i]);
        if (a == "-i") {
          inputFileName = argv[i + 1];
        } else if (a == "-o") {
          outputFileName = argv[i + 1];
        } else {
          std::cout << "Unknow arguments of " << a << std::endl;
        }
      }
      args++;
    }
  
    if (inputFileName.empty() || outputFileName.empty()) {
      std::cerr << "Usage: " << argv[0] << " -i <input file> -o <output file>" << std::endl;
      return 1;
    }
  
    std::ifstream inputFile(inputFileName, std::ios::binary); // Open input file in binary mode
    if (!inputFile.is_open()) {
      std::cerr << "Error: Could not open input file." << std::endl;
      return 1;
    }
  
    std::ofstream outputFile(outputFileName, std::ios::binary | std::ios::out | std::ios::trunc); // Open output file in binary mode
    if (!outputFile.is_open()) {
      std::cerr << "Error: Could not open output file." << std::endl;
      return 1;
    }
    while (!inputFile.eof()) {
      inputFile.read(buffer, sizeof(buffer));
      outputFile.write(buffer, inputFile.gcount());
    }
  
    inputFile.close();
    outputFile.close();
  
    std::cout << "File conversion complete." << std::endl;
    
  } catch (const char *err) {
    std::cout << "Error: " << err << std::endl;
  }

  return 0;
}
