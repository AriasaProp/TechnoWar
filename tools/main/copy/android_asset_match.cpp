#include <filesystem>

namespace fs = std::filesystem;

void android_asset_match(fs::path from, fs::path to) {
	if (fs::exists(to/"assets")) fs::remove_all(to/"assets");
  fs::copy(from, to/"assets", fs::copy_options::recursive);
}