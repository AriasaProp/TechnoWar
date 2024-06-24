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

#define PACKED_SIZE 124
#define RECTS 8

int main (int argc, char *argv[]) {
  try {
    std::cout << "Converting ... " << std::endl;
    if ((argc < 2) || !argv[1] || !argv[1][0])
      throw "Input empty";
    
    stbi::rectpack::rect rects[RECTS] = {
        { 8, 21, (0xff0000ff), 0, 0, 0},   // red
        {15, 10, (0xff00ff00), 0, 0, 0},  // green
        {30, 20, (0xffff0000), 0, 0, 0},  // blue
        {21,  7, (0xffffff00), 0, 0, 0},   // green+blue
        {13, 21, (0xff213473), 0, 0, 0},  // ?
        {16, 44, (0xffa2bf00), 0, 0, 0},  //
        {16, 44, (0xffa0bf0a), 0, 0, 0},  //
        {30, 14, (0xff00ffff), 0, 0, 0}}; // red + green

    if (stbi::rectpack::pack_rects (PACKED_SIZE, PACKED_SIZE, rects, RECTS)) {
      uint32_t outBuffer[PACKED_SIZE * PACKED_SIZE] = {0};
      for (size_t i = 0; i < RECTS; ++i) {
        const stbi::rectpack::rect &r = rects[i];
        if (!r.was_packed) continue;
        for (size_t y = 0; y < r.h; y++)
          std::fill_n (outBuffer + ((r.y + y) * PACKED_SIZE) + r.x, r.w, uint32_t (r.id));
        std::cout << "packed " << r.w << " x " << r.h << " in (" << r.x << "," << r.y << ")" << std::endl;
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