#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <iostream>
#include <vector>

typedef enum {ORTHOGRAPHIC = 1, PERSPECTIVE} CameraType;

class Camera
{
public:
	Camera(){};

	Camera(CameraType cameraType, const glm::mat4& projectionMatrix);

	void lookAt(glm::vec3& point);
	void translate(const glm::vec3& offset);
	void moveTo(const glm::vec3& point);
	void scale(float zoomFactor);
	void rotate(const glm::vec3& rotation);
	void rotateAbout(const glm::vec3& rotation, const glm::vec3& point);
	glm::mat4 getInverse(){return glm::inverse(mView);};
	glm::vec3 windowToWorld(const glm::vec3& winCoord, const glm::vec4& viewPort);

	glm::vec3 getPosition(){return cameraPosition;}
	glm::mat4 getModelView(){return mView;}
	glm::mat4 getProjection(){return mProjection;}
	void setProjection(const glm::mat4& projectionMatrix){ mProjection = projectionMatrix;}

private:
	glm::mat4 mView;
	glm::mat4 mProjection;
	glm::mat4 mScaleProjection;
	glm::vec3 cameraUp;
	glm::vec3 cameraPosition;
	glm::vec3 cameraAt;
	CameraType cameraType;
};
