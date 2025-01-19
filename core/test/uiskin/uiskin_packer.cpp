//#include "qoi/qoi.hpp"
#include "stbi/stbi_load.hpp"
#include "stbi/stbi_rectpack.hpp"
#include "stbi/stbi_write.hpp"
#include "utils/uistage.hpp"
#include "utils/value.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

void uiskin_packer (fs::path assets, fs::path converted) {
  fs::path uiskin_path = assets / "uiskin";
  if (!fs::exists (uiskin_path) || !fs::is_directory (uiskin_path))
    throw std::string ("uiskin folder not exist!");

  // create converted directory for uiskin result
  fs::path uiskin_result_path = converted / "uiskin";
  if (fs::exists (uiskin_result_path) || !fs::create_directory (uiskin_result_path))
    throw std::string ("error make converted dir");
  // find all subfolder inside uiskin
  for (const fs::directory_entry &skin : fs::directory_iterator (uiskin_path)) {
    // skip non directory
    if (!fs::is_directory (skin.status ())) continue;
    // make part skin
    fs::path uiskin_part_path = uiskin_result_path / skin.path ().filename ();
    fs::create_directory (uiskin_part_path);

    // bin rects for hold rects
    std::vector<stbi::rectpack::rect> image_rects;

    // as temporary data receive as data image holder (dih)
    int dih[3];

    // get all image files inside uiskin sub folder
    for (const fs::directory_entry &image : fs::directory_iterator (skin.path ())) {
      if (!fs::is_regular_file (image.status ())) continue;
      std::string image_path = image.path ().string ();
      if (!image_path.ends_with (".png")) continue;
      bool ninepatch = image_path.ends_with (".9.png");
      if (!stbi::load::info (image_path.c_str (), dih, dih + 1, dih + 2))
        throw std::string (stbi::load::failure_reason ());
      image_rects.push_back ({(unsigned int)dih[0] - 2*ninepatch, (unsigned int)dih[1] - 2*ninepatch, image_path, 0, 0, 0});
    }

    // packing
    if (!stbi::rectpack::pack_rects (PACK_SIZE, PACK_SIZE, image_rects.data (), image_rects.size ()))
      throw std::string ("cannot pack image for size PACK_SIZE x PACK_SIZE!");
    // write packed result
    fs::path outfile = uiskin_part_path / skin.path ().filename ();
    fs::path outfile_txt = outfile;
    outfile_txt += ".txt";
    std::ofstream atlas_out (outfile_txt.c_str (), std::ios::out | std::ios::trunc);
    if (!atlas_out.is_open ()) throw std::string ("fail stream atlas text");
    uint32_t *outBuffer = new uint32_t[PACK_SIZE * PACK_SIZE];
    unsigned char *image_buffer;
    // patch: left, top, right, bottom
    // padding: left, top, right, bottom
    unsigned int patch[4], padding[4];
    for (const stbi::rectpack::rect &r : image_rects) {
      image_buffer = stbi::load::load_from_filename (r.id.c_str (), dih, dih + 1, dih + 2, stbi::load::channel::rgb_alpha);
      if (!image_buffer) throw std::string (stbi::load::failure_reason ());
      {
        size_t lastSlashPos = r.id.find_last_of ("/\\");
        lastSlashPos = (lastSlashPos == std::string::npos) ? 0 : lastSlashPos + 1;
        size_t lastDotPos = r.id.find_last_of ('.');
        if (lastDotPos == std::string::npos || lastDotPos < lastSlashPos) {
          lastDotPos = r.id.length ();
        }
        atlas_out << r.id.substr (lastSlashPos, lastDotPos - lastSlashPos);
      }
    	bool ninepatch = r.id.ends_with(".9.png");
    	//left right patch
      if (ninepatch) {
      	//left
      	for (patch[0] = 0; patch[0] < r.w; ++patch[0])
      		if (*(((uint32_t*)image_buffer) + patch[0] + 1) == 0xff000000) break;
      	//right
      	for (patch[2] = 0; patch[2] < r.w; ++patch[2])
      		if (*(((uint32_t*)image_buffer) + r.w - patch[2]) == 0xff000000) break;
      }
      for (size_t y = 0; y < r.h; y++) {
      	if (ninepatch) {
      		// patch top, padding top
      		if (image_buffer + y)
      		patch[1] = patch
      		padding[1]
      	}
        memcpy ((void *)(outBuffer + (r.y + y) * PACK_SIZE + r.x), (void *)(image_buffer + (( y + ninepatch) * (r.w + 2 * ninepatch) + 1) * 4), r.w * 4);
      }
    	//left right padding
      if (ninepatch) {
      	//left
      	for (padding[0] = 0; padding[0] < r.w; ++padding[0])
      		if (*(((uint32_t*)image_buffer) + padding[0] + 1) == 0xff000000) break;
      	//right
      	for (padding[2] = 0; padding[2] < r.w; ++padding[2])
      		if (*(((uint32_t*)image_buffer) + r.w - padding[2]) == 0xff000000) break;
      }
      stbi::load::image_free (image_buffer);
      
      atlas_out << ":" << r.x << " " << r.y << " " << r.w << " " << r.h;
      if (ninepatch) {
	      atlas_out << " " << patch[0] << " " << patch[1]  << " " << patch[2]  << " " << patch[3];
	      atlas_out << " " << padding[0] << " " << padding[1]  << " " << padding[2]  << " " << padding[3];
      }
      atlas_out << std::endl;

    }
    atlas_out.close ();
    fs::path outfile_png = outfile;
    outfile_png += ".png";
    stbi::write::png (outfile_png.c_str (), PACK_SIZE, PACK_SIZE, 4, outBuffer, 0);
    delete[] outBuffer;
  }
}