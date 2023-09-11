#include <fstream>
#include <iostream>
#include <string>

int main (int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "Usage: converter <input_file> <output_file>" << std::endl;
    return 1;
  }

  std::string input_file_name = argv[1];
  std::string output_file_name = argv[2];

  std::ifstream input_file (input_file_name);
  if (!input_file.is_open ()) {
    std::cout << "Error: Failed to open input file " << input_file_name << std::endl;
    return 1;
  }

  std::ofstream output_file (output_file_name);
  if (!output_file.is_open ()) {
    std::cout << "Error: Failed to open output file " << output_file_name << std::endl;
    return 1;
  }

  // Loop through input file line by line and write each line to output file
  std::string line;
  while (getline (input_file, line)) {
    // Perform conversion operation here (e.g. convert from uppercase to lowercase)
    output_file << line << std::endl;
  }

  // Close input and output files
  input_file.close ();
  output_file.close ();

  std::cout << "File converted successfully!" << std::endl;
  return 0;
}
