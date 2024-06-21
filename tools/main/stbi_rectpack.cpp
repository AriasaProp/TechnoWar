#include "stbi_rectpack.hpp"

#include <algorithm>
#include <cstdlib>

#ifndef ASSERT
#include <cassert>
#define ASSERT assert
#endif

#ifdef _MSC_VER
#define NO_USE(v) (void)(v)
#else
#define NO_USE(v) (void)sizeof (v)
#endif

#define STBRP__MAXVAL 0x7fffffff
// this is the maximum supported coordinate value.

stbi::rectpack::context::context (unsigned int w, unsigned int h) : width (w), height (h) {
  size_t i = 0;
  nodes = new stbi::rectpack::node[width];
  do {
    nodes[i].next = nodes + i + 1;
  } while (++i < width);
  nodes[i].next = NULL;
  free_head = nodes;
  active_head = extra;

  // node 0 is the full width, node 1 is the sentinel (lets us not store width explicitly)
  extra[0].x = 0;
  extra[0].y = 0;
  extra[0].next = &extra[1];
  extra[1].x = width;
  extra[1].y = (1 << 30);
  extra[1].next = NULL;
}

stbi::rectpack::context::~context () {
  delete[] nodes;
}

// find minimum y position if it starts at x1
static int stbrp__skyline_find_min_y (stbi::rectpack::context *c, stbi::rectpack::node *first, unsigned int x0, unsigned int width, unsigned int *pwaste) {
  stbi::rectpack::node *node = first;
  unsigned int x1 = x0 + width;
  unsigned int min_y, visited_width, waste_area;

  NO_USE (c);

  ASSERT (first->x <= x0);

#if 0
  // skip in case we're past the node
  while (node->next->x <= x0)
    ++node;
#else
  ASSERT (node->next->x > x0); // we ended up handling this in the caller for efficiency
#endif

  ASSERT (node->x <= x0);

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
      unsigned int under_width = node->next->x - node->x;
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

struct stbrp__findresult {
  unsigned int x, y;
  stbi::rectpack::node **prev_link;
};

static stbrp__findresult stbrp__skyline_find_best_pos (stbi::rectpack::context *c, int width, int height) {
  unsigned int best_waste = (1 << 30), best_x, best_y = (1 << 30);
  stbrp__findresult fr;
  stbi::rectpack::node **prev, *node, *tail, **best = NULL;

  // align to multiple of 2
  width = (width + 2 - 1);
  width -= width % 2;
  ASSERT (width % 2 == 0);

  // if it can't possibly fit, bail immediately
  if (width > c->width || height > c->height) {
    fr.prev_link = NULL;
    fr.x = fr.y = 0;
    return fr;
  }

  node = c->active_head;
  prev = &c->active_head;
  while (node->x + width <= c->width) {
    unsigned int y, waste;
    y = stbrp__skyline_find_min_y (c, node, node->x, width, &waste);
    if (c->hr == stbi::rectpack::heuristic::skylineBL_sortHeight) { // actually just want to test BL
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
  //    ____________________
  //   |____________________|
  //
  //        into
  //
  //  |                 |
  //  |         ____________|
  //  |____________|
  //
  // then right-aligned reduces waste, but bottom-left BL is always chooses left-aligned
  //
  // This makes BF take about 2x the time

  if (c->hr == stbi::rectpack::heuristic::skylineBF_sortHeight) {
    tail = c->active_head;
    node = c->active_head;
    prev = &c->active_head;
    // find first node that's admissible
    while (tail->x < width)
      tail = tail->next;
    while (tail) {
      unsigned int xpos = tail->x - width;
      unsigned int y, waste;
      ASSERT (xpos >= 0);
      // find the left position that matches this
      while (node->next->x <= xpos) {
        prev = &node->next;
        node = node->next;
      }
      ASSERT (node->next->x > xpos && node->x <= xpos);
      y = stbrp__skyline_find_min_y (c, node, xpos, width, &waste);
      if (y + height <= c->height) {
        if (y <= best_y) {
          if (y < best_y || waste < best_waste || (waste == best_waste && xpos < best_x)) {
            best_x = xpos;
            ASSERT (y <= best_y);
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

bool stbi::rectpack::pack_rects (context *context, rect *rects, unsigned int num_rects) {
  size_t i;

  // we use the 'was_packed' field internally to allow sorting/unsorting
  for (i = 0; i < num_rects; ++i) {
    rects[i].was_packed = i;
  }

  // sort according to heuristic
  std::sort (rects, rects + num_rects, [] (const rect &p, const rect &q) {
    return (p.w * p.h) > (q.w * q.h);
  });

  for (i = 0; i < num_rects; ++i) {
    if (rects[i].w == 0 || rects[i].h == 0) {
      rects[i].x = rects[i].y = 0; // empty rect needs no space
    } else {
      // pack rect
      // find best position according to heuristic
      stbrp__findresult fr = stbrp__skyline_find_best_pos (context, rects[i].w, rects[i].h);
      /* bail if:
       *   1. it failed
       *   2. the best node doesn't fit (we don't always check this)
       *   3. we're out of memory
       */
      if (fr.prev_link == NULL || fr.y + rects[i].h > context->height || context->free_head == NULL) {
        rects[i].x = rects[i].y = STBRP__MAXVAL;
      } else {
        stbi::rectpack::node *node, *cur;
        // on success, create new node
        node = context->free_head;
        node->x = fr.x;
        node->y = fr.y + rects[i].h;
        context->free_head = node->next;

        // insert the new node into the right starting point, and
        // let 'cur' point to the remaining nodes needing to be
        // stiched back in
        cur = *fr.prev_link;
        if (cur->x < fr.x) {
          // preserve the existing one, so start testing with the next one
          stbi::rectpack::node *next = cur->next;
          cur->next = node;
          cur = next;
        } else {
          *fr.prev_link = node;
        }

        // from here, traverse cur and free the nodes, until we get to one
        // that shouldn't be freed
        while (cur->next && cur->next->x <= fr.x + rects[i].w) {
          stbi::rectpack::node *next = cur->next;
          // move the current node to the free list
          cur->next = context->free_head;
          context->free_head = cur;
          cur = next;
        }

        // stitch the list back in
        node->next = cur;

        if (cur->x < fr.x + rects[i].w)
          cur->x = fr.x + rects[i].w;

#ifdef _DEBUG
        cur = context->active_head;
        while (cur->x < context->width) {
          ASSERT (cur->x < cur->next->x);
          cur = cur->next;
        }
        ASSERT (cur->next == NULL);

        {
          size_t count = 0;
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
          ASSERT (count == context->width + 2);
        }
#endif
        rects[i].x = fr.x;
        rects[i].y = fr.y;
      }
    }
  }

  // unsort
  std::sort (rects, rects + num_rects, [] (const rect &p, const rect &q) {
    return p.was_packed < q.was_packed;
  });

  // set was_packed flags and all_rects_packed status
  bool all_rects_packed = true;
  for (i = 0; i < num_rects; ++i) {
    rects[i].was_packed = !(rects[i].x == STBRP__MAXVAL && rects[i].y == STBRP__MAXVAL);
    if (!rects[i].was_packed) {
      all_rects_packed &= false;
    }
  }

  // return the all_rects_packed status
  return all_rects_packed;
}
