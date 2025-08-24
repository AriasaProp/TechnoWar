#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "math_util.h"
#include "stb/stb_image.h"

#define UISTAGE_IMPLEMENTATION
#include "uistage.h"
#define UISTAGE_MAX_ACTORS 128

typedef enum {
  ACTOR_INVALID = 0,
  ACTOR_TEXT,
} _actor_type;
typedef struct {
  vec2 uv, uvm;
  vec2 size, off;
  float xadv;
} character;
typedef struct {
  char a, b;
  float d;
} kerning;
typedef struct {
  _actor_type type;
  vec2 origin;
  struct {
    unsigned int screen : 4;
    unsigned int origin : 4;
  } pivot;
  union {
    struct {
      unsigned int align : 4;
      size_t length;
      char *text;
    } label;
  } d;
} _actor;
static struct {
  struct {
    texture bitmap;
    vec2 bitmap_size;
    float size, lineHeight, base;
    character chs[0x80];
    size_t kerning_length;
    kerning *kearns;
  } font;
  _actor actors[UISTAGE_MAX_ACTORS + 2];
  flat_vertex vertex_buffer[MAX_UI_DRAW];
} src;

actor create_text(size_t strl) {
  actor i;
  for (i = 0; i < UISTAGE_MAX_ACTORS; ++i) {
    if (src.actors[i].type != ACTOR_INVALID)
      continue;
    src.actors[i].type = ACTOR_TEXT;
    src.actors[i].d.label.length = strl;
    src.actors[i].d.label.text = (char *)malloc(strl);
    break;
  }
  return i;
}
actor destroy_actor(actor a) {
  switch (src.actors[a].type) {
  case ACTOR_TEXT:
    free(src.actors[a].d.label.text);
    break;
  default:
    break;
  }
  memmove(src.actors + a, src.actors + a + 1, sizeof(_actor) * (UISTAGE_MAX_ACTORS - a + 1));
  src.actors[a].type = ACTOR_INVALID;
}

void set_actor_origin(actor a, const vec2 v) {
  src.actors[a].origin = v;
}
void set_actor_pivot(actor a, const pivot_state screen, const pivot_state origin) {
  src.actors[a].pivot.screen = screen;
  src.actors[a].pivot.origin = origin;
}
void set_text_str(actor a, const char *t, ...) {
  if (src.actors[a].type != ACTOR_TEXT)
    return;
  va_list args;
  va_start(args, t);
  vsnprintf(src.actors[a].d.label.text, src.actors[a].d.label.length, t, args);
  va_end(args);
}

static int assetRead_clbk(void *a, char *b, int l) {
  return global_engine.assetRead(a, (void *)b, (size_t)l);
}
static void assetSeek_clbk(void *a, int l) {
  global_engine.assetSeek(a, l);
}
static int assetEOF_clbk(void *a) {
  return global_engine.assetLength(a) == 0;
}
void uistage_init() {
  memset(&src, 0, sizeof(src));
  // default usage
  {
    int tempi[9];
    // load font image
    stbi_io_callbacks cf = {
      .read = assetRead_clbk,
      .skip = assetSeek_clbk,
      .eof = assetEOF_clbk,
    };
    void *ast = global_engine.openAsset("fonts/default/default.png");
    void *img = stbi_load_from_callbacks(&cf, ast, tempi, tempi + 1, tempi + 2, 4);
    src.font.bitmap = global_engine.genTexture((uivec2){(uint16_t)tempi[0], (uint16_t)tempi[1]}, img);
    vec2 textureSize = vec2_inv((vec2){(float)tempi[0], (float)tempi[1]});
    global_engine.assetClose(ast);

    // load font sets
    size_t i, l;
    char *line;
    ast = global_engine.assetBuffer("fonts/default/default.fnt", (const void **)&line, &l);
    line = strtok(line, "\n");
    static const float scaleup = 2.1f;
    do {
      if (strstr(line, "info")) {
        char tname[64], tchar[64];
        sscanf(line, "info face=%s size=%d bold=%d italic=%d charset=%s unicode=%d stretchH=%d smooth=%d aa=%d",
               tname, tempi, tempi + 1, tempi + 2, tchar, tempi + 3, tempi + 4, tempi + 5, tempi + 6);
        src.font.size = (float)tempi[0] * scaleup;
      } else if (strstr(line, "common")) {
        sscanf(line, "common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=%d packed=%d",
               tempi, tempi + 1, tempi + 2, tempi + 3, tempi + 4, tempi + 5);
        src.font.lineHeight = (float)tempi[0] * scaleup;
        src.font.base = (float)tempi[1] * scaleup;
      } else if (strstr(line, "chars ")) {
        sscanf(line, "chars count=%d", tempi + 8);
        for (i = 0; i < tempi[8]; ++i) {
          line = strtok(NULL, "\n");
          sscanf(line, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d",
                 tempi, tempi + 1, tempi + 2, tempi + 3, tempi + 4, tempi + 5, tempi + 6, tempi + 7);
          character A;
          A.uv = vec2_mul((vec2){(float)tempi[1], (float)tempi[2]}, textureSize);
          A.size = (vec2){(float)tempi[3], (float)tempi[4]};
          A.uvm = vec2_add(A.uv, vec2_mul(A.size, textureSize));
          A.off = (vec2){(float)tempi[5], -(float)tempi[6] /* system 2d coordinate fliped upside-down */};
          vec2_sclf(&A.size, scaleup);
          vec2_sclf(&A.off, scaleup);
          A.off.y -= A.size.y;
          A.xadv = (float)tempi[7] * scaleup;
          src.font.chs[tempi[0]] = A;
        }
      } else if (strstr(line, "kernings ")) {
        sscanf(line, "kernings count=%zu", &src.font.kerning_length);
        src.font.kearns = (kerning *)malloc(sizeof(kerning) * src.font.kerning_length);
        for (i = 0; i < src.font.kerning_length; ++i) {
          line = strtok(NULL, "\n");
          sscanf(line, "kerning first=%d second=%d amount=%d", tempi, tempi + 1, tempi + 2);
          src.font.kearns[i].a = (char)tempi[0];
          src.font.kearns[i].b = (char)tempi[1];
          src.font.kearns[i].d = (float)tempi[2];
        }
      }
    } while ((line = strtok(NULL, "\n")));
    global_engine.assetClose(ast);
  }
}
void uistage_draw() {

  size_t v = 0;
  // draw images

  // draw fonts
  v = 0;
  vec2 screenSize = global_engine.getScreenSize();
  for (actor i = 0; i < UISTAGE_MAX_ACTORS; ++i) {
    _actor T = src.actors[i];
    switch (T.type) {
    case ACTOR_INVALID:
      i = UISTAGE_MAX_ACTORS;
      break;
    case ACTOR_TEXT: {
      vec2 vorig = T.origin;
      {
        // text rect
        vec2 maxSize = VEC2_ZERO;
        {
          float width = 0;
          for (char *t = T.d.label.text; *t; ++t) {
            if (*t == '\n') {
              maxSize.y += src.font.lineHeight;
              if (maxSize.x < width)
                maxSize.x = width;
              width = 0.0f;
            } else if (*t < 0x80) {
              width += src.font.chs[*t].xadv;
            }
          }
          // except last lineHeight
          if (maxSize.x < width)
            maxSize.x = width;
        }
        // screen pivot
        uint8_t ps = T.pivot.screen;
        switch (ps & PIVOT_HCENTER) {
        case PIVOT_HCENTER:
          vorig.x += screenSize.x * 0.5f;
          break;
        case PIVOT_RIGHT:
          vorig.x = screenSize.x - vorig.x;
          break;
        default:
          break;
        }
        switch (ps & PIVOT_VCENTER) {
        case PIVOT_VCENTER:
          vorig.y += screenSize.y * 0.5f;
          break;
        case PIVOT_TOP:
          vorig.y = screenSize.y - vorig.y;
          break;
        default:
          break;
        }
        // origin pivot
        ps = T.pivot.origin;
        switch (ps & PIVOT_HCENTER) {
        case PIVOT_HCENTER: {
          vorig.x -= maxSize.x * 0.5f;
          break;
        }
        case PIVOT_RIGHT: {
          vorig.x -= maxSize.x;
          break;
        }
        default:
          break;
        }
        switch (ps & PIVOT_VCENTER) {
        case PIVOT_VCENTER: {
          vorig.y += (maxSize.y - src.font.lineHeight) * 0.5f;
          break;
        }
        case PIVOT_TOP: {
          vorig.y -= src.font.lineHeight;
          break;
        }
        case PIVOT_BOTTOM: {
          vorig.y += maxSize.y;
          break;
        }
        default:
          break;
        }
      }

      vec2 start_ = vorig;
      flat_vertex fv;
      character A;
      for (char *t = T.d.label.text; *t; ++t) {
        if (*t > 0x7f) {
          continue;
        }
        if (*t == '\n') {
          start_.x = vorig.x;
          start_.y -= src.font.lineHeight;
          continue;
        }
        A = src.font.chs[*t];
        if (A.size.x) {

          fv.uv = (vec2){A.uvm.x, A.uvm.y};
          fv.pos = (vec2){start_.x + A.size.x, start_.y};
          vec2_trn(&fv.pos, A.off);
          fv.pos.y += src.font.lineHeight;
          src.vertex_buffer[v++] = fv;

          fv.uv = (vec2){A.uv.x, A.uvm.y};
          fv.pos = start_;
          vec2_trn(&fv.pos, A.off);
          fv.pos.y += src.font.lineHeight;
          src.vertex_buffer[v++] = fv;

          fv.uv = (vec2){A.uvm.x, A.uv.y};
          fv.pos = (vec2){start_.x + A.size.x, start_.y + A.size.y};
          vec2_trn(&fv.pos, A.off);
          fv.pos.y += src.font.lineHeight;
          src.vertex_buffer[v++] = fv;

          fv.uv = (vec2){A.uv.x, A.uv.y};
          fv.pos = (vec2){start_.x, start_.y + A.size.y};
          vec2_trn(&fv.pos, A.off);
          fv.pos.y += src.font.lineHeight;
          src.vertex_buffer[v++] = fv;
        }

        start_.x += A.xadv;
      }
      break;
    }
    default:
      break;
    }
  }
  if (v)
    global_engine.flatRender(src.font.bitmap, src.vertex_buffer, v >> 2);
}
void uistage_term() {
  for (actor i = 0; i < UISTAGE_MAX_ACTORS; ++i) {
    _actor T = src.actors[i];
    switch (T.type) {
    case ACTOR_INVALID:
      i = UISTAGE_MAX_ACTORS;
      break;
    case ACTOR_TEXT:
      free(T.d.label.text);
      break;
    default:
      break;
    }
  }
  free(src.font.kearns);
  global_engine.deleteTexture(src.font.bitmap);
}