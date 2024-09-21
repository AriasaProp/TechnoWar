#include <filesystem>

namespace fs = std::filesystem;

void copy_others(fs::path assets, fs::converted) {
	
  fs::copy(assets/"fonts", converted/"fonts", fs::copy_options::recursive);
  fs::copy(assets/"images", converted/"images", fs::copy_options::recursive);
}