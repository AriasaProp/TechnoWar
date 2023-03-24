

namespace engine {
	//ui_core
	struct flat_vertex {
		float x, y;
		unsigned char color[4];
	};
}
void flat_render(engine::flat_vertex *v, unsigned int len) {
	glDisable(GL_DEPTH_TEST);
	glUseProgram(ubatch->shader);
	if (ubatch->dirty_projection) {
		glUniformMatrix4fv(ubatch->u_projection, 1, false, ubatch->ui_projection);
		ubatch->dirty_projection = false;
	}
	glBindVertexArray(ubatch->vao);
	glBindBuffer(GL_ARRAY_BUFFER, ubatch->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4*len*sizeof(engine::flat_vertex), (void*)v);
	glDrawElements(GL_TRIANGLES, 6*len, GL_UNSIGNED_SHORT, NULL);
	glBindVertexArray(0);
	glUseProgram(0);
}

std::unordered_set<Actor*> actors;
void user_interface::addActor(Actor *a) {
	actors.insert(a);
}
void user_interface::draw() {
	const unsigned int len = actors.size();
	if (!len) return;
	engine::flat_vertex *tmp_v = new engine::flat_vertex[len * 4];
	engine::flat_vertex *curv = tmp_v;
	for (Actor *const &act : actors) {
		curv->x = act->x;
		curv->y = act->y;
		memcpy(curv->color, act->color, 4 * sizeof(unsigned char));
		curv++;
		curv->x = act->x;
		curv->y = act->y+act->height;
		memcpy(curv->color, act->color, 4 * sizeof(unsigned char));
		curv++;
		curv->x = act->x+act->width;
		curv->y = act->y;
		memcpy(curv->color, act->color, 4 * sizeof(unsigned char));
		curv++;
		curv->x = act->x+act->width;
		curv->y = act->y+act->height;
		memcpy(curv->color, act->color, 4 * sizeof(unsigned char));
		curv++;
	}
	engine::graph->flat_render(tmp_v, len);
	delete[] tmp_v;
}