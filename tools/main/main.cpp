#include "stbi_load.hpp"
#include "stbi_rectpack.hpp"
#include "stbi_write.hpp"

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;

#define PACKED_SIZE 4096
#define NODE_SIZE 5000

int main (int argc, char *argv[]) {
  try {
    std::cout << "Converting ... " << std::endl;
    if ((argc < 3) || !argv[1] || !argv[1][0] || !argv[2] || !argv[2][0])
      throw "Input empty";
    std::cout << "Input folder: " << argv[1] << std::endl;
    static const std::set<std::string> exten = {".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif"};
    stbi::rectpack::context p_context;
    stbi::rectpack::node p_nodes[NODE_SIZE];
    stbi::rectpack::init_target (&p_context, PACKED_SIZE, PACKED_SIZE, p_nodes, NODE_SIZE);
    std::vector<stbi::rectpack::rect> rects;
    for (const auto &entry : fs::directory_iterator (argv[1])) {
      if ((!fs::is_regular_file (entry.status ())) || (exten.find (entry.path ().extension ().string ()) == exten.end ())) continue;
      static int dat[3];
      stbi::load::info (entry.path ().c_str (), dat, dat + 1, dat + 2);
      if (dat[2] != stbi::load::channel::rgb_alpha) continue;
    	std::cout << "Size: " << dat[0] << " x " << dat[1] << ", Channel is RGBA." << std::endl;
      rects.push_back ({(void *)new std::string (entry.path ().string ()), dat[0], dat[1], 0, 0, 0});
    }
    if (!stbi::rectpack::pack_rects (&p_context, rects.data (), rects.size ()))
    	std::cout << "Warning: All not packed!" << std::endl;
    unsigned char *outBuffer = new unsigned char[PACKED_SIZE * PACKED_SIZE * 4];
    for (stbi::rectpack::rect r : rects) {
    	if (!r.was_packed) continue;
      static int dat[3];
      std::string *iname = static_cast<std::string *> (r.id);
      std::cout << "i: " << *iname;
      unsigned char *inpBuffer = stbi::load::load_from_filename (iname->c_str (), dat, dat + 1, dat + 2, stbi::load::channel::rgb_alpha);
      if (inpBuffer) {
      	static const int w_b = 4 * sizeof(unsigned char);
        for (unsigned y = 0; y < dat[1]; y++)
          memcpy (outBuffer + ((r.y + y) * PACKED_SIZE * w_b) + (r.x * w_b), inpBuffer + (y * dat[0] w_b), dat[0] * w_b);
        stbi::load::image_free (inpBuffer);
        std::cout << " is loaded";
      } else {
        std::cout << " is failed to load";
      }
      std::cout << std::endl;
      delete iname;
    }
    rects.clear ();
    stbi::write::png (argv[2], PACKED_SIZE, PACKED_SIZE, stbi::load::channel::rgb_alpha, outBuffer, 0);
    delete[] outBuffer;
    std::cout << "Output: " << argv[2] << " completed." << std::endl;
  } catch (const fs::filesystem_error &e) {
    std::cerr << "Error reading directory: " << e.what () << std::endl;
    return EXIT_FAILURE;
  } catch (const char *err) {
    std::cout << " Error: " << err << std::endl;
    std::cout << " STBI Error: " << stbi::load::failure_reason () << std::endl;
    return EXIT_FAILURE;
  }
}