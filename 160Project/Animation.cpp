#include "Animation.h" 

Animation::Animation(AnimationType type):animationType(type), started(false), ended(false){}


void Animation::setStart(glm::vec3& start){
	this->start = start;
}
void Animation::setGoal(glm::vec3& goal, int duration){
	this->goal = goal;
	this->duration = duration;
}


glm::vec3 Animation::calculateStep(float t){
	//linear interpolation (y = mx + b)
	/*
	y is the new position, m is change in value over duration, x is time, b is starting value
	*/
	glm::vec3 changeInValue = (goal - start)/(float)duration;
	return changeInValue * t + start;
}

//Implement these, decide whether to make calculations separately or not.
void Animation::update(Mesh* mesh, int time){
	if(!started){
		startTime = time;
		started = true;
	}
	int difference = time - startTime;
	
	if(difference > duration){
		ended = true;
		//take parameters from chained one to this one
		//or let client handle traversing chain
		//or have an animation manager class that can start animations. That way mesh is only passed once.
		//bad thins is that you can't use the same animation on multiple variables witout perhaps some ugliness.
		return;
	}
	glm::vec3 step = calculateStep(difference);
	switch(animationType){
		case TRANSLATE:
			mesh->translate(step);
			break;
		case ROTATE:
			mesh->rotateSelf(step);
			break;
		case SCALE:
			mesh->scaleCenter(step);
			break;
		case POSITION:
			mesh->moveTo(step);
			break;
	}
}

void Animation::chain(Animation& animation){
	//copy to heap
	nextLink = new Animation(animation);
}
