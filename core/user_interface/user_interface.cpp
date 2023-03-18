#include "user_interface.hpp"
#include "../engine.hpp"
#include <unordered_set>

std::unordered_set<Actor*> actors;

void user_interface::addActor(Actor *a) {
	actors->insert(a);
}
void user_interface::removeActor(Actor *a) {
	std::unordered_set<Actor*>::iterator v = actors->find(a);
	if (v == actors->end()) return;
	actors->erase(v);
}
void user_interface::draw() {
	const unsigned int len = actors->size();
	if (len == 0) return;
	engine::flat_vertex tmp_v[len*4];
	engine::flat_vertex *curv = tmp_v;
	unsigned int i = 0;
	std::unordered_set<Actor*>::iterator t = actors->begin();
	while((i < len) && (t != actors->end())) {
		Actor *act = *t;
		engine::flat_vertex fv;
		memcpy(&fv.color, &act->color, 4 * sizeof(unsigned char));
		fv.x = act->x;
		fv.y = act->y;
		memcpy(curv++, &fv, sizeof(engine::flat_vertex));
		fv.y += act->height;
		memcpy(curv++, &fv, sizeof(engine::flat_vertex));
		fv.x += act->width;
		fv.y = act->y;
		memcpy(curv++, &fv, sizeof(engine::flat_vertex));
		fv.y += act->height;
		memcpy(curv++, &fv, sizeof(engine::flat_vertex));
		i++, t++;
	}
	engine::graph->flat_render(tmp_v, len);
}
void user_interface::clearActor() {
	actors->clear();
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


