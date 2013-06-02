#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Mesh.h"


//Client program creates these and passes them to mesh.
//Right when mesh finds one of these, it starts the update method.
class Animation
{
public:
	enum AnimationType {ROTATE, SCALE, TRANSLATE, POSITION};
	Animation(AnimationType type);

	//Pass in the initial values, read from somewhere.
	//Typically 0,0,0 if object is untranslated.
	//1.0, 1.0f, 1.0f if object is unscaled.
	//0, 0, 0 if object is unrotated.
	//Pass in current center if moving to an absolute position
	void setStart(Mesh* mesh, glm::vec3& start);
	//Enter final scale, rotation (degrees), or position
	void setGoal(glm::vec3& goal, int duration);
	//Calls the appropriate transformation function in the mesh.
	void update(int time);

	void chain(Animation& animation);

	bool hasEnded() { return ended;}
	Animation* getLink(){ return nextLink;}
private:
	glm::vec3 calculateStep(float t);
	AnimationType animationType;
	glm::vec3 start;
	glm::vec3 goal;
	glm::vec3 currentTransformation; // a portion of the goal trans
	int startTime;
	int timeLeft;
	int duration;
	bool started;
	bool ended;
	Mesh* mesh;
	Animation* nextLink;
};
