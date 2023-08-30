#ifndef Included_UISTAGE_
#define Included_UISTAGE_

#include "../engine.hpp"
#include "math.hpp"
#include <string>
#include <cstdint>

namespace uistage {
  struct texKey_state {
    enum State: unsigned int{
      Idle = 0,
      Hover,
      Pressed,
      Disabled,
      Focused,
      Selected,
      Error,
      Loading,
      Checked,
      Empty,
      Active
    } mState;
    std::string key;
  };
  struct actor {
    virtual Rect getRect() const = 0; 
    virtual std::string texKey() const = 0;
  };
  struct texture_region {
    unsigned int pos[2], size[2];
    unsigned int patch[4];
  };
  //void addBMFont();
  void addTextureRegion(std::string,engine::texture_core*, const texture_region &);
  void addTextureRegion(std::string,engine::texture_core*, const texture_region &, const size_t);
  void act(float);
  void draw();
  void clear();
  
  //all actor types
  actor *makeImage(std::string,Rect);
  actor *makeButton(texKey_state*,Rect);
}

#endif //Included_UISTAGE_