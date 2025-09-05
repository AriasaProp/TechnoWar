#ifndef UISTAGE_INCLUDE
#define UISTAGE_INCLUDE

#include "common.h"

typedef enum : uint8_t {
  PIVOT_INVALID = 0,
  PIVOT_LEFT = 1,
  PIVOT_RIGHT = 2,
  PIVOT_HCENTER = 3,

  PIVOT_BOTTOM = 4,
  PIVOT_TOP = 8,
  PIVOT_VCENTER = 12,
} pivot_state;

// actor
typedef uint16_t actor;

#ifndef UISTAGE_IMPLEMENTATION
extern actor create_text(size_t);
extern void destroy_actor(actor);

extern void set_actor_origin(actor, const vec2);
extern void set_actor_pivot(actor, const pivot_state, const pivot_state);
extern void set_text_str(actor, const char *, ...);

extern void uistage_init();
extern void uistage_draw();
extern void uistage_term();
#endif // UISTAGE_IMPLEMENTATION

#endif // UISTAGE_INCLUDE