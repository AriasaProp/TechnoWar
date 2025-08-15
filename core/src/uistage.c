#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "math/vec_math.h"
#include "stb/stb_image.h"

#define UISTAGE_IMPLEMENTATION
#include "uistage.h"
#define UISTAGE_MAX_ACTORS 128

typedef enum {
  ACTOR_INVALID = 0,
  ACTOR_LABEL,
} _actor_type;
typedef struct {
  vec2 pos, size, off;
  float xadv;
} character;
typedef struct {
  char a, b;
  float d;
} kerning;
typedef struct {
  _actor_type type;
  vec2 origin;
  pivot_state origin_pivot, world_pivot;
  union {
    struct {
      char *text;
    } label;
  } d;
} _actor;
static struct {
  struct {
    texture bitmap;
    vec2 bitmap_size;
    float size, lineHeight, base;
    character chs[0xff];
    size_t kerning_length;
    kerning *kearns;
  } font;
  _actor actors[UISTAGE_MAX_ACTORS + 2];
  flat_vertex vertex_buffer[MAX_UI_DRAW];
} src;

actor create_label(size_t strl) {
  actor i;
  for (i = 0; i < UISTAGE_MAX_ACTORS; ++i) {
    if (src.actors[i].type != ACTOR_INVALID)
      continue;
    src.actors[i].type = ACTOR_LABEL;
    src.actors[i].d.label.text = (char *)malloc(strl);
    break;
  }
  return i;
}
actor destroy_actor(actor a) {
  switch (src.actors[a].type) {
  case ACTOR_LABEL:
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
void set_actor_pivot_origin(actor a, const pivot_state p) {
  src.actors[a].origin_pivot = p;
}
void set_actor_pivot_world(actor a, const pivot_state p) {
  src.actors[a].world_pivot = p;
}
void set_label_text(actor a, const char *t) {
  if (src.actors[a].type != ACTOR_LABEL)
    return;
  strcpy(src.actors[a].d.label.text, t);
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
    src.font.bitmap_size = (vec2){(float)tempi[0], (float)tempi[1]};
    global_engine.assetClose(ast);

    // load font sets
    size_t i, l;
    char *line;
    ast = global_engine.assetBuffer("fonts/default/default.fnt", (void **)&line, &l);
    line = strtok(line, "\n");
    do {
      if (strstr(line, "info")) {
        char tname[64], tchar[64];
        sscanf(line, "info face=%s size=%d bold=%d italic=%d charset=%s unicode=%d stretchH=%d smooth=%d aa=%d",
               tname, tempi, tempi + 1, tempi + 2, tchar, tempi + 3, tempi + 4, tempi + 5, tempi + 6);
        src.font.size = (float)tempi[0];
      } else if (strstr(line, "common")) {
        sscanf(line, "common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=%d packed=%d",
               tempi, tempi + 1, tempi + 2, tempi + 3, tempi + 4, tempi + 5);
        src.font.lineHeight = (float)tempi[0];
        src.font.base = (float)tempi[1];
      } else if (strstr(line, "chars ")) {
        sscanf(line, "chars count=%d", tempi + 8);
        for (i = 0; i < tempi[8]; ++i) {
          line = strtok(NULL, "\n");
          sscanf(line, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d",
                 tempi, tempi + 1, tempi + 2, tempi + 3, tempi + 4, tempi + 5, tempi + 6, tempi + 7);
          src.font.chs[tempi[0]].pos = (vec2){(float)tempi[1], (float)tempi[2]};
          src.font.chs[tempi[0]].size = (vec2){(float)tempi[3], (float)tempi[4]};
          src.font.chs[tempi[0]].off = (vec2){(float)tempi[5], (float)tempi[6]};
          src.font.chs[tempi[0]].xadv = (float)tempi[7];
        }
      } else if (strstr(line, "kernings ")) {
        sscanf(line, "kernings count=%lu", &src.font.kerning_length);
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
  for (actor i = 0; i < UISTAGE_MAX_ACTORS; ++i) {
    _actor T = src.actors[i];
    switch (T.type) {
    case ACTOR_INVALID:
      i = UISTAGE_MAX_ACTORS;
      break;
    case ACTOR_LABEL: {
      float left = 0, top = global_engine.getScreenSize().y * 0.5f;
      for (char *t = T.d.label.text; *t; ++t) {
        character A = src.font.chs[*t];
        vec2 isize = vec2_div((vec2){1.f, 1.f}, src.font.bitmap_size);

        src.vertex_buffer[v].uv = (vec2){A.pos.x + A.size.x, A.pos.y + A.size.y};
        vec2_scl(&src.vertex_buffer[v].uv, isize);
        src.vertex_buffer[v++].pos = (vec2){left + A.size.x, top};

        src.vertex_buffer[v].uv = (vec2){A.pos.x, A.pos.y + A.size.y};
        vec2_scl(&src.vertex_buffer[v].uv, isize);
        src.vertex_buffer[v++].pos = (vec2){left, top};

        src.vertex_buffer[v].uv = (vec2){A.pos.x + A.size.x, A.pos.y};
        vec2_scl(&src.vertex_buffer[v].uv, isize);
        src.vertex_buffer[v++].pos = (vec2){left + A.size.x, top + A.size.y};

        src.vertex_buffer[v].uv = (vec2){A.pos.x, A.pos.y};
        vec2_scl(&src.vertex_buffer[v].uv, isize);
        src.vertex_buffer[v++].pos = (vec2){left, top + A.size.y};

        left += A.size.x;
      }
      break;
    }
    default:
      break;
    }
  }
  if (v)
    global_engine.flatRender(src.font.bitmap, src.vertex_buffer, v >> 2);

  /*
  v = 0;
  {
    vec2 p = {.x = 50, .y = 50};
    src.vertex_buffer[v++].pos = (vec2){p.x + 150, p.y};
    src.vertex_buffer[v++].pos = (vec2){p.x + 150, p.y + 150};
    src.vertex_buffer[v++].pos = (vec2){p.x, p.y};
    src.vertex_buffer[v++].pos = (vec2){p.x, p.y + 150};

    p.x += 250;
    src.vertex_buffer[v++].pos = (vec2){p.x + 150, p.y};
    src.vertex_buffer[v++].pos = (vec2){p.x, p.y};
    src.vertex_buffer[v++].pos = (vec2){p.x + 150, p.y + 150};
    src.vertex_buffer[v++].pos = (vec2){p.x, p.y + 150};
  }
  if (v)
    global_engine.flatRender(0, src.vertex_buffer, v >> 2);
  */
}
void uistage_term() {
  for (actor i = 0; i < UISTAGE_MAX_ACTORS; ++i) {
    _actor T = src.actors[i];
    switch (T.type) {
    case ACTOR_LABEL:
      free(T.d.label.text);
      break;
    default:
      break;
    }
  }
  free(src.font.kearns);
  global_engine.deleteTexture(src.font.bitmap);
}