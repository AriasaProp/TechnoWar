#include "stbi_load.hpp"
#include "stbi_rectpack.hpp"
#include "stbi_write.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;

#define PACKED_SIZE 512

int main (int argc, char *argv[]) {
  try {
    std::cout << "Converting ... " << std::endl;
    if ((argc < 2) || !argv[1] || !argv[1][0])
      throw "Input empty";
    stbi::rectpack::context p_context (PACKED_SIZE, PACKED_SIZE);
    std::vector<stbi::rectpack::rect> rects{
        {8, 21},
        {15, 10},
        {30, 20},
        {21, 7},
        {30, 14}};
    std::vector<uint32_t> colors{
        0xff0000ff,
        0x00ff00ff,
        0x0000ffff,
        0x00ffffff,
        0xffff00ff};

    if (!stbi::rectpack::pack_rects (&p_context, rects.data (), rects.size ()))
      std::cout << "Warning: All not packed!" << std::endl;
    unsigned int *outBuffer = new unsigned int[PACKED_SIZE * PACKED_SIZE];
    size_t color_count = 0;
    for (stbi::rectpack::rect r : rects) {
      if (!r.was_packed) continue;
      for (size_t y = 0; y < r.h; y++)
        std::fill_n (outBuffer + ((r.y + y) * PACKED_SIZE) + r.x, r.w, colors[color_count]);
      ++color_count;
      color_count %= 5;
    }
    stbi::write::png (argv[1], PACKED_SIZE, PACKED_SIZE, stbi::load::channel::rgb_alpha, (void *)outBuffer, 0);
    delete[] outBuffer;
    std::cout << "Output: " << argv[1] << " completed." << std::endl;
  } catch (const fs::filesystem_error &e) {
    std::cerr << "Error reading directory: " << e.what () << std::endl;
    return EXIT_FAILURE;
  } catch (const char *err) {
    std::cout << " Error: " << err << std::endl;
    return EXIT_FAILURE;
  }
}