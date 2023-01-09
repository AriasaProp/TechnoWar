#include "user_interface.h"
#include <unordered_set>

std::unordered_set<Actor*> *actors;

void user_interface::addActor(Actor *a) {
	if (!actors)
		actors = new std::unordered_set<Actor*>({a});
	else
		actors->insert(a);
}
void user_interface::removeActor(Actor *a) {
	if (!actors) return;
	std::unordered_set<Actor*>::iterator v = actors->find(a);
	if (v == actors->end()) return;
	actors->erase(v);
}
void user_interface::draw(graphics *g) {
	if (!actors) return;
	const size_t len = actors->size();
	if (len == 0) return;
	flat_vertex *tmp_v = new flat_vertex[len*4];
	flat_vertex fv;
	size_t i = 0;
	std::unordered_set<Actor*>::iterator t = actors->begin();
	while((i < len) && (t != actors->end())) {
		Actor *act = *t;
		fv.r = act->color[0];
		fv.g = act->color[1];
		fv.b = act->color[2];
		fv.a = act->color[3];
		fv.x = act->x;
		fv.y = act->y;
		memcpy(&tmp_v[i*4], &fv, sizeof(flat_vertex));
		fv.y += act->height;
		memcpy(&tmp_v[i*4+1], &fv, sizeof(flat_vertex));
		fv.x += act->width;
		fv.y = act->y;
		memcpy(&tmp_v[i*4+2], &fv, sizeof(flat_vertex));
		fv.y += act->height;
		memcpy(&tmp_v[i*4+3], &fv, sizeof(flat_vertex));
		i++, t++;
	}
	/*
	flat_vertex tmp_v[12] = {
		{120, 120, 0xff, 0x00, 0x00, 0xff},
		{120, 295, 0xff, 0x00, 0x00, 0xff},
		{320, 120, 0xff, 0x00, 0x00, 0xff},
		{320, 295, 0xff, 0x00, 0x00, 0x00},
		
		{700, 100, 0x00, 0xff, 0x00, 0xff},
		{700, 225, 0x00, 0xff, 0x00, 0xff},
		{850, 100, 0x00, 0xff, 0x00, 0xff},
		{850, 225, 0x00, 0xff, 0x00, 0xff},
		
		{1100, 100, 0x00, 0xff, 0xff, 0xff},
		{1100, 200, 0x00, 0xff, 0xff, 0xff},
		{1225, 100, 0x00, 0xff, 0xff, 0xff},
		{1225, 200, 0x00, 0xff, 0xff, 0xff}
	};
	*/
	g->flat_render(tmp_v, len);
	delete[] tmp_v;
}
void user_interface::clearActor() {
	if (!actors) return;
	actors->clear();
	delete actors;
	actors = nullptr;
}
/*
void user_interface::keyDown(int keyCode) {
	
}
void user_interface::keyUp(int keyCode) {
	
}
void user_interface::keyTyped(char key) {
	
}
void user_interface::touchDown(float x,float y,int ptr,int button) {
	
}
void user_interface::touchDragged(float y,float x,int ptr) {
	
}
void user_interface::touchUp(float x,float y,int ptr,int button) {
	
}
void user_interface::mouseMoved(float x,float y) {
	
}
void user_interface::scrolled(float amout) {
	
}
*/


