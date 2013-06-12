
/*
Author: Gerardo Perez

An incomplete mesh class that could use more features.

Currently supports:
Creating a Buffer.
Creating a shader (with hard-coded values)
Reading from a file with Alex Pang's format.
Reading from a list of vertices.
Calculating normals for verts and surfaces.
Changing lighting type.
*/

#pragma once
#include "Angel.h"
// #include "Animation.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <queue>
#include <sstream>
#include "Camera.h"

typedef enum {FLAT, SMOOTH} ShadingType;
typedef enum {NORMAL_MODE = 1, COLOR_ID} LightingType;

class Mesh
{
public:
	Mesh(const std::string &coords, const std::string &polys);
	Mesh(std::vector<glm::vec3> &vertices);
	void calculateNormals();
	std::vector<glm::vec3> getRawVertices(){return rawVerts;}
	std::vector<glm::vec3> getVertexNormals(){return vertexNormals;}
	std::vector<glm::vec3> getSurfaceNormals(){return surfaceNormals;}
	GLuint getCurrentLighting(){return currentLighting;}
	//Prereq, normals should have been calculated already
	void createGLBuffer(bool smooth, GLuint* vao, int index);
	void changeShading(ShadingType shading);
	ShadingType getCurrentShading(){return currentShading;}
	void setLighting(LightingType lighting);

	void translate(glm::vec3& offset);
	void rotateSelf(glm::vec3& offset, bool positive=true);
	void translateOrigin();
	void translateBack();

	void absoluteTranslate(glm::vec3& position);
	void moveTo(glm::vec3& point);
	void rotate(glm::vec3& rotations, bool positive = true);
	void scale(float scaling);
	void zoom(float zoomFactor);
	void moveCamera(glm::vec3& offset);
	//too specific of a function, but itll do. 
	void anchorBottom();
	void scaleCenterUniform(float scaling);
	void scaleCenter(glm::vec3& scaleFactor);
	void drawBoundingBox(){drawBox = true;}
	void removeBoundingBox(){drawBox = false;}
	void setDiffuse(glm::vec4 lightDiffuse, glm::vec4 materialDiffuse);
	void setSpecular(glm::vec4 materialDiffuse, float shininess);
	bool colorMatch(unsigned char *color);
	void setupShader(GLuint& program, Camera* camera);
	glm::vec4 getSize(){return size;}
	glm::vec3 getCenter(){return currentCenter;}
	void setWireframe(bool value){wireframe = value;}
	void draw();
	~Mesh();

	GLuint vertexBuffer;
	GLuint boxVertexBuffer;
	GLuint boxElementBuffer;

	GLuint program;
	int vaoIndex;
	GLuint* vao;
	GLuint normalModeIndex,  colorKeyIndex;
	GLuint specularOn, specularOff;

	GLuint subroutines[1];

	GLuint vPosition, vNormal, modelView_loc, mTransformation_loc, mProjection_loc;
	GLuint lightPosition_loc;
	GLuint lightDiffuse_loc, materialDiffuse_loc;
	GLuint lightSpecular_loc, materialSpecular_loc, shininess_loc;

	/* Uniform Variables*/
	glm::mat4 mTransformation;

	size_t sizeofVertices;
	size_t sizeofNormals;

private:

	Camera* camera;
	bool wireframe;
	class Vertex;
	class Polygon;
	class BBox;
	bool drawBox;
	//Tells us to reload uniform variables.
	bool changedUniform;
	/*Colors*/
	glm::vec4 materialDiffuse;
	glm::vec4 lightDiffuse;
	glm::vec4 lightPosition;
	glm::vec4 lightSpecular;
	glm::vec4 materialSpecular;
	float shininess;

	glm::mat4 mTranslation;
	glm::mat4 mRotation;
	glm::mat4 mScale;
	glm::vec3 maxCoords;
	glm::vec3 minCoords;
	glm::vec4 size;
	glm::vec3 center;
	//Animation* animation;
	//specifies center of display of the mesh (may differ from actual coordinates in vertices array)
	glm::vec3 currentCenter;
	BBox* boundingBox;

	void setColorID();
	std::vector<Vertex*> vertexList;
	std::vector<Polygon*> polygons;

	std::vector<glm::vec3> rawVerts;
	std::vector<glm::vec3> rawCoords;
	std::vector<glm::vec3> vertexNormals;
	//std::vector<glm::vec3> boundingBox;

	std::vector<glm::vec3*> vertexNRefs;
	std::vector<glm::vec3> surfaceNormals;
	//float shininess;
	bool smartPolys;
	LightingType currentLighting;
	ShadingType currentShading;
	unsigned char colorID[3];
	static unsigned char globalColorID[3];
};
