#include "stbi_rectpack.hpp"

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

#define RE_ 30
#define RECTS 30

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

static unsigned int rtInt (unsigned int n) {
  if (n == 0 || n == 1)
    return n;
  double x, root = n;
  do {
    x = root;
    root = 0.5 * (x + n / x);
  } while (std::abs (root - x) > 0.0);
  return std::ceil (root);
}

bool stbi_rectpack_test () {
  std::cout << "STBI RECTPACK Test" << std::endl;
  try {
    stbi::rectpack::rect rects[RECTS];
    
    for(unsigned int re = 0; re < RE_; ++re) {
    unsigned int area = 0;
    for (stbi::rectpack::rect &rect : rects) {
      rect.w = genRNG (6) + 10;           // (0 ~ 63) + 10
      rect.h = genRNG (6) + 10;           // (0 ~ 63) + 10
      area += rect.w * rect.h;
    }
    const unsigned int Packed_Size = (unsigned int)(rtInt (area) * 1.05);
    std::cout << "Packed size: " << Packed_Size << " x " << Packed_Size << std::endl;
    bool result = stbi::rectpack::pack_rects (Packed_Size, Packed_Size, rects, RECTS);
    if (!result) {
	    for (const stbi::rectpack::rect &r : rects) {
	      if (r.was_packed)
	        std::cout << "√ " << r.w << " x " << r.h << " in (" << r.x << "," << r.y << ")" << std::endl;
	      else
	        std::cout << "× " << r.w << " x " << r.h << std::endl;
	    }
    	throw "All not packed within pack!";
    }
    std::cout << "Completed." << std::endl;
  }
    return true;
  } catch (const char *err) {
    std::cout << " Error: " << err << std::endl;
    return false;
  }
}