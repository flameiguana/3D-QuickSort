/* 
* Gerardo Perez
* Lab 7
* Contains some code from OpenGL Cookbook (in shader)
* 
* Clicking on an object will change its shading.
* Now includes zooming.
*/

#include "Angel.h"
#include "Mesh.h"
#include "glm/glm.hpp"
#include <Windows.h>

#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <string>

#include <AntTweakBar.h>

int WINDOW_BORDER;
const int HEIGHT = 800;
int winWidth;
int winHeight;

typedef enum {SCALE=1, ROTATE_V, ROTATE_H, TRANSLATE} TransformMode;
glm::mat4 globalCamera;
TransformMode currentTransform;
//Initialize the static variable.
unsigned char Mesh::globalColorID[3] = {0,0,0};

//Allow us to create a menu for switching between diffuse, ambient, and ambient + diffuse lighting.
const LightingType Lighting_MAX = DIFFUSE_AND_AMBIENT;
LightingType currentLighting = DIFFUSE;
LightingType prevLighting = DIFFUSE;
float translate = 0.0;
bool currSpecular;
bool zoomMode;

std::vector<Mesh*> objects;
Mesh* selected;
int specularMenuValue = 0;
int boundingBoxMenuValue = 1;
int wireframeMenuValue = 0;
unsigned int modelCount = 1;
unsigned int oldModelCount = 1;
GLuint vao[50];
int vaoIndex = 0;
GLuint program1, program2;
//comparator for findmax and findmin.
inline bool compareZ(glm::vec3 &a, glm::vec3 &b) { return a.z < b.z; }

//Takes a range and produces a random value whithin that range.
float randomRanged(float a, float b) {
	float random = ((float) rand()) / (float) RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}
void makeObjects(std::vector<int> list, float height){
	float positionX = -.5f;
	float positionY = -.45f;
	float boxWidth =  1.0f/list.size()/2.0f;
	float margin = boxWidth/2.0f;
	float scale = height/7.0f; //1  over the max
	for(int i = 0; i < list.size(); i++){
		float x = positionX + margin + boxWidth*i;
		std::vector<glm::vec3> line;
		line.push_back(glm::vec3(x, positionY, 0.0f));
		line.push_back(glm::vec3(x, positionY + list.at(i)*scale, 0.0f));
		Mesh* box = new Mesh(line);
		box->calculateNormals();
		box->createGLBuffer(false, vao, vaoIndex++);
		box->setupShader(program1, globalCamera);
		objects.push_back(box);
		//translate object so that top size is at x axis, scale y, translte back up
	}
}

void init()
{
	//Load shaders and use the resulting shader program
	program1 = Angel::InitShader("vshader.glsl", "fshader.glsl");
	program2 = Angel::InitShader("vshader.glsl", "fshader.glsl");
	//globalCamera = glm::perspective(35.0f, 1.0f, 0.01f, 200.0f); 
	globalCamera =glm::ortho (-.5f, .5f, -.5f, .5f, -100.0f, 100.0f);
	glGenVertexArrays(50, vao);
	
	//Questions: Should length of array be limited?
	std::vector<int> unsorted;
	unsorted.push_back(1);
	unsorted.push_back(2);
	unsorted.push_back(7);
	unsorted.push_back(3);

	makeObjects(unsorted, .75f);

	std::sort(unsorted.begin(), unsorted.end());
	//test deletion
	//objects.clear();
	//makeObjects(unsorted, .75f);

	//TODO: calculate new positions, translate there. Don't recreate objects.


	/*
	//Create mesh objects with different shader programs.
	Mesh* figure = new Mesh("woman.coor", "woman.poly");
	figure->calculateNormals();
	figure->createGLBuffer(false, vao, vaoIndex++);
	figure->setupShader(program1, globalCamera);
	figure->absoluteTranslate(-figure->getCenter());
	figure->scaleCenter(1/figure->getSize().y);
	objects.push_back(figure);
	*/
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(4.0f);
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

//---------Glut Callback Functions------------
void display()
{
	if(currentLighting != prevLighting){
		for(auto i = objects.begin(); i < objects.end(); i++)
			(*i)->setLighting(currentLighting, currSpecular);
		prevLighting = currentLighting;
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glClearDepth(1.0);
	for(auto i = objects.begin(); i < objects.end(); i++)
		(*i)->draw();
	TwDraw();
	glutSwapBuffers();
	glutPostRedisplay();
}

glm::vec4 viewPort;

glm::vec3 objCoordsNew, objCoordsOld;
glm::vec3 winCoordNew, winCoordsOld;
int prevX;
int prevY;
GLfloat depth;
//Uses color picking.
void mouseSelect(int button, int state, int x, int y){
	//Send event to atb. If it didn't handle it, then call our own code.
	if(!TwEventMouseButtonGLUT(button, state, x, y)){
		zoomMode = false;
		selected = NULL;
		zoomMode = false;
		prevX = x;
		prevY = winHeight - y - 1;
		if(state != GLUT_DOWN) //don't do anything if mouse is up.
			return;
		if(button == GLUT_RIGHT_BUTTON){
			zoomMode = true;
			return;
		}
		//else if(button = GLUT_LEFT_BUTTON) 

		unsigned char color[3];
		//Turn off lighting.
		for(auto i = objects.begin(); i < objects.end(); i++)
			(*i)->setLighting(COLOR_ID, false);
		
		//Render the objects.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
		glClearDepth(1.0);
		for(auto i = objects.begin(); i < objects.end(); i++)
			(*i)->draw();
		//Read color of mouse coordinate.
		glReadPixels(x, winHeight - y - 1, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, color);
		//We now use depth for translation because perspective makes farther things smaller.
		glReadPixels(x, winHeight - y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
		for(auto i = objects.begin(); i < objects.end(); i++){
			if((*i)->colorMatch(color))
				selected = *i;
		}
		if(selected != NULL){
			//iniitalize "previous"
			winCoordsOld = glm::vec3(prevX, prevY, depth);
			objCoordsOld = selected->windowToWorld(winCoordsOld, viewPort);

			viewPort = glm::vec4(0.0f, 0.0f, winWidth, winHeight);
			if(selected->getCurrentShading() == FLAT)
				selected->changeShading(SMOOTH);
			else 
				selected->changeShading(FLAT);
		}
		//Restore lighting.
		for(auto i = objects.begin(); i < objects.end(); i++)
			(*i)->setLighting(currentLighting, currSpecular);
	}
}


void reshape(int width, int height)
{
	winWidth = width;
	WINDOW_BORDER = HEIGHT - height;
	winHeight = height;
	std::cout << winHeight << std::endl;
	glViewport(0, 0, width,height);
	TwWindowSize(width, height);
}

void keyboard(unsigned char key, int x, int y)
{
	if(!TwEventKeyboardGLUT(key, x, y)){
		switch (key) {
		case 's':
			currentTransform = SCALE;
			break;
		case 't':
			currentTransform = TRANSLATE;
			break;
		case 'v':
			currentTransform = ROTATE_V;
			break;
		case 'h':
			currentTransform = ROTATE_H;
			break;
		}
	}
}

float direction;
void passiveMouse(int x, int y){
	TwEventMouseMotionGLUT(x, y);
}

const float SCALE_FACTOR = .02f;
const float ROTATION_FACTOR = 3.0f;

void activeMouse(int x, int y){
	if(!TwEventMouseMotionGLUT(x, y)){
		float differenceY = (float)(winHeight - y - 1 - prevY);
		float differenceX = (float)(prevX - x);

		float scale, rotation;
		bool positive = true;
		if(zoomMode == true){
			glm::vec3 movement;
			if(differenceY < 0.0f){
				movement = glm::vec3(0.0f, 0.0f, -.01f);
			}
			else {
				movement = glm::vec3(0.0f, 0.0f, .01f);
			}
			for(auto i = objects.begin(); i < objects.end(); i++){
				//very inneficient should probably make camera stuff external.
				(*i)->moveCamera(movement);
			}
		}
		else if(selected != NULL){
			switch(currentTransform){
				case SCALE:
					scale = 1.0f - SCALE_FACTOR;
					if(differenceY > 0.0f)
						scale = SCALE_FACTOR + 1.0f;
					differenceY = abs(differenceY);
					if(differenceY > 1.0)
						selected->scaleCenter(scale);
					break;
				case TRANSLATE:
						winCoordNew = glm::vec3(x, winHeight - y - 1, depth);
						objCoordsNew = selected->windowToWorld(winCoordNew, viewPort);
						std::cout << "xPrev: " << prevX << ", " << prevY << std::endl;
						std::cout << objCoordsNew.x << " " << objCoordsNew.y << std::endl;
						selected->absoluteTranslate(glm::vec3(objCoordsNew.x - objCoordsOld.x, objCoordsNew.y - objCoordsOld.y, 0.0f));
						objCoordsOld = objCoordsNew;
					break;
				case ROTATE_V:
					rotation = ROTATION_FACTOR;
					if(differenceX < 0.0f){
						positive = false;
					}
						selected->rotateSelf(glm::vec3(0.0f, rotation, 0.0f), positive);
					break;
				case ROTATE_H:
					rotation = ROTATION_FACTOR;
					if(differenceY < 0.0f){
						positive = false;
					}
						selected->rotateSelf(glm::vec3(rotation, 0.0f, 0.0f), positive);
					break;
			}
		}
		prevX = x;
		prevY = winHeight - y - 1;
	}
}

void Terminate()
{ 
	TwTerminate();
}

//--Menu Callback Functions--
void TW_CALL SetWireFrameCB(const void *value, void *clientData){
	(void)clientData;
	wireframeMenuValue = *(const int *)value;
	if( wireframeMenuValue!= 0 ) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void TW_CALL GetWireFrameCB(void *value, void *clientData){
	(void)clientData;
	*(int *)value = wireframeMenuValue;
}

//Toggles specularity.
void TW_CALL SetSpecularCB(const void *value, void *clientData)
{
	(void)clientData; // unused

	specularMenuValue = *(const int *)value;
	if( specularMenuValue!= 0 ) {
		for(auto i = objects.begin(); i < objects.end(); i++)
			(*i)->setLighting(currentLighting, true);
		currSpecular = true;
	}
	else{
		for(auto i = objects.begin(); i < objects.end(); i++)
			(*i)->setLighting(currentLighting, false);
		currSpecular = false;
	}
}

void TW_CALL GetSpecularCB(void *value, void *clientData)
{
	(void)clientData;
	*(int *)value = specularMenuValue;
}

//Toggles bounding box
void TW_CALL SetBoundingBoxCB(const void *value, void *clientData)
{
	(void)clientData; // unused

	boundingBoxMenuValue = *(const int *)value;
	if( boundingBoxMenuValue!=0 ) {
		for(auto i = objects.begin(); i < objects.end(); i++)
			(*i)->drawBoundingBox();
	}
	else{
		for(auto i = objects.begin(); i < objects.end(); i++)
			(*i)->removeBoundingBox();
	}
}

void TW_CALL GetBoundingBoxCB(void *value, void *clientData)
{
	(void)clientData;
	*(int *)value = boundingBoxMenuValue;
}


int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, HEIGHT);
	glutCreateWindow("160 Project Prototype");
	glutDisplayFunc(display);
	glutMouseFunc(mouseSelect);
	//glutCreateMenu(NULL);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glewExperimental = GL_TRUE;
	glewInit();
	

	//glutMouseFunc((GLUTmousebuttonfun)TwEventMouseButtonGLUT);
	glutMotionFunc(activeMouse);
	glutPassiveMotionFunc(passiveMouse);
	glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);
	glutKeyboardFunc(keyboard);
	atexit(Terminate);
	//TwGLUTModifiersFunc(glutGetModifiers);
	glutReshapeFunc(reshape);
	

	//Initialize AntTweakBar
	TwInit(TW_OPENGL, NULL);
	TwBar* bar = TwNewBar("OptionBar");
	TwDefine(" OptionBar position='0 10' size='220 300' color='25 113 255' label='Options' ");

	TwAddVarCB(bar, "WireFrame", TW_TYPE_BOOL32, SetWireFrameCB, GetWireFrameCB, NULL, " key='w' help='Toggle Wireframe' ");
	{
		TwEnumVal lightingEV[Lighting_MAX] = { {DIFFUSE, "Diffuse"}, {AMBIENT, "Ambient"}, {DIFFUSE_AND_AMBIENT, "Both"} };
		TwType lightingEnum = TwDefineEnum("LightingType", lightingEV, Lighting_MAX);
		TwAddVarRW(bar, "Lighting", lightingEnum, &currentLighting, " keyIncr=',' keyDecr='.' help='Select Lighting Type.' ");
	}

	TwAddVarCB(bar, "Specular", TW_TYPE_BOOL32, SetSpecularCB, GetSpecularCB, NULL, " key=space help='Toggle Specular Lighting' ");
	TwAddVarCB(bar, "Bounding Box", TW_TYPE_BOOL32, SetBoundingBoxCB, GetBoundingBoxCB, NULL, " key='b' help='Toggle Visibility of bounding box' ");
	TwAddVarRW(bar, "Number of Models", TW_TYPE_UINT32, &modelCount, " min=1 max=50 key='+' ");
	init();
	glutMainLoop();
	return 0;
}


/*
Really helpful for learning about managing array objects.
https://developer.apple.com/library/ios/#documentation/3DDrawing/Conceptual/OpenGLES_ProgrammingGuide/TechniquesforWorkingwithVertexData/TechniquesforWorkingwithVertexData.html
http://en.wikibooks.org/wiki/OpenGL_Programming/Bounding_box
*/