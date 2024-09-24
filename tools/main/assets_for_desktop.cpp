#include <filesystem>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

static std::stringstream serr;

void extern uiskin_packer (fs::path, fs::path);

void assets_for_desktop (fs::path des_path, fs::path res_path) {
  try {
    try {
      if (fs::exists (des_path)) fs::remove_all (des_path);
      if (!fs::create_directory (des_path)) {
        serr.str ("");
        serr << "failed create assets folder in desktop";
        throw serr.str ();
      }

      fs::copy (res_path / "fonts", des_path / "fonts", fs::copy_options::recursive);
      fs::copy (res_path / "images", des_path / "images", fs::copy_options::recursive);
      uiskin_packer (res_path, des_path);
    } catch (const fs::filesystem_error &e) {
      serr.str ("");
      serr << "(Filesystem) " << e.what ();
      throw serr.str ();
    }
  } catch (const std::string err) {
    serr.str ("");
    serr << "Desktop Assets Generator " << err;
    throw serr.str ();
  }
}
