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
	static typedef enum {ROTATE, SCALE, TRANSLATE, POSITION} AnimationType;
	Animation(AnimationType type);
	//Enter final scale, rotation (degrees), or position
	void setGoal(glm::vec3& goal, int duration);
	//Calls the appropriate transformation function in the mesh.
	void update(Mesh* mesh, int time);
private:
	AnimationType animationType;
	glm::vec3 goal;
	glm::vec3 currentTransformation; // a portion of the goal trans
	int timeLeft;
	bool started;
	bool ended;
};
