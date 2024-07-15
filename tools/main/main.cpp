#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

extern bool uiskin_packing(fs::path, fs::path);

int main (int argc, char *argv[]) {
  try {
    std::cout << "Converting Assets" << std::endl;
    // path of source files
    fs::path assets = "assets";
    if (!fs::exists (assets)) throw "assets didn't exist!";
    // create converted directory for result
    fs::path converted = "converted";
    if (!fs::create_directory (converted)) throw "";
    if (!uiskin_packing(assets, converted)) throw "uisking packing state was error";
    
  } catch (const fs::filesystem_error &e) {
    std::cerr << "Error file: " << e.what () << std::endl;
    return EXIT_FAILURE;
  } catch (const char *err) {
    std::cout << " Error: " << err << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
