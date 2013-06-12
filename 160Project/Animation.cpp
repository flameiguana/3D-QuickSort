#include "Animation.h" 

Animation::Animation(AnimationType type):animationType(type), started(false), ended(false),hasLink(false){}


void Animation::setStart(Mesh* mesh, glm::vec3& start){
	this->mesh = mesh;
	this->start = start;
}
void Animation::setGoal(glm::vec3& goal, float duration, EasingType easing){
	this->goal = goal;
	this->duration = duration;
	changeInValue = goal - start;
	easingType = easing;
}

//Linear or now, try ease.
glm::vec3 Animation::calculateStep(float t){
	switch(easingType)
	{
		case LINEAR:
			return changeInValue * (t/duration) + start;
		case ELASTIC_OUT: //From Robert Penning's easing functions
			if (t == 0.0f) return start;
			if ((t /= duration) == 1.0f) return start + changeInValue;  
			float p = duration * .3f;
			glm::vec3 a = changeInValue; 
			float s = p / 4;
			return (a * (float)pow(2, -10 * t) * (float)sin( (t * duration - s) * (2 * PI)/ p ) + changeInValue + start);	
	}
	//linear interpolation (y = mx + b)
	/*
	y is the new position, m is change in value over duration, x is time, b is starting value
	*/
	//return (1.0f - t) * start + t* goal;
}

//Implement these, decide whether to make calculations separately or not.
void Animation::update(int time){
	if(!started){
		startTime = time;
		started = true;
	}
	float difference = time - startTime;
	
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
	hasLink = true;
	nextLink = new Animation(animation);
}

Animation::~Animation(){
	//delete nextLink;
}
