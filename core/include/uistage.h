#ifndef UISTAGE_INCLUDE
#define UISTAGE_INCLUDE

#define IMAGE_TYPE_TOTAL 4

enum image_type
{
  PANEL = 0,
  BUTTON_LIFE,
  BUTTON_PRESSED,
  BUTTON_DEATH,
};

extern void uistage_init();
extern void uistage_term();

#endif // UISTAGE_INCLUDE