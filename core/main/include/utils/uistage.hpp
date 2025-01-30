#ifndef Included_UISTAGE_
#define Included_UISTAGE_

#include "engine.h"
#include "math.hpp"
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>

namespace uistage {
struct actor {
  virtual Rect &getRect () = 0;
  virtual std::string getKey () = 0;
  virtual void draw (float);
  virtual ~actor () {}
};

struct text_actor : public actor {
private:
  std::string text;
  Rect rectangle;

public:
  text_actor (Vector2, const Align &, std::string);
  Rect &getRect () override;
  std::string getKey () override;
  void draw (float) override;
  void setText (const char *, ...);
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
  button_actor (std::string *, Rect, void (*) ());
  Rect &getRect () override;
  std::string getKey () override;
  void setState (size_t);
  void draw (float) override;
  ~button_actor () override;
};

struct texture_region {
  unsigned int pos[2], size[2];
  unsigned int patch[4], padding[4];
  color32_t rc;
};
void loadBMFont (const char *);
void loadUISkin (const char *);
void draw (float);
void clear ();

// all actor types
image_actor *makeImage (std::string, Rect);
button_actor *makeButton (std::initializer_list<std::string>, Rect, void (*) ());
text_actor *makeText (Vector2, const Align &, std::string);
void temporaryTooltip ();
void temporaryTooltip (const char *, ...);

void touchDown (float, float, int, int);
void touchMove (float, float, float, float, int, int);
void touchUp (float, float, int, int);
void touchCanceled (float, float, int, int);
} // namespace uistage

#endif // Included_UISTAGE_