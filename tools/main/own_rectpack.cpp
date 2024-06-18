#include "own_rectpack.hpp"

struct packer__skyline_node {
  int x, y, width;
};

own::rectpack::Packer::Packer(int width, int height) : binWidth(width), binHeight(height) {
  // Inisialisasi skyline dengan posisi (0,0) dengan lebar sebesar bin
  skyline.push_back({0, 0, binWidth});
}

bool own::rectpack::Packer::insert(Rect& rect) {
  for (size_t i = 0; i < skyline.size(); ++i) {
  	const packer__skyline_node &node = skyline[i];
	  if (node.x + rect.w <= binWidth) {
		  int widthLeft = rect.w;
		  int y = node.y;
			for (size_t j = skyline.size() - 1; j < skyline.size(); --j) {
			  if (y + rect.h <= binHeight) {
				  widthLeft -= skyline[j].width;
				  if (widthLeft < 0) {
      			placeAt(i, skyline[i], rect, y);
      			return true;
				  }
				  y = std::max(y, skyline[j].y);
			  }
			}
		}
  }
  return false;
}

const std::vector<Rect>& own::rectpack::Packer::getPackedRects() const {
  return packedRects;
}

static void placeAt(size_t skylineIndex, const packer__skyline_node& node, Rect& rect, int y) {
  rect.x = node.x;
  rect.y = y;
  packedRects.push_back(rect);

  packer__skyline_node newNode = {rect.x, rect.y + rect.h, rect.w};
  skyline.insert(skyline.begin() + skylineIndex, newNode);

  for (size_t i = skylineIndex + 1; i < skyline.size(); ++i) {
    if (skyline[i].x < newNode.x + newNode.width) {
      skyline[i].x = newNode.x + newNode.width;
      skyline[i].width -= newNode.width;
      if (skyline[i].width <= 0) {
        skyline.erase(skyline.begin() + i);
        --i;
      } else {
        break;
      }
    }
  }
  
  //merge skyline
	for (size_t i = 0; i < skyline.size() - 1; ++i) {
	  if (skyline[i].y == skyline[i + 1].y) {
      skyline[i].width += skyline[i + 1].width;
      skyline.erase(skyline.begin() + i + 1);
      --i;
	  }
	}
}

#endif OWN_RECTPACK_HPP