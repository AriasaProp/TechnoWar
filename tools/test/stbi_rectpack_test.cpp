#include "stbi_rectpack.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>

#define RE_ 20
#define RECTS 20

static inline unsigned int genRNG (unsigned int numBits) {
  static std::atomic<unsigned int> seed;
  unsigned int lfsr = seed.load ();
  lfsr += static_cast<unsigned int> (time (0));
  seed.store (lfsr);
  lfsr &= ((1u << numBits) - 1);
  unsigned int bit = ((lfsr >> 0) ^ (lfsr >> 1) ^ (lfsr >> 3) ^ (lfsr >> 4)) & 1;
  lfsr = (lfsr >> 1) | (bit << (numBits - 1));
  return lfsr;
}

bool stbi_rectpack_test () {
  std::cout << "STBI RECTPACK Test" << std::endl;
  try {
    stbi::rectpack::rect rects[RECTS];

    unsigned int area;
    for (unsigned int re = 0; re < RE_; ++re) {
      std::cout << "Packed size " << re << ": ";
      area = 0;
      for (stbi::rectpack::rect &rect : rects) {
        rect.w = genRNG (6) + 10; // (0 ~ 63) + 10
        rect.h = genRNG (6) + 10; // (0 ~ 63) + 10
        area += rect.w * rect.h;
      }
      std::cout << "\n  total area " << area << " is ";
      {
        const double n = static_cast<double> (area) * 1.3;
        double root = n;
        double x;
        do {
          x = root;
          root = 0.5 * (x + n / x);
        } while (std::abs (root - x) > 0.0);
        root = std::ceil (root);
        area = static_cast<unsigned int> (root) + 5;
      }
      std::cout << "\n  used area " << area << " x " << area << " = " << area * area << " is ";
      if (stbi::rectpack::pack_rects (area, area, rects, RECTS)) {
        std::cout << "Success" << std::endl;
      } else {
        std::cout << "Failure" << std::endl;
        std::cout << "There is the list: " << std::endl;
        for (const stbi::rectpack::rect &r : rects) {
          if (r.was_packed)
            std::cout << "√ " << r.w << " x " << r.h << " in (" << r.x << "," << r.y << ")" << std::endl;
          else
            std::cout << "× " << r.w << " x " << r.h << std::endl;
        }
        throw "there is not enough space!";
      }
    }
    return true;
  } catch (const char *err) {
    std::cerr << " Error: " << err << std::endl;
    return false;
  }
}