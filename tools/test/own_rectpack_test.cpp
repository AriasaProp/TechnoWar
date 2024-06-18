#include "own_rectpack.hpp"

#include <iostream>
#include <vector>
#include <cstring>

bool own_rectpack_test() {
  std::cout << "Start Test - Rect pack" << std::endl;
	unsigned int binWidth = 64;
  unsigned int binHeight = 64;
  own::rectpack::Packer packer(binWidth, binHeight);

  std::vector<own::rectpack::Rect> rects = {
      {0, 0, 19, 7},
      {0, 0, 21, 5},
      {0, 0, 8, 2},
      {0, 0, 5, 19},
      {0, 0, 18, 11},
      {0, 0, 14, 13}
      {0, 0, 21, 4},
      {0, 0, 14, 13}
  };
  std::cout << "Result: \n"
  for (auto& rect : rects) {
    std::cout << "Rect " << rect.w << " x " << rect.h << (packer.insert(rect))? "  success" : "  fail" << "\n";
  }
  
  char output[binHeight*binWidth];
  memset(output, ' ', sizeof output);
  std::cout << "\n\nVisual: \n"
  char outW = 'A';
  for (const auto& rect : packer.getPackedRects()) {
	  for (unsigned y = 0; y < rect.h; ++y) {
	  	memset(output+((y+rect.y)*binWidth)+rect.x, outW, rect.w);
	  }
	  outW++;
  }
  char outline[binWidth];
  for (unsigned y = 0; y < binHeight; ++y) {
  	memcpy(outline, output+(y*binWidth), sizeof outline);
  	std::cout << "\n" << outline;
  }
  std::cout << std::endl;

	return true;
}