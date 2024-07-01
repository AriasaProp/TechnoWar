#include "stbi_rectpack.hpp"

// BOTTOM-LEFT 0, BEST-FIT 1
#define HEURISTIC_SKYLINE 1

struct stbrp_node {
  unsigned int x, y;
  stbrp_node *next;
};

#include <algorithm>

#ifndef ASSERT
#include <cassert>
#define ASSERT assert
#endif

bool stbi::rectpack::pack_rects (const unsigned int c_width, const unsigned int c_height, stbi::rectpack::rect *rects, const size_t num_rects) {
  size_t i;

  // init context
  const size_t num_nodes = c_width + 15;
  stbrp_node nodes[num_nodes];
  stbrp_node *c_free_head = nodes;
  for (i = 0; i < num_nodes - 1; ++i)
    nodes[i].next = nodes + i + 1;
  nodes[i].next = NULL;

  stbrp_node extra_node[2];
  // node 0 is the full width, node 1 is the sentinel (lets us not store width explicitly)
  extra_node[0].x = 0;
  extra_node[0].y = 0;
  extra_node[0].next = extra_node + 1;
  extra_node[1].x = c_width;
  extra_node[1].y = c_height;
  extra_node[1].next = NULL;

  // we use the 'was_packed' field internally to allow sorting/unsorting
  for (i = 0; i < num_rects; ++i) {
    rects[i].was_packed = i;
  }

  // sort according to heuristic by it's size
  std::sort (rects, rects + num_rects, [] (const stbi::rectpack::rect &p, const stbi::rectpack::rect &q) -> bool {
    return p.w * p.h > q.w * q.h;
  });

  for (i = 0; i < num_rects; ++i) {
    stbi::rectpack::rect &rect = rects[i];
    rect.x = c_width;
    rect.y = c_height;
    // empty rect needs no space, rect size over bin skipped
    if (rect.w == 0 || rect.w >= c_width || rect.h == 0 || rect.h >= c_height) {
      continue;
    }
    {
      stbrp_node **res_prev_link = NULL;
      stbrp_node *free_head = extra_node;
      // find best position according to heuristic
      {
        unsigned int best_waste = c_height, best_x, best_y = c_height;

        // align to multiple of 2
        const unsigned int r_width = rect.w + (rect.w % 2);

        stbrp_node *node = free_head;
        stbrp_node **prev = &free_head;
        stbrp_node *tail;
        stbrp_node **best = NULL;
        while (node->x + r_width <= c_width) {
          unsigned int y = 0, waste = 0;
          
          {
  stbrp_node *_node = node;

  ASSERT (_node->next->x > node->x); // we ended up handling this in the caller for efficiency

  const unsigned int x1 = node->x + r_width;
  unsigned int visited_width = 0;
  while (_node->x < x1) {
    if (_node->y > y) {
      // raise y higher.
      // we've accounted for all waste up to y,
      // but we'll now add more waste for everything we've visted
      waste += visited_width * (_node->y - y);
      y = _node->y;
      // the first time through, visited_width might be reduced
      if (_node->x < node->x)
        visited_width += _node->next->x - node->x;
      else
        visited_width += _node->next->x - _node->x;
    } else {
      // add waste area
      unsigned int under_width = _node->next->x - _node->x;
      if (under_width + visited_width > r_width)
        under_width = r_width - visited_width;
      waste += under_width * (y - _node->y);
      visited_width += under_width;
    }
    _node = _node->next;
  }
}

#if (HEURISTIC_SKYLINE == 0)
          // bottom left
          // actually just want to test BL
          if (y < best_y) {
            best_y = y;
            best = prev;
          }
#else
          // best-fit
          if (y + rect.h <= c_height) {
            // can only use it if it first vertically
            if (y < best_y || (y == best_y && waste < best_waste)) {
              best_y = y;
              best_waste = waste;
              best = prev;
            }
          }
#endif
          prev = &node->next;
          node = node->next;
        }

        best_x = (best == NULL) ? 0 : (*best)->x;

#if (HEURISTIC_SKYLINE == 1)
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
        tail = free_head;
        node = free_head;
        prev = &free_head;
        // find first node that's admissible
        while (tail->x < r_width)
          tail = tail->next;
        while (tail) {
          unsigned int xpos = tail->x - r_width;
          ASSERT (xpos >= 0);
          // find the left position that matches this
          while (node->next->x <= xpos) {
            prev = &node->next;
            node = node->next;
          }
          ASSERT (node->next->x > xpos && node->x <= xpos);
          unsigned int y = 0, waste = 0;
					{
					  stbrp_node *_node = node;
					
					  ASSERT (_node->next->x > xpos); // we ended up handling this in the caller for efficiency
					  ASSERT (_node->x <= xpos);
					
					  const unsigned int x1 = xpos + r_width;
					  unsigned int visited_width = 0;
					  while (_node->x < x1) {
					    if (_node->y > y) {
					      // raise y higher.
					      // we've accounted for all waste up to y,
					      // but we'll now add more waste for everything we've visted
					      waste += visited_width * (_node->y - y);
					      y = _node->y;
					      // the first time through, visited_width might be reduced
					      if (_node->x < xpos)
					        visited_width += _node->next->x - xpos;
					      else
					        visited_width += _node->next->x - _node->x;
					    } else {
					      // add waste area
					      unsigned int under_width = _node->next->x - _node->x;
					      if (under_width + visited_width > r_width)
					        under_width = r_width - visited_width;
					      waste += under_width * (y - _node->y);
					      visited_width += under_width;
					    }
					    _node = _node->next;
					  }
					}

          if (y + rect.h <= c_height) {
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
#endif

        res_prev_link = best;
        rect.x = best_x;
        rect.y = best_y;
      }
      // bail if:
      //    1. it failed
      //    2. the best node doesn't fit (we don't always check this)
      //    3. we're out of memory
      if (res_prev_link == NULL || rect.y + rect.h > c_height || c_free_head == NULL)
        continue;
      {
        stbrp_node *node, *cur;

        // on success, create new node
        node = c_free_head;
        node->x = rect.x;
        node->y = rect.y + rect.h;

        c_free_head = node->next;

        // insert the new node into the right starting point, and
        // let 'cur' point to the remaining nodes needing to be
        // stiched back in

        cur = *res_prev_link;
        if (cur->x < rect.x) {
          // preserve the existing one, so start testing with the next one
          stbrp_node *next = cur->next;
          cur->next = node;
          cur = next;
        } else {
          *res_prev_link = node;
        }

        // from here, traverse cur and free the nodes, until we get to one
        // that shouldn't be freed
        while (cur->next && cur->next->x <= rect.x + rect.w) {
          stbrp_node *next = cur->next;
          // move the current node to the free list
          cur->next = c_free_head;
          c_free_head = cur;
          cur = next;
        }

        // stitch the list back in
        node->next = cur;

        if (cur->x < rect.x + rect.w)
          cur->x = (rect.x + rect.w);

#ifdef _DEBUG
        cur = free_head;
        while (cur->x < c_width) {
          ASSERT (cur->x < cur->next->x);
          cur = cur->next;
        }
        ASSERT (cur->next == NULL);
        {
          unsigned int count = 0;
          cur = free_head;
          while (cur) {
            cur = cur->next;
            ++count;
          }
          cur = c_free_head;
          while (cur) {
            cur = cur->next;
            ++count;
          }
          ASSERT (count == num_nodes + 2);
        }
#endif
      }
    }
  }

  // unsort 0, 1 ,2 ....
  std::sort (rects, rects + num_rects, [] (const stbi::rectpack::rect &p, const stbi::rectpack::rect &q) -> bool {
    return p.was_packed < q.was_packed;
  });

  bool all_rects_packed = true;
  // set was_packed flags and all_rects_packed status
  for (i = 0; i < num_rects; ++i) {
    rects[i].was_packed = ((rects[i].x + rects[i].w) <= c_width) && ((rects[i].y + rects[i].h) <= c_height);
    all_rects_packed &= rects[i].was_packed;
  }

  // return the all_rects_packed status
  return all_rects_packed;
}
