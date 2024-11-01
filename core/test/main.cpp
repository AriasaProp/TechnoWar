#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

// test group
extern void stbi_rectpack_test ();

// converting group
extern void assets_for_android (fs::path, fs::path);
extern void assets_for_desktop (fs::path, fs::path);

int main (int argc, char *argv[]) {
  std::cerr << "Test Tools ";
  try {
    stbi_rectpack_test ();
  } catch (const char *err) {
    std::cerr << "Error!\n " << err << std::endl;
    return EXIT_FAILURE;
  }
  std::cerr << "Done!" << std::endl;

  std::cerr << "Converting Assets ";
  try {
    // path of source files
    fs::path rootDir = argv[1];
    fs::path projectDir = argv[2];
    fs::path assets = projectDir / "assets";
    if (!fs::exists (assets) || !fs::is_directory (assets)) throw std::string("assets folder didn't exist!");

    assets_for_android (rootDir / "android/src/main/assets", assets);
    // assets_for_desktop (rootDir / "desktop/assets", assets);

  } catch (const fs::filesystem_error &e) {
    std::cerr << "Error(Filesystem): " << e.what () << std::endl;
    return EXIT_FAILURE;
  } catch (const std::string *err) {
    std::cerr << "Error: " << err << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Error: Unknown" << std::endl;
    return EXIT_FAILURE;
  }
  std::cerr << "Done!" << std::endl;
  return EXIT_SUCCESS;
}
