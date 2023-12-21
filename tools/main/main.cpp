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
    
    for (int i = 1; i < (argc - 1); i += 2) {
      std::string a = std::string(argv[i]);
      if (a == "-i") {
        inputFileName = std::string(argv[i + 1]);
      } else if (a == "-o") {
        outputFileName = std::string(argv[i + 1]);
      } else {
        throw std::string("Unknow arguments of ") + a;
      }
    }
    
    if (inputFileName.empty() || outputFileName.empty())
      throw "Input empty";
  	try {
	    std::ifstream inputFile(inputFileName, std::ios::binary);
	    if (!inputFile.is_open())
	      throw "Could not open input file.";
    	std::ofstream outputFile(outputFileName, std::ios::binary | std::ios::out | std::ios::trunc);
	    if (!outputFile.is_open())
	      throw "Could not open output file.";
	    while (!inputFile.eof()) {
	      inputFile.read(buffer, sizeof(buffer));
	      outputFile.write(buffer, inputFile.gcount());
	    }
	  
	    inputFile.close();
	    outputFile.close();
  	} catch (const char *err) { throw err;
  	} catch (...) { throw "Unknow file proccess error."; }
  
    std::cout << "File conversion complete." << std::endl;
  } catch (std::string err) {
    std::cout << "Error: " << err << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
