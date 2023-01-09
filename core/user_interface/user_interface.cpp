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
	for(Actor *act : *actors) {
		memcpy(d_tmp, (flat_vertex[]){
			{act->x, act->y, act->color[0], act->color[1], act->color[2], act->color[3]},
			{act->x, act->y+act->height, act->color[0], act->color[1], act->color[2], act->color[3]},
			{act->x+act->width, act->y, act->color[0], act->color[1], act->color[2], act->color[3]},
			{act->x+act->width, act->y+act->height, act->color[0], act->color[1], act->color[2], act->color[3]}
		}, 4*sizeof(flat_vertex));
		d_tmp += 4;
	}
	g->flat_render(tmp_v, len);
	delete[] tmp_v;
}
void user_interface::clearActor() {
	if (!actors) return;
	actors->clear();
	delete actors;
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


