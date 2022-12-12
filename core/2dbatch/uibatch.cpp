#include "uibatch.h"
#include <cstring>
#include "../math/matrix4.h"

extern TranslatedGraphicsFunction *tgf;

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
};
static vertices_data temp_vert;
static vertices_data *vertices;

UI_Batch::UI_Batch(float width, float height) {
	vertices = new vertices_data[MAX_TEXTURE_UI*4];
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
    memcpy(&temp_vert.colors, &colorPacked, 4*sizeof(unsigned char));
    temp_vert.x = x;
    temp_vert.y = y;
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



