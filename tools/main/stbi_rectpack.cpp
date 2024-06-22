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

// this is the maximum supported coordinate value.

enum stbrp__heuristic {
  skylineBL_sortHeight,
  skylineBF_sortHeight
};
struct stbrp__node {
  unsigned int x, y;
  stbrp__node *next;
};
struct stbrp__context {
  stbrp__heuristic hr = stbrp__heuristic::skylineBL_sortHeight;
  stbrp__node *nodes;
  stbrp__node *free_head;
  stbrp__node *active_head;
  stbrp__node extra[2];
};

// find minimum y position if it starts at x1
static int stbrp__skyline_find_min_y (stbrp__context *c, stbrp__node *first, unsigned int x0, unsigned int width, unsigned int *pwaste) {
  stbrp__node *node = first;
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
  stbrp__node **prev_link;
};

bool stbi::rectpack::pack_rects (const unsigned int width, const unsigned int height, stbi::rectpack::rect *rects, unsigned int num_rects) {
  size_t i;

  // we use the 'was_packed' field internally to allow sorting/unsorting
  for (i = 0; i < num_rects; ++i) {
    rects[i].was_packed = i;
  }

  // sort according to heuristic
  std::sort (rects, rects + num_rects, [] (const rect &p, const rect &q) {
    return (p.w * p.h) > (q.w * q.h);
  });
  {
    stbrp__context context;

    // Initialize a rectangle packer to:
    //    pack a rectangle that is 'width' by 'height' in dimensions
    //
    stbrp__node nodes[width + 25];
    do {
      p
          nodes[i]
              .next = nodes + i + 1;
    } while (++i < width + 25);
    nodes[i].next = NULL;
    context.free_head = nodes;
    context.active_head = context.extra;

    // node 0 is the full width, node 1 is the sentinel (lets us not store width explicitly)
    context.extra[0].x = 0;
    context.extra[0].y = 0;
    context.extra[0].next = &context.extra[1];
    context.extra[1].x = width;
    context.extra[1].y = (1 << 30);
    context.extra[1].next = NULL;

    for (i = 0; i < num_rects; ++i) {
      stbi::rectpack::rect &rect = rects[i];
      // empty rect needs no space
      if (rect.w == 0 || rect.h == 0) {
        rect.x = rect.y = 0;
        continue;
      }
      // rect that bigger than rect bin skipped
      if (rect.w >= width || rect.h >= height) {
        rect.x = width;
        rect.y = height;
        continue;
      }

      // pack rect
      // find best position according to heuristic
      stbrp__findresult fr;
      {
        // align to multiple of 2
        unsigned int r_width = rect.w + (rect.w % 2);

        // if it can't possibly fit, bail immediately
        unsigned int best_waste = (1 << 30), best_x, best_y = (1 << 30);
        stbrp__node **prev = &context.active_head;
        stbrp__node *node = context.active_head;
        stbrp__node *tail;
        stbrp__node **best = NULL;

        while (node->x + r_width <= width) {
          unsigned int waste;
          unsigned int y = stbrp__skyline_find_min_y (c, node, node->x, r_width, &waste);
          if (context.hr == stbrp__heuristic::skylineBL_sortHeight) { // actually just want to test BL
            // bottom left
            if (y < best_y) {
              best_y = y;
              best = prev;
            }
          } else {
            // best-fit
            if (y + rect.h <= height) {
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
        if (context.hr == stbrp__heuristic::skylineBF_sortHeight) {
          tail = context.active_head;
          node = context.active_head;
          prev = &context.active_head;
          // find first node that's admissible
          while (tail->x < r_width)
            tail = tail->next;
          while (tail) {
            unsigned int xpos = tail->x - r_width;
            unsigned int y, waste;
            ASSERT (xpos >= 0);
            // find the left position that matches this
            while (node->next->x <= xpos) {
              prev = &node->next;
              node = node->next;
            }
            ASSERT (node->next->x > xpos && node->x <= xpos);
            y = stbrp__skyline_find_min_y (c, node, xpos, r_width, &waste);
            if (y + rect.h <= height) {
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
      }
      /* bail if:
       *   1. it failed
       *   2. the best node doesn't fit (we don't always check this)
       *   3. we're out of memory
       */
      if (fr.prev_link == NULL || fr.y + rect.h > height || context.free_head == NULL) {
        rect.x = width;
        rect.y = height;
      } else {
        stbrp__node *node, *cur;
        // on success, create new node
        node = context.free_head;
        node->x = fr.x;
        node->y = fr.y + rect.h;
        context.free_head = node->next;

        // insert the new node into the right starting point, and
        // let 'cur' point to the remaining nodes needing to be
        // stiched back in
        cur = *fr.prev_link;
        if (cur->x < fr.x) {
          // preserve the existing one, so start testing with the next one
          stbrp__node *next = cur->next;
          cur->next = node;
          cur = next;
        } else {
          *fr.prev_link = node;
        }

        // from here, traverse cur and free the nodes, until we get to one
        // that shouldn't be freed
        while (cur->next && cur->next->x <= fr.x + r_width) {
          stbrp__node *next = cur->next;
          // move the current node to the free list
          cur->next = context.free_head;
          context.free_head = cur;
          cur = next;
        }

        // stitch the list back in
        node->next = cur;

        if (cur->x < fr.x + r_width)
          cur->x = fr.x + r_width;

#ifdef _DEBUG
        cur = context.active_head;
        while (cur->x < width) {
          ASSERT (cur->x < cur->next->x);
          cur = cur->next;
        }
        ASSERT (cur->next == NULL);

        {
          size_t count = 0;
          cur = context.active_head;
          while (cur) {
            cur = cur->next;
            ++count;
          }
          cur = context.free_head;
          while (cur) {
            cur = cur->next;
            ++count;
          }
          ASSERT (count == width + 2);
        }
#endif
        rect.x = fr.x;
        rect.y = fr.y;
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
    int &state = rects[i].was_packed;
    state = (rects[i].x >= width || rects[i].y >= height) ? 0 : 1;
    if (!state) {
      all_rects_packed &= false;
    }
  }

  // return the all_rects_packed status
  return all_rects_packed;
}
