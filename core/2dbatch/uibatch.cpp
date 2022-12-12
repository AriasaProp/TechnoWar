#include "uibatch.h"

#include "../math/matrix4.h"

#define X1 0
#define Y1 1
#define C1 2
#define U1 3
#define V1 4
#define X2 5
#define Y2 6
#define C2 7
#define U2 8
#define V2 9
#define X3 10
#define Y3 11
#define C3 12
#define U3 13
#define V3 14
#define X4 15
#define Y4 16
#define C4 17
#define U4 18
#define V4 19

extern TranslatedGraphicsFunction *tgf;

UI_Batch::UI_Batch(float width, float height) {
	vertices = new float[MAX_TEXTURE_UI*20];
	projection = new float[16];
	//prepare
	resize(width, height);
}
UI_Batch::~UI_Batch() {
	delete[] vertices;
	delete[] projection;
}
void UI_Batch::resize(float width, float height) {
	matrix4::toOrtho(projection, 0, width, 0, height, 0, 1);
	tgf->update_2d_batch_projection(projection);
}
void UI_Batch::begin() {
  tgf->depth_mask(false);
}
void UI_Batch::end() {
  if (texUsed) {
  	tgf->draw_2d_batch_vertices(lastTexture, vertices, texUsed);
		texUsed = 0;
  }
  lastTexture = nullptr;
  tgf->depth_mask(true);
  tgf->switch_capability(TGF_BLEND, true);
}
void UI_Batch::draw(texture_core *t, float x, float y) {
    draw(t, x, y, t->width, t->height);
}
void UI_Batch::draw(texture_core *t, float x, float y, float width, float height) {
    draw(t, x, y, width, height, 0, 1, 1, 0);
}
struct vertices_data {
	float x;
	float y;
	struct color {
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	} colors;
	float u;
	float v;
} temp_vert;
void UI_Batch::draw(texture_core *t, float x, float y, float width, float height, float u, float v, float u2, float v2) {
    if (!lastTexture) {
    	lastTexture = t;
    } else if (t != lastTexture) {
    	tgf->draw_2d_batch_vertices(lastTexture, vertices, texUsed);
    	lastTexture = t;
    }
    else if (texUsed == MAX_TEXTURE_UI) {
    	tgf->draw_2d_batch_vertices(lastTexture, vertices, texUsed);
    	texUsed = 0;
    }
    temp_vert.x = x;
    temp_vert.y = y;
    memcpy(&temp_vert.colors, &colorPacked, 4*sizeof(unsigned char));
    temp_vert.u = u;
    temp_vert.v = v;
    vertices_data *cmp = (vertices_data*)(vertices+texUsed);
    memcpy(cmp, &temp_vert, sizeof(vertices_data));
    cmp++;
    temp_vert.y = y + height;
    temp_vert.v = v2;
    memcpy(cmp, &temp_vert, sizeof(vertices_data));
    cmp++;
    temp_vert.x = x + width;
    temp_vert.u = u2;
    memcpy(cmp, &temp_vert, sizeof(vertices_data));
    cmp++;
    temp_vert.y = y;
    temp_vert.v = v;
    memcpy(cmp, &temp_vert, sizeof(vertices_data));
    texUsed++;
}



