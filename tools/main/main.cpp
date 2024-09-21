#include <iostream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

//converting group
extern void assets_for_android(fs::path, fs::path);
extern void assets_for_desktop(fs::path, fs::path);

int main (int argc, char *argv[]) {
  std::cout << "Converting Assets";
  try {
  	try {
	    // path of source files
	    fs::path rootDir = argv[1];
	    fs::path projectDir = argv[2];
	    fs::path assets = projectDir / "assets";
	    if (!fs::exists (assets) || !fs::is_directory (assets)) throw "assets folder didn't exist!";
	    
	    assets_for_android(rootDir/"android/src/main/assets", assets);
	    assets_for_desktop(rootDir/"desktop/assets", assets);
	    
		} catch (const fs::filesystem_error &e) {
			std::string thr_er = "(Filesystem) " + e.what();
			throw thr_er.c_str();
		}
  } catch (const char *err) {
  	std::cout << " Error!\n" << err << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << " Done!" << std::endl;
  return EXIT_SUCCESS;
}

