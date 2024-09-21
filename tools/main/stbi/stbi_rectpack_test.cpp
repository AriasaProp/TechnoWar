#include "stbi_rectpack.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
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

void stbi_rectpack_test () {
  stbi::rectpack::rect rects[RECTS];
  unsigned int area_total, area_used;
  for (unsigned int re = 0; re < RE_; ++re) {
    area_total = 0;
    for (stbi::rectpack::rect &rect : rects) {
      rect.w = genRNG (6) + 10; // (0 ~ 63) + 10
      rect.h = genRNG (6) + 10; // (0 ~ 63) + 10
      area_total += rect.w * rect.h;
    }
    {
      const double n = static_cast<double> (area_total) * 1.3;
      double root = n;
      double x;
      do {
        x = root;
        root = 0.5 * (x + n / x);
      } while (std::abs (root - x) > 0.0);
      root = std::ceil (root);
      area_used = static_cast<unsigned int> (root) + 5;
    }
    if (!stbi::rectpack::pack_rects (area_used, area_used, rects, RECTS)) {
    	std::stringstream serr;
    	serr << "STBI RectPack Failure - Packing rect with container " << area_used * area_used << " px² and total rect area " << area_total << " px², there is:\n";
      for (const stbi::rectpack::rect &r : rects) {
      	serr << "[";
        if (r.was_packed)
          serr << "√ " << r.w << " x " << r.h << " in (" << r.x << "," << r.y << ")";
        else
          serr << "× " << r.w << " x " << r.h;
      	serr << "]";
      }
      serr << "\n";
      throw serr.str().c_str();
    }
  }
}