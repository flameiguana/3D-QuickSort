
#include "Camera.h"

Camera::Camera(CameraType cameraType, glm::mat4 projectionMatrix)
	:cameraType(cameraType), mProjection(projectionMatrix){
	cameraPosition = glm::vec3(0.0f, 0.0f, 1.0f);
	cameraAt = glm::vec3(0.0f, 0.0f, 0.0f); //Look at origin
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //Determines which axis is "up" for the camera.
	mView = glm::lookAt(cameraPosition, cameraAt, cameraUp);
}


void Camera::scale(float zoomFactor){
	glm::mat4 scale(1.0f);
	scale = glm::scale(scale, glm::vec3(zoomFactor, zoomFactor, zoomFactor));
	mProjection = scale * mProjection;
	mScaleProjection = scale * mScaleProjection;
}

void Camera::translate(glm::vec3 offset){
	glm::mat4 translation(1.0f);
	translation = glm::translate(translation, offset);
	mView = translation * mView;
	cameraPosition = glm::vec3(translation * glm::vec4(cameraPosition, 1.0f));
}
//use cos for x, sin for y, as a function of time. scale these by radius sqrt((cameraPosition - objectCenter)^2)
void Camera::moveTo(glm::vec3 point){
	cameraPosition = point;
	mView = glm::lookAt(cameraPosition, cameraAt, cameraUp);
}

//Does not have same name semantics as glm::lookat. 
//You only provide point to look at, the rest remains same
void Camera::lookAt(glm::vec3 point){
	cameraAt = point;
	mView = glm::lookAt(cameraPosition, cameraAt, cameraUp);
}

glm::vec3 Camera::windowToWorld(glm::vec3 winCoord, glm::vec4 viewPort){
	return glm::unProject(winCoord, mView, mProjection, viewPort);
}