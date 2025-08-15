#ifndef UISTAGE_INCLUDE
#define UISTAGE_INCLUDE

#include "common.h"

typedef enum : uint8_t {
  PIVOT_INVALID = 0,

  PIVOT_VTOP_HLEFT = 1,
  PIVOT_TOP = 2,
  PIVOT_VTOP_HRIGHT = 3,

  PIVOT_HLEFT = 4,
  PIVOT_CENTER = 5,
  PIVOT_HRIGHT = 6,

  PIVOT_VBOTTOM_HLEFT = 7,
  PIVOT_VBOTTOM = 8,
  PIVOT_VBOTTOM_HRIGHT = 9,
}
pivot_state;

// actor
typedef uint16_t actor;

#ifndef UISTAGE_IMPLEMENTATION
extern actor create_label(size_t);
extern actor destroy_actor(actor);

extern void set_actor_origin(actor, const vec2);
extern void set_actor_pivot_origin(actor, const pivot_state);
extern void set_actor_pivot_world(actor, const pivot_state);
extern void set_label_text(actor, const char *);

extern void uistage_init();
extern void uistage_draw();
extern void uistage_term();
#endif // UISTAGE_IMPLEMENTATION

#endif // UISTAGE_INCLUDE