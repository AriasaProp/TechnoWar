#include "stbi_rectpack.hpp"

enum {
  STBRP_HEURISTIC_Skyline_default = 0,
  STBRP_HEURISTIC_Skyline_BL_sortHeight = STBRP_HEURISTIC_Skyline_default,
  STBRP_HEURISTIC_Skyline_BF_sortHeight
};

//////////////////////////////////////////////////////////////////////////////
//
// the details of the following structures don't matter to you, but they must
// be visible so you can handle the memory allocations for them

struct stbrp_node {
  int x, y;
  stbrp_node *next;
};

#include <cstdint>
#include <cstdlib>
#include <algorithm>

#ifndef ASSERT
#include <cassert>
#define ASSERT assert
#endif

struct stbrp__context {
  int width;
  int height;
  int heuristic;
  int num_nodes;
  stbrp_node *active_head;
  stbrp_node *free_head;
  stbrp_node extra[2]; // we allocate two extra nodes so optimal user-node-count is 'width' not 'width+2'
};

// find minimum y position if it starts at x1
static int stbrp__skyline_find_min_y (stbrp_node *first, int x0, int width, int *pwaste) {
  stbrp_node *node = first;
  int x1 = x0 + width;
  int min_y, visited_width, waste_area;

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

struct stbrp__findresult {
  int x, y;
  stbrp_node **prev_link;
};

static stbrp__findresult stbrp__skyline_find_best_pos (stbrp__context &c, int width, int height) {
  int best_waste = (1 << 30), best_x, best_y = (1 << 30);
  stbrp__findresult fr;
  stbrp_node **prev, *node, *tail, **best = NULL;

  // align to multiple of 2
  width += width % 2;

  // if it can't possibly fit, bail immediately
  if (width > c.width || height > c.height) {
    fr.prev_link = NULL;
    fr.x = fr.y = 0;
    return fr;
  }

  node = c.active_head;
  prev = &c.active_head;
  while (node->x + width <= c.width) {
    int y, waste;
    y = stbrp__skyline_find_min_y (node, node->x, width, &waste);
    if (c.heuristic == STBRP_HEURISTIC_Skyline_BL_sortHeight) { // actually just want to test BL
      // bottom left
      if (y < best_y) {
        best_y = y;
        best = prev;
      }
    } else {
      // best-fit
      if (y + height <= c.height) {
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

  if (c.heuristic == STBRP_HEURISTIC_Skyline_BF_sortHeight) {
    tail = c.active_head;
    node = c.active_head;
    prev = &c.active_head;
    // find first node that's admissible
    while (tail->x < width)
      tail = tail->next;
    while (tail) {
      int xpos = tail->x - width;
      int y, waste;
      ASSERT (xpos >= 0);
      // find the left position that matches this
      while (node->next->x <= xpos) {
        prev = &node->next;
        node = node->next;
      }
      ASSERT (node->next->x > xpos && node->x <= xpos);
      y = stbrp__skyline_find_min_y (node, xpos, width, &waste);
      if (y + height <= c.height) {
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

static stbrp__findresult stbrp__skyline_pack_rectangle (stbrp__context &context, int width, int height) {
  // find best position according to heuristic
  stbrp__findresult res = stbrp__skyline_find_best_pos (context, width, height);
  stbrp_node *node, *cur;

  // bail if:
  //    1. it failed
  //    2. the best node doesn't fit (we don't always check this)
  //    3. we're out of memory
  if (res.prev_link == NULL || res.y + height > context.height || context.free_head == NULL) {
    res.prev_link = NULL;
    return res;
  }

  // on success, create new node
  node = context.free_head;
  node->x = (int)res.x;
  node->y = (int)(res.y + height);

  context.free_head = node->next;

  // insert the new node into the right starting point, and
  // let 'cur' point to the remaining nodes needing to be
  // stiched back in

  cur = *res.prev_link;
  if (cur->x < res.x) {
    // preserve the existing one, so start testing with the next one
    stbrp_node *next = cur->next;
    cur->next = node;
    cur = next;
  } else {
    *res.prev_link = node;
  }

  // from here, traverse cur and free the nodes, until we get to one
  // that shouldn't be freed
  while (cur->next && cur->next->x <= res.x + width) {
    stbrp_node *next = cur->next;
    // move the current node to the free list
    cur->next = context.free_head;
    context.free_head = cur;
    cur = next;
  }

  // stitch the list back in
  node->next = cur;

  if (cur->x < res.x + width)
    cur->x = (int)(res.x + width);

#ifdef _DEBUG
  cur = context.active_head;
  while (cur->x < context.width) {
    ASSERT (cur->x < cur->next->x);
    cur = cur->next;
  }
  ASSERT (cur->next == NULL);

  {
    int count = 0;
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
    ASSERT (count == context.num_nodes + 2);
  }
#endif

  return res;
}

bool stbi::rectpack::pack_rects (int c_width, int c_height, stbi::rectpack::rect *rects, int num_rects) {
  size_t i;

  stbrp__context context;
  // init context
  int num_nodes = c_width + 15;
  stbrp_node nodes[num_nodes];

  for (i = 0; i < num_nodes - 1; ++i)
    nodes[i].next = &nodes[i + 1];
  nodes[i].next = NULL;

  context.heuristic = STBRP_HEURISTIC_Skyline_default;
  context.free_head = &nodes[0];
  context.active_head = &context.extra[0];
  context.width = c_width;
  context.height = c_height;
  context.num_nodes = num_nodes;

  // node 0 is the full width, node 1 is the sentinel (lets us not store width explicitly)
  context.extra[0].x = 0;
  context.extra[0].y = 0;
  context.extra[0].next = &context.extra[1];
  context.extra[1].x = c_width;
  context.extra[1].y = (1 << 30);
  context.extra[1].next = NULL;
  

  // we use the 'was_packed' field internally to allow sorting/unsorting
  for (i = 0; i < num_rects; ++i) {
    rects[i].was_packed = i;
  }

  // sort according to heuristic
  std::sort (rects, rects+num_rects, [] (const stbi::rectpack::rect &p, const stbi::rectpack::rect &q) -> bool {
    if (p.h != q.h)
      return p.h > q.h;
    return p.w > q.w;
  });

  for (i = 0; i < num_rects; ++i) {
  	stbi::rectpack::rect &rect = rects[i];
    if (rect.w == 0 || rect.w >= c_width || rect.h == 0 || rect.h >= c_height) {
      // empty rect needs no space, rect size over bin skipped
      rect.x = c_width;
      rect.y = c_height;
    } else {
      stbrp__findresult fr = stbrp__skyline_pack_rectangle (context, rect.w, rect.h);
      if (fr.prev_link) {
        rect.x = fr.x;
        rect.y = fr.y;
      } else {
        rect.x = c_width;
        rect.y = c_height;
      }
    }
  }

  // unsort 0, 1 ,2 ....
  std::sort (rects, rects+num_rects, [] (const stbi::rectpack::rect &p, const stbi::rectpack::rect &q) -> bool {
    return p.was_packed < q.was_packed;
  });

  bool all_rects_packed = true;
  // set was_packed flags and all_rects_packed status
  for (i = 0; i < num_rects; ++i) {
    rects[i].was_packed = ((rects[i].x + rects[i].w) <= c_width) && ((rects[i].y + rects[i].h) <= c_height);
    all_rects_packed &= rects[i].was_packed;
    
    if (!rects[i].was_packed)
  }

  // return the all_rects_packed status
  return all_rects_packed;
}
