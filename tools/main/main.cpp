#include <filesystem>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

// test group
extern void stbi_rectpack_test ();

// converting group
extern void assets_for_android (fs::path, fs::path);
extern void assets_for_desktop (fs::path, fs::path);

int main (int argc, char *argv[]) {
  std::cout << "Test Tools";
  try {
    stbi_rectpack_test ();
  } catch (const char *err) {
    std::cout << " Error!\n " << err << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << " Done!" << std::endl;

  std::cout << "Converting Assets";
  try {
    try {
      // path of source files
      fs::path rootDir = argv[1];
      fs::path projectDir = argv[2];
      fs::path assets = projectDir / "assets";
      if (!fs::exists (assets) || !fs::is_directory (assets)) throw "assets folder didn't exist!";

      assets_for_android (rootDir / "android/src/main/assets", assets);
      assets_for_desktop (rootDir / "desktop/assets", assets);

    } catch (const fs::filesystem_error &e) {
      std::string thr_er = "(Filesystem) ";
      thr_er += e.what ();
      throw thr_er.c_str ();
    }
  } catch (const char *err) {
    std::cout << " Error!\n"
              << err << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << " Done!" << std::endl;
  return EXIT_SUCCESS;
}
