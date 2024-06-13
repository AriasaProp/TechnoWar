#include "stb_rect_pack.hpp"

#ifndef STBRP_SORT
#include <cstdlib>
#define STBRP_SORT qsort
#endif

#ifndef STBRP_ASSERT
#include <cassert>
#define STBRP_ASSERT assert
#endif

#ifdef _MSC_VER
#define STBRP__NOTUSED(v) (void)(v)
#define STBRP__CDECL __cdecl
#else
#define STBRP__NOTUSED(v) (void)sizeof (v)
#define STBRP__CDECL
#endif

enum {
  STBRP__INIT_skyline = 1
};

void stbi::rect_pack::setup_heuristic (stbi::rect_pack::context *context, int heuristic) {
  switch (context->init_mode) {
  case STBRP__INIT_skyline:
    STBRP_ASSERT (heuristic == STBRP_HEURISTIC_Skyline_BL_sortHeight || heuristic == STBRP_HEURISTIC_Skyline_BF_sortHeight);
    context->heuristic = heuristic;
    break;
  default:
    STBRP_ASSERT (0);
  }
}

void stbi::rect_pack::setup_allow_out_of_mem (stbi::rect_pack::context *context, int allow_out_of_mem) {
  if (allow_out_of_mem)
    // if it's ok to run out of memory, then don't bother aligning them;
    // this gives better packing, but may fail due to OOM (even though
    // the rectangles easily fit). @TODO a smarter approach would be to only
    // quantize once we've hit OOM, then we could get rid of this parameter.
    context->align = 1;
  else {
    // if it's not ok to run out of memory, then quantize the widths
    // so that num_nodes is always enough nodes.
    //
    // I.e. num_nodes * align >= width
    //                  align >= width / num_nodes
    //                  align = ceil(width/num_nodes)

    context->align = (context->width + context->num_nodes - 1) / context->num_nodes;
  }
}

void stbi::rect_pack::init_target (stbi::rect_pack::context *context, int width, int height, stbi::rect_pack::node *nodes, int num_nodes) {
  int i;

  for (i = 0; i < num_nodes - 1; ++i)
    nodes[i].next = &nodes[i + 1];
  nodes[i].next = NULL;
  context->init_mode = STBRP__INIT_skyline;
  context->heuristic = STBRP_HEURISTIC_Skyline_default;
  context->free_head = &nodes[0];
  context->active_head = &context->extra[0];
  context->width = width;
  context->height = height;
  context->num_nodes = num_nodes;
  stbi::rect_pack::setup_allow_out_of_mem (context, 0);

  // node 0 is the full width, node 1 is the sentinel (lets us not store width explicitly)
  context->extra[0].x = 0;
  context->extra[0].y = 0;
  context->extra[0].next = &context->extra[1];
  context->extra[1].x = (uint32_t)width;
  context->extra[1].y = (1 << 30);
  context->extra[1].next = NULL;
}

// find minimum y position if it starts at x1
static int stbrp__skyline_find_min_y (stbi::rect_pack::context *c, stbi::rect_pack::node *first, int x0, int width, int *pwaste) {
  stbi::rect_pack::node *node = first;
  int x1 = x0 + width;
  int min_y, visited_width, waste_area;

  STBRP__NOTUSED (c);

  STBRP_ASSERT (first->x <= x0);

#if 0
   // skip in case we're past the node
   while (node->next->x <= x0)
      ++node;
#else
  STBRP_ASSERT (node->next->x > x0); // we ended up handling this in the caller for efficiency
#endif

  STBRP_ASSERT (node->x <= x0);

  min_y = 0;
  waste_area = 0;
  visited_width = 0;
  while (node->x < x1) {
    if (node->y > min_y) {
      // raise min_y higher.
      // we've accounted for all waste up to min_y,
      // but we'll now add more waste for everything we've visted
      waste_area += visited_width * (node->y - min_y);
      min_y = node->y;
      // the first time through, visited_width might be reduced
      if (node->x < x0)
        visited_width += node->next->x - x0;
      else
        visited_width += node->next->x - node->x;
    } else {
      // add waste area
      int under_width = node->next->x - node->x;
      if (under_width + visited_width > width)
        under_width = width - visited_width;
      waste_area += under_width * (min_y - node->y);
      visited_width += under_width;
    }
    node = node->next;
  }

  *pwaste = waste_area;
  return min_y;
}

typedef struct
{
  int x, y;
  stbi::rect_pack::node **prev_link;
} stbrp__findresult;

static stbrp__findresult stbrp__skyline_find_best_pos (stbi::rect_pack::context *c, int width, int height) {
  int best_waste = (1 << 30), best_x, best_y = (1 << 30);
  stbrp__findresult fr;
  stbi::rect_pack::node **prev, *node, *tail, **best = NULL;

  // align to multiple of c->align
  width = (width + c->align - 1);
  width -= width % c->align;
  STBRP_ASSERT (width % c->align == 0);

  // if it can't possibly fit, bail immediately
  if (width > c->width || height > c->height) {
    fr.prev_link = NULL;
    fr.x = fr.y = 0;
    return fr;
  }

  node = c->active_head;
  prev = &c->active_head;
  while (node->x + width <= c->width) {
    int y, waste;
    y = stbrp__skyline_find_min_y (c, node, node->x, width, &waste);
    if (c->heuristic == STBRP_HEURISTIC_Skyline_BL_sortHeight) { // actually just want to test BL
      // bottom left
      if (y < best_y) {
        best_y = y;
        best = prev;
      }
    } else {
      // best-fit
      if (y + height <= c->height) {
        // can only use it if it first vertically
        if (y < best_y || (y == best_y && waste < best_waste)) {
          best_y = y;
          best_waste = waste;
          best = prev;
        }
      }
    }
    prev = &node->next;
    node = node->next;
  }

  best_x = (best == NULL) ? 0 : (*best)->x;

  // if doing best-fit (BF), we also have to try aligning right edge to each node position
  //
  // e.g, if fitting
  //
  //     ____________________
  //    |____________________|
  //
  //            into
  //
  //   |                         |
  //   |             ____________|
  //   |____________|
  //
  // then right-aligned reduces waste, but bottom-left BL is always chooses left-aligned
  //
  // This makes BF take about 2x the time

  if (c->heuristic == STBRP_HEURISTIC_Skyline_BF_sortHeight) {
    tail = c->active_head;
    node = c->active_head;
    prev = &c->active_head;
    // find first node that's admissible
    while (tail->x < width)
      tail = tail->next;
    while (tail) {
      int xpos = tail->x - width;
      int y, waste;
      STBRP_ASSERT (xpos >= 0);
      // find the left position that matches this
      while (node->next->x <= xpos) {
        prev = &node->next;
        node = node->next;
      }
      STBRP_ASSERT (node->next->x > xpos && node->x <= xpos);
      y = stbrp__skyline_find_min_y (c, node, xpos, width, &waste);
      if (y + height <= c->height) {
        if (y <= best_y) {
          if (y < best_y || waste < best_waste || (waste == best_waste && xpos < best_x)) {
            best_x = xpos;
            STBRP_ASSERT (y <= best_y);
            best_y = y;
            best_waste = waste;
            best = prev;
          }
        }
      }
      tail = tail->next;
    }
  }

  fr.prev_link = best;
  fr.x = best_x;
  fr.y = best_y;
  return fr;
}

static stbrp__findresult stbrp__skyline_pack_rectangle (stbi::rect_pack::context *context, int width, int height) {
  // find best position according to heuristic
  stbrp__findresult res = stbrp__skyline_find_best_pos (context, width, height);
  stbi::rect_pack::node *node, *cur;

  // bail if:
  //    1. it failed
  //    2. the best node doesn't fit (we don't always check this)
  //    3. we're out of memory
  if (res.prev_link == NULL || res.y + height > context->height || context->free_head == NULL) {
    res.prev_link = NULL;
    return res;
  }

  // on success, create new node
  node = context->free_head;
  node->x = (uint32_t)res.x;
  node->y = (uint32_t)(res.y + height);

  context->free_head = node->next;

  // insert the new node into the right starting point, and
  // let 'cur' point to the remaining nodes needing to be
  // stiched back in

  cur = *res.prev_link;
  if (cur->x < res.x) {
    // preserve the existing one, so start testing with the next one
    stbi::rect_pack::node *next = cur->next;
    cur->next = node;
    cur = next;
  } else {
    *res.prev_link = node;
  }

  // from here, traverse cur and free the nodes, until we get to one
  // that shouldn't be freed
  while (cur->next && cur->next->x <= res.x + width) {
    stbi::rect_pack::node *next = cur->next;
    // move the current node to the free list
    cur->next = context->free_head;
    context->free_head = cur;
    cur = next;
  }

  // stitch the list back in
  node->next = cur;

  if (cur->x < res.x + width)
    cur->x = (uint32_t)(res.x + width);

#ifdef _DEBUG
  cur = context->active_head;
  while (cur->x < context->width) {
    STBRP_ASSERT (cur->x < cur->next->x);
    cur = cur->next;
  }
  STBRP_ASSERT (cur->next == NULL);

  {
    int count = 0;
    cur = context->active_head;
    while (cur) {
      cur = cur->next;
      ++count;
    }
    cur = context->free_head;
    while (cur) {
      cur = cur->next;
      ++count;
    }
    STBRP_ASSERT (count == context->num_nodes + 2);
  }
#endif

  return res;
}

static int STBRP__CDECL rect_height_compare (const void *a, const void *b) {
  const stbi::rect_pack::rect *p = (const stbi::rect_pack::rect *)a;
  const stbi::rect_pack::rect *q = (const stbi::rect_pack::rect *)b;
  if (p->h > q->h)
    return -1;
  if (p->h < q->h)
    return 1;
  return (p->w > q->w) ? -1 : (p->w < q->w);
}

static int STBRP__CDECL rect_original_order (const void *a, const void *b) {
  const stbi::rect_pack::rect *p = (const stbi::rect_pack::rect *)a;
  const stbi::rect_pack::rect *q = (const stbi::rect_pack::rect *)b;
  return (p->was_packed < q->was_packed) ? -1 : (p->was_packed > q->was_packed);
}

int stbi::rect_pack::pack_rects (stbi::rect_pack::context *context, stbi::rect_pack::rect *rects, int num_rects) {
  int i, all_rects_packed = 1;

  // we use the 'was_packed' field internally to allow sorting/unsorting
  for (i = 0; i < num_rects; ++i) {
    rects[i].was_packed = i;
  }

  // sort according to heuristic
  STBRP_SORT (rects, num_rects, sizeof (rects[0]), rect_height_compare);

  for (i = 0; i < num_rects; ++i) {
    if (rects[i].w == 0 || rects[i].h == 0) {
      rects[i].x = rects[i].y = 0; // empty rect needs no space
    } else {
      stbrp__findresult fr = stbrp__skyline_pack_rectangle (context, rects[i].w, rects[i].h);
      if (fr.prev_link) {
        rects[i].x = (uint32_t)fr.x;
        rects[i].y = (uint32_t)fr.y;
      } else {
        rects[i].x = rects[i].y = STBRP__MAXVAL;
      }
    }
  }

  // unsort
  STBRP_SORT (rects, num_rects, sizeof (rects[0]), rect_original_order);

  // set was_packed flags and all_rects_packed status
  for (i = 0; i < num_rects; ++i) {
    rects[i].was_packed = !(rects[i].x == STBRP__MAXVAL && rects[i].y == STBRP__MAXVAL);
    if (!rects[i].was_packed)
      all_rects_packed = 0;
  }

  // return the all_rects_packed status
  return all_rects_packed;
}