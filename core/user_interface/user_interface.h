#ifndef Included_User_Interface
#define Included_User_Interface 1

#include "../graphics/graphics.h"
#include "actors/actor.h"

namespace user_interface {
	struct Touchable {
		bool hit(float, float);
	};
	//called by input
	void keyDown(int);
	void keyUp(int);
	void keyTyped(char);
	
	void touchDown(float,float,int,int);
	void touchDragged(float,float,int);
	void touchUp(float,float,int,int);
	void mouseMoved(float,float);
	void scrolled(float);
	
	void addActor(Actor*);
	void removeActor(Actor*);
	void draw(graphics*);
	void clearActor();
}


#endif //Included_User_Interface