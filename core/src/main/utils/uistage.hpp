#ifndef Included_UISTAGE_
#define Included_UISTAGE_

#include "../engine.hpp"
#include "math.hpp"
#include <cstdint>
#include <initializer_list>
#include <string>

namespace uistage {
struct texKey_state {
  enum State : unsigned int {
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
  virtual Rect &getRect () = 0;
  virtual std::string getKey () = 0;
  virtual size_t getType () const = 0;
  virtual void draw (float);
  virtual ~actor () {}
};

struct text_actor : public actor {
private:
  std::string text;
  Rect rectangle;
public:
  text_actor (float, float, Align, std::string);
  Rect &getRect () override;
  std::string getKey () override;
  void draw (float) override;
  size_t getType () const override;
  void setText(const char *, ...);
  ~text_actor () override;
};
struct image_actor : public actor {
private:
  std::string key;
  Rect rectangle;
public:
  image_actor (std::string, Rect);
  Rect &getRect () override;
  std::string getKey () override;
  size_t getType () const override;
  void draw (float) override;
  ~image_actor () override;
};
struct button_actor : public uistage::actor {
private:
  std::string *keys;
  size_t mstate = 0;
  Rect rectangle;
public:
  void (*onClick) ();
  button_actor (std::string *, Rect, void(*)());
  Rect &getRect () override;
  std::string getKey () override;
  void setState (size_t);
  size_t getType () const override;
  void draw (float) override;
  ~button_actor () override;
};

struct texture_region {
  unsigned int pos[2], size[2];
  unsigned int patch[4];
};
void loadBMFont (const char *);
void addTextureRegion (std::string, engine::texture_core *, const texture_region &);
void addTextureRegion (std::string, engine::texture_core *, const texture_region &, const uint32_t);
void draw (float);
void clear ();

// all actor types
image_actor *makeImage (std::string, Rect);
button_actor *makeButton (std::initializer_list<std::string>, Rect, void (*) ());
text_actor *makeText (float, float, Align, std::string);

void touchDown (float, float, int, int);
void touchMove (float, float, float, float, int, int);
void touchUp (float, float, int, int);
void touchCanceled (float, float, int, int);

} // namespace uistage

#endif // Included_UISTAGE_