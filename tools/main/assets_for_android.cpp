#include <filesystem>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

static std::stringstream serr;

extern void uiskin_packer (fs::path, fs::path);

void assets_for_android (fs::path des_path, fs::path res_path) {
  if (fs::exists (des_path)) fs::remove_all (des_path);
  if (!fs::create_directory (des_path)) [[unlikely]] {
    serr.str ("");
    serr << "failed create assets folder in android";
    throw serr.str ();
  }

  uiskin_packer (res_path, des_path);

  fs::copy (res_path / "fonts", des_path / "fonts", fs::copy_options::recursive);
  fs::copy (res_path / "images", des_path / "images", fs::copy_options::recursive);
}
