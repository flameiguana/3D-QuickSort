#include "Animation.h" 

Mesh::Animation(AnimationType type):animationType(type), started(false), ended(false){}

Mesh::setGoal(glm::vec3& goal, int duration){
	this->goal = goal;
	timer = duration;
}

Mesh::update(Mesh* mesh, int time){
	switch(animationType){
		case TRANSLATE:
			mesh->translate();
			break;
		case ROTATE:
			mesh->rotateSelf();
			break;
		case SCALE:
			mesh->scaleSelf();
			break;
		case POSITION:
			mesh->moveTo();
			break;
	}
}