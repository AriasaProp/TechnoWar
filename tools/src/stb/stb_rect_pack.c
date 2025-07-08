#include "stb/stb_rect_pack.h"
#include "util.h"

enum {
   STBRP_HEURISTIC_Skyline_BL_sortHeight = 0,
   STBRP_HEURISTIC_Skyline_BF_sortHeight
} global_heuristic = STBRP_HEURISTIC_Skyline_BL_sortHeight;

struct stbrp_node {
   stbrp_coord  x,y;
  struct stbrp_node  *next;
};

struct stbrp_context {
   int width;
   int height;
   int align;
   int init_mode;
   int heuristic;
   int num_nodes;
  struct stbrp_node *active_head;
  struct stbrp_node *free_head;
  struct stbrp_node extra[2]; // we allocate two extra nodes so optimal user-node-count is 'width' not 'width+2'
};

// find minimum y position if it starts at x1
static int stbrp__skyline_find_min_y(struct stbrp_node *first, int x0, int width, int *pwaste) {
  struct stbrp_node *node = first;
   int x1 = x0 + width;
   int min_y, visited_width, waste_area;

   ASSERT(first->x <= x0);

   #if 0
   // skip in case we're past the node
   while (node->next->x <= x0)
      ++node;
   #else
   ASSERT(node->next->x > x0); // we ended up handling this in the caller for efficiency
   #endif

   ASSERT(node->x <= x0);

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

typedef struct {
   int x,y;
  struct stbrp_node **prev_link;
} stbrp__findresult;

static stbrp__findresult stbrp__skyline_find_best_pos(struct stbrp_context *c, int width, int height) {
   int best_waste = (1<<30), best_x, best_y = (1 << 30);
   stbrp__findresult fr;
  struct stbrp_node **prev, *node, *tail, **best = NULL;

   // align to multiple of c->align
   width = (width + c->align - 1);
   width -= width % c->align;
   ASSERT(width % c->align == 0);

   // if it can't possibly fit, bail immediately
   if (width > c->width || height > c->height) {
      fr.prev_link = NULL;
      fr.x = fr.y = 0;
      return fr;
   }

   node = c->active_head;
   prev = &c->active_head;
   while (node->x + width <= c->width) {
      int y,waste;
      y = stbrp__skyline_find_min_y(node, node->x, width, &waste);
      if (global_heuristic == STBRP_HEURISTIC_Skyline_BL_sortHeight) { // actually just want to test BL
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

   if (global_heuristic == STBRP_HEURISTIC_Skyline_BF_sortHeight) {
      tail = c->active_head;
      node = c->active_head;
      prev = &c->active_head;
      // find first node that's admissible
      while (tail->x < width)
         tail = tail->next;
      while (tail) {
         int xpos = tail->x - width;
         int y,waste;
         ASSERT(xpos >= 0);
         // find the left position that matches this
         while (node->next->x <= xpos) {
            prev = &node->next;
            node = node->next;
         }
         ASSERT(node->next->x > xpos && node->x <= xpos);
         y = stbrp__skyline_find_min_y(node, xpos, width, &waste);
         if (y + height <= c->height) {
            if (y <= best_y) {
               if (y < best_y || waste < best_waste || (waste==best_waste && xpos < best_x)) {
                  best_x = xpos;
                  ASSERT(y <= best_y);
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

static stbrp__findresult stbrp__skyline_pack_rectangle(struct stbrp_context *context, int width, int height) {
   // find best position according to heuristic
   stbrp__findresult res = stbrp__skyline_find_best_pos(context, width, height);
  struct stbrp_node *node, *cur;

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
   node->x = (stbrp_coord) res.x;
   node->y = (stbrp_coord) (res.y + height);

   context->free_head = node->next;

   // insert the new node into the right starting point, and
   // let 'cur' point to the remaining nodes needing to be
   // stiched back in

   cur = *res.prev_link;
   if (cur->x < res.x) {
      // preserve the existing one, so start testing with the next one
     struct stbrp_node *next = cur->next;
      cur->next = node;
      cur = next;
   } else {
      *res.prev_link = node;
   }

   // from here, traverse cur and free the nodes, until we get to one
   // that shouldn't be freed
   while (cur->next && cur->next->x <= res.x + width) {
     struct stbrp_node *next = cur->next;
      // move the current node to the free list
      cur->next = context->free_head;
      context->free_head = cur;
      cur = next;
   }

   // stitch the list back in
   node->next = cur;

   if (cur->x < res.x + width)
      cur->x = (stbrp_coord) (res.x + width);

#ifdef _DEBUG
   cur = context->active_head;
   while (cur->x < context->width) {
      ASSERT(cur->x < cur->next->x);
      cur = cur->next;
   }
   ASSERT(cur->next == NULL);

   {
      int count=0;
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
      ASSERT(count == context->num_nodes+2);
   }
#endif

   return res;
}

static int CDECL rect_height_compare(const void *a, const void *b) {
   const struct stbrp_rect *p = (const struct stbrp_rect *) a;
   const struct stbrp_rect *q = (const struct stbrp_rect *) b;
   if (p->h > q->h)
      return -1;
   if (p->h < q->h)
      return  1;
   return (p->w > q->w) ? -1 : (p->w < q->w);
}

static int CDECL rect_original_order(const void *a, const void *b) {
   const struct stbrp_rect *p = (const struct stbrp_rect *) a;
   const struct stbrp_rect *q = (const struct stbrp_rect *) b;
   return (p->id < q->id) ? -1 : (p->id > q->id);
}

int stbrp_pack_rects(struct stbrp_rect *rects, int num_rects, int width, int height) {
   int i, all_rects_packed = 1;
   // sort according to heuristic
   qsort(rects, num_rects, sizeof(rects[0]), rect_height_compare);
   struct stbrp_context context;
   struct stbrp_node *nodes = (struct stbrp_node*)calloc(width, sizeof(struct stbrp_node));
   { // init context
	   int i;
	   
	   for (i=0; i < width-1; ++i)
	      nodes[i].next = &nodes[i+1];
	   nodes[i].next = NULL;
	   context.free_head = &nodes[0];
	   context.active_head = &context.extra[0];
	   context.width = width;
	   context.height = height;
	   context.num_nodes = width;
	   context.align = 1;
	
	   // node 0 is the full width, node 1 is the sentinel (lets us not store width explicitly)
	   context.extra[0].x = 0;
	   context.extra[0].y = 0;
	   context.extra[0].next = &context.extra[1];
	   context.extra[1].x = (stbrp_coord) width;
	   context.extra[1].y = (1<<30);
	   context.extra[1].next = NULL;
   }

   for (i=0; i < num_rects; ++i) {
      if (rects[i].w == 0 || rects[i].h == 0) {
         rects[i].x = rects[i].y = 0;  // empty rect needs no space
      } else {
         stbrp__findresult fr = stbrp__skyline_pack_rectangle(&context, rects[i].w, rects[i].h);
         if (fr.prev_link) {
            rects[i].x = (stbrp_coord) fr.x;
            rects[i].y = (stbrp_coord) fr.y;
         } else {
            rects[i].x = rects[i].y = STBRP__MAXVAL;
         }
      }
   }

   // unsort
   qsort(rects, num_rects, sizeof(rects[0]), rect_original_order);

   for (i=0; i < num_rects; ++i) {
      if (rects[i].x == STBRP__MAXVAL && rects[i].y == STBRP__MAXVAL) {
      	rects[i].w = rects[i].h = 0;
        all_rects_packed = 0;
      }
   }
   free(nodes);
   // return the all_rects_packed status
   return all_rects_packed;
}
