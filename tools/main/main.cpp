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

namespace fs = std::filesystem;

#define PACKED_SIZE 235

int main (int argc, char *argv[]) {
  try {
    std::cout << "Converting ... " << std::endl;
    if ((argc < 2) || !argv[1] || !argv[1][0])
      throw "Input empty";
    stbi::rectpack::rect rects[7] = {
        {8, 21, (0xff0000ff), 0, 0, 0},
        {15, 10, (0x00ff00ff), 0, 0, 0},
        {30, 20, (0x0000ffff), 0, 0, 0},
        {21, 7, (0x00ffffff), 0, 0, 0},
        {7, 7, (0xffffffff), 0, 0, 0},
        {7, 7, (0x000000ff), 0, 0, 0},
        {30, 14, (0xffff00ff), 0, 0, 0}};

    if (stbi::rectpack::pack_rects (PACKED_SIZE, PACKED_SIZE, rects, 5)) {
      uint32_t outBuffer[PACKED_SIZE * PACKED_SIZE] = {0};
      for (size_t i = 0; i < 7; ++i) {
        stbi::rectpack::rects &r = rects[i];
        if (r.was_packed)
          for (size_t y = 0; y < r.h; y++)
            std::fill_n (outBuffer + ((r.y + y) * PACKED_SIZE) + r.x, r.w, uint32_t (r.id));
      }
      stbi::write::png (argv[1], PACKED_SIZE, PACKED_SIZE, stbi::load::channel::rgb_alpha, (void *)outBuffer, 0);
      std::cout << "Output: " << argv[1] << " completed." << std::endl;
    } else
      std::cout << "Error: All not packed!" << std::endl;
  } catch (const fs::filesystem_error &e) {
    std::cerr << "Error reading directory: " << e.what () << std::endl;
    return EXIT_FAILURE;
  } catch (const char *err) {
    std::cout << " Error: " << err << std::endl;
    return EXIT_FAILURE;
  }
}