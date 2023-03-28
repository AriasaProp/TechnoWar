#include "user_interface.hpp"
#include "../engine.hpp"
#include <unordered_set>

std::unordered_set<Actor*> actors;

void user_interface::addActor(Actor *a) {
	actors.insert(a);
}
void user_interface::removeActor(Actor *a) {
	std::unordered_set<Actor*>::iterator v = actors.find(a);
	if (v == actors.end()) return;
	actors.erase(v);
}
void user_interface::draw() {
	const unsigned int len = actors.size();
	if (!len) return;
	engine::flat_vertex *tmp_v = new engine::flat_vertex[len * 4];
	engine::flat_vertex *curv = tmp_v;
	std::unordered_set<Actor*>::iterator it = actors.begin();
	engine::texture_core *cur_tex = (*it)->img.getCore();
	unsigned int i = 0;
	while (it != actors.end()) {
		const Actor *const &act = *it;
		if (cur_tex != act->img.getCore()) {
			engine::graph->flat_render(cur_tex, tmp_v, i);
			i = 0;
			curv = tmp_v;
			cur_tex = act->img.getCore();
		}
		curv->x = act->x;
		curv->y = act->y;
		memcpy(curv->color, act->color, 4 * sizeof(unsigned char));
		curv->u = 0;
		curv->v = 0;
		curv++;
		curv->x = act->x;
		curv->y = act->y+act->height;
		memcpy(curv->color, act->color, 4 * sizeof(unsigned char));
		curv->u = 0;
		curv->v = 1;
		curv++;
		curv->x = act->x+act->width;
		curv->y = act->y;
		memcpy(curv->color, act->color, 4 * sizeof(unsigned char));
		curv->u = 1;
		curv->v = 0;
		curv++;
		curv->x = act->x+act->width;
		curv->y = act->y+act->height;
		memcpy(curv->color, act->color, 4 * sizeof(unsigned char));
		curv->u = 1;
		curv->v = 1;
		curv++;
		it++;
		i++;
	}
	engine::graph->flat_render(cur_tex, tmp_v, i);
	delete[] tmp_v;
}
void user_interface::clearActor() {
	actors.clear();
}
