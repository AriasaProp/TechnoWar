#include "stbi_load.hpp"
#include "stbi_rectpack.hpp"
#include "stbi_write.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

namespace fs = std::filesystem;

#define RECTS 30

unsigned int genRNG (unsigned int numBits) {
  static std::atomic<unsigned int> seed;
  unsigned int lfsr = seed.load ();
  lfsr += static_cast<unsigned int> (time (0));
  seed.store (lfsr);
  lfsr &= ((1u << numBits) - 1);
  unsigned int bit = ((lfsr >> 0) ^ (lfsr >> 1) ^ (lfsr >> 3) ^ (lfsr >> 4)) & 1;
  lfsr = (lfsr >> 1) | (bit << (numBits - 1));
  return lfsr;
}

unsigned int rtInt (unsigned int n) {
  if (n == 0 || n == 1)
    return n;
  double x, root = n;
  do {
    x = root;
    root = 0.5 * (x + n / x);
  } while (std::abs (root - x) > 0.0);
  return std::ceil (root);
}

int main (int argc, char *argv[]) {
  try {
    std::cout << "Converting ... " << std::endl;
    if ((argc < 2) || !argv[1] || !argv[1][0])
      throw "Input empty";

    stbi::rectpack::rect rects[RECTS];
    unsigned int area = 0;
    for (stbi::rectpack::rect &rect : rects) {
      rect.id = 0xff000000 | genRNG (24); // 0 ~ 0x00ffffff
      rect.w = genRNG (6) + 10;           // (0 ~ 63) + 10
      rect.h = genRNG (6) + 10;           // (0 ~ 63) + 10
      area += rect.w * rect.h;
    }
    unsigned int Packed_Size = (unsigned int)(rtInt (area) * 1.05);
    if (!stbi::rectpack::pack_rects (Packed_Size, Packed_Size, rects, RECTS))
      std::cout << "Warning: All not packed! with " << Packed_Size << " px2" << std::endl;
    uint32_t outBuffer[Packed_Size * Packed_Size] = {0};
    for (size_t i = 0; i < RECTS; ++i) {
      const stbi::rectpack::rect &r = rects[i];
      if (r.was_packed) {
        for (size_t y = 0; y < r.h; y++)
          std::fill_n (outBuffer + ((r.y + y) * Packed_Size) + r.x, r.w, uint32_t (r.id));
        std::cout << "packed " << r.w << " x " << r.h << " in (" << r.x << "," << r.y << ")" << std::endl;
      } else {
        std::cout << "not packed " << r.w << " x " << r.h << std::endl;
      }
    }
    stbi::write::png (argv[1], Packed_Size, Packed_Size, stbi::load::channel::rgb_alpha, (void *)outBuffer, 0);
    std::cout << "Output: " << argv[1] << " completed." << std::endl;
  } catch (const fs::filesystem_error &e) {
    std::cerr << "Error file: " << e.what () << std::endl;
    return EXIT_FAILURE;
  } catch (const char *err) {
    std::cout << " Error: " << err << std::endl;
    return EXIT_FAILURE;
  }
}