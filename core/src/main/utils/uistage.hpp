#ifndef Included_UISTAGE_
#define Included_UISTAGE_

#include "../engine.hpp"
#include "math.hpp"
#include <string>
#include <cstdint>
#include <initializer_list>

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
    virtual Rect &getRect() = 0; 
    virtual std::string texKey() = 0;
    virtual size_t getType() const = 0;
    virtual ~actor() {}
  };
  struct texture_region {
    unsigned int pos[2], size[2];
    unsigned int patch[4];
  };
  //void addBMFont();
  void addTextureRegion(std::string,engine::texture_core*, const texture_region &);
  void addTextureRegion(std::string,engine::texture_core*, const texture_region &, const uint32_t);
  void draw(float);
  void clear();
  
  //all actor types
  actor *makeImage(std::string,Rect);
  actor *makeButton(std::initializer_list<std::string>,Rect);
}

#endif //Included_UISTAGE_