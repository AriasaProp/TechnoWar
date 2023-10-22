#include <iostream>
#include <fstream>
#include <string>

char buffer[2048];
int main(int, char** argv) {
  try {
    std::string inputFileName;
    std::string outputFileName;
    
    char **args = argv;
    std::string name = *args;
    std::string a, b;
    while (!(a = *(++args)).empty() && !(b = *(++args)).empty()) {
      if (a == "-i") {
        inputFileName = b;
      } else if (a == "-o") {
        outputFileName = b;
      } else {
        throw "Unknow arguments of " + a;
      }
    }
  
    if (inputFileName.empty() || outputFileName.empty())
      throw "Usage: " + name + " -i <input file> -o <output file>";
  
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
    
    return 0;
  } catch (std::string err) {
    std::cout << "Error: " << err << std::endl;
    return 1;
  }
}
