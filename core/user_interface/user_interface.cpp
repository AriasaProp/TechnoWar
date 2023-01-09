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
	size_t len = actors->size();
	if (len == 0) return;
	len *= 4;
	flat_vertex *tmp_v = new flat_vertex[len];
	flat_vertex *d_tmp = tmp_v;
	flat_vertex fv;
	for(Actor *act : *actors) {
		fv.x = act->x;
		fv.y = act->y;
		memcpy(&fv.r, act->color, 4*sizeof(unsigned char));
		memcpy(d_tmp, &fv, sizeof(flat_vertex));
		d_tmp++;
		fv.y += act->height;
		memcpy(d_tmp, &fv, sizeof(flat_vertex));
		d_tmp++;
		fv.x += act->width;
		fv.y = act->y;
		memcpy(d_tmp, &fv, sizeof(flat_vertex));
		d_tmp++;
		fv.y += act->height;
		memcpy(d_tmp, &fv, sizeof(flat_vertex));
		d_tmp++;
	}
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


