#ifndef OWN_RECTPACK_HPP
#define OWN_RECTPACK_HPP

#include <iostream>
#include <vector>
#include <algorithm>

namespace own {
namespace rectpack {
	
struct Rect {
    int x, y, w, h;
};

struct Packer {
public:
  Packer(int, int);
  bool pack(Rect&);
  const std::vector<Rect>& getPackedRects() const;
private:
    int binWidth, binHeight;
    std::vector<packer__skyline_node> skyline;
    std::vector<Rect> packedRects;
};

int main() {
    int binWidth = 100;
    int binHeight = 100;
    Packer packer(binWidth, binHeight);

    std::vector<Rect> rects = {
        {0, 0, 30, 40},
        {0, 0, 20, 50},
        {0, 0, 50, 20},
        {0, 0, 40, 30}
    };

    for (auto& rect : rects) {
        if (!packer.pack(rect)) {
            std::cout << "Failed to pack rect with size (" << rect.w << ", " << rect.h << ")\n";
        }
    }

    for (const auto& rect : packer.getPackedRects()) {
        std::cout << "Packed rect at (" << rect.x << ", " << rect.y << ") with size (" << rect.w << ", " << rect.h << ")\n";
    }

    return 0;
}

} // namespace rectpack
} // namespace own

#endif OWN_RECTPACK_HPP