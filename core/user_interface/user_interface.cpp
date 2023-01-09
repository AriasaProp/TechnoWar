#include "user_interface.h"
#include <unordered_set>

std::unordered_set<user_interface::Actor*> *actors;

void user_interface::addActor(user_interface::Actor *a) {
	if (!actors)
		actors = new std::unordered_set<user_interface::Actor*>();
	actors->insert(a);
}
void user_interface::removeActor(user_interface::Actor *a) {
	if (!actors) return;
	std::unordered_set<user_interface::Actor*>::iterator v = actors->find(a);
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
	std::unordered_set<user_interface::Actor*>::iterator t = actors.begin();
	while((i < len) && (t != actors.end())) {
		Actor *act = *t;
		memcpy(&fv.r, act->color, 4*sizeof(unsigned char));
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
	g->flat_render(tmp_v, len*4);
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


