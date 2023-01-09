#include "user_interface.h"
#include <unordered_set>

std::unordered_set<user_interface::Actor*> actors;

void user_interface::addActor(user_interface::Actor *a) {
	actors.insert(a);
}
void user_interface::removeActor(user_interface::Actor *a) {
	actors.erase(a);
}
void user_interface::draw(graphics *g) {
	size_t len = actors.size();
	if (len == 0) return;
	len *= 4;
	flat_vertex *tmp_v = (flat_vertex*)alloca(len*sizeof(flat_vertex));
	flat_vertex *d_tmp = tmp_v;
	for(Actor *act : actors) {
		flat_vertex per[4] = {
			{act->x, act->y, act->color[0], act->color[1], act->color[2], act->color[3]},
			{act->x, act->y+act->height, act->color[0], act->color[1], act->color[2], act->color[3]},
			{act->x+act->width, act->y, act->color[0], act->color[1], act->color[2], act->color[3]},
			{act->x+act->width, act->y+act->height, act->color[0], act->color[1], act->color[2], act->color[3]}
		};
		memcpy(d_tmp, per, 4*sizeof(flat_vertex));
		d_tmp += 4;
	}
	
	g->flat_render(tmp_v, len);
}
void user_interface::clearActor() {
	actors.clear();
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


