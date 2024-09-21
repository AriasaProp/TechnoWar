#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

//test group
extern void stbi_rectpack_test ();

//converting group
extern void uiskin_packing (fs::path, fs::path);
extern void copy_others (fs::path, fs::path);
extern void android_asset_match (fs::path, fs::path);

int main (int argc, char *argv[]) {
  std::cout << "Test Tools";
  try {
  	stbi_rectpack_test ();
  } catch (const char *err) {
  	std::cout << "\nError -> " << err << std::endl;
  	return EXIT_FAILURE;
  }
	std::cout << " Done!" << std::endl;
	
  std::cout << "Converting Assets";
  try {
  	try {
	    // path of source files
	    fs::path projectDir = argv[1];
	    fs::path assets = projectDir / "assets";
	    if (!fs::exists (assets) || !fs::is_directory (assets)) throw "assets folder didn't exist!";
	    // create converted directory for result
	    fs::path converted = projectDir /  "converted/assets";
	    if (!fs::create_directory (converted)) throw "cannot make a directory";
	    uiskin_packing (assets, converted);
	    copy_others (assets, converted);
	    android_asset_match (converted, projectDir/"../src/main");
		} catch (const fs::filesystem_error &e) {
			std::string thr_er = "(Filesystem) ";
			thr_er += e.what();
			throw thr_er.c_str();
		}
  } catch (const char *err) {
    std::cout << "\n Error " << err << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << " Done!" << std::endl;
  return EXIT_SUCCESS;
}

