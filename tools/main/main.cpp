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
#include <cmemory>

namespace fs = std::filesystem;

#define PACKED_SIZE 80

int main (int argc, char *argv[]) {
  try {
    std::cout << "Converting ... " << std::endl;
    if ((argc < 2) || !argv[1] || !argv[1][0])
      throw "Input empty";
    stbi::rectpack::context p_context (PACKED_SIZE, PACKED_SIZE);
    std::cout << "Contexted ... " << std::endl;
    stbi::rectpack::rect rects[5] = {
        {8, 21, 0, 0, 0},
        {15, 10, 0, 0, 0},
        {30, 20, 0, 0, 0},
        {21, 7, 0, 0, 0},
        {30, 14, 0, 0, 0}};
    std::cout << "Rects ... " << std::endl;
    uint32_t colors[5] = {
        0xff0000ff,
        0x00ff00ff,
        0x0000ffff,
        0x00ffffff,
        0xffff00ff};
    std::cout << "Colors ... " << std::endl;

    if (!stbi::rectpack::pack_rects (&p_context, rects, 5))
      std::cout << "Warning: All not packed!" << std::endl;
    std::cout << "Packed ... " << std::endl;
    unsigned int *outBuffer = (unsigned int *) malloc(PACKED_SIZE * PACKED_SIZE * sizeof(uint32_t));
    std::cout << "Buffed ... " << std::endl;
    size_t color_count = 0;
    for (stbi::rectpack::rect r : rects) {
      if (!r.was_packed) continue;
      for (size_t y = 0; y < r.h; y++)
        std::fill_n (outBuffer + ((r.y + y) * PACKED_SIZE) + r.x, r.w, colors[color_count]);
      ++color_count;
      color_count %= 5;
    }
    std::cout << "Filled ... " << std::endl;
    stbi::write::png (argv[1], PACKED_SIZE, PACKED_SIZE, stbi::load::channel::rgb_alpha, (void *)outBuffer, 0);
    std::cout << "Write ... " << std::endl;
    free(outBuffer);
    std::cout << "Cleanup ... " << std::endl;
    std::cout << "Output: " << argv[1] << " completed." << std::endl;
  } catch (const fs::filesystem_error &e) {
    std::cerr << "Error reading directory: " << e.what () << std::endl;
    return EXIT_FAILURE;
  } catch (const char *err) {
    std::cout << " Error: " << err << std::endl;
    return EXIT_FAILURE;
  }
}