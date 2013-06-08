/* 
* Gerardo Perez
* Lab 7
* Contains some code from OpenGL Cookbook (in shader)
* 
* Clicking on an object will change its shading.
* Now includes zooming.
*/



///TODO instead of having a camera per object, create a scene object

#include "Angel.h"
#include "Mesh.h"
#include "Animation.h"
#include "Camera.h"
#include "glm/glm.hpp"
#include <Windows.h>

#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <string>
#include <list>
#include <AntTweakBar.h>

int WINDOW_BORDER;
const int HEIGHT = 800;
int winWidth;
int winHeight;

typedef enum {SCALE=1, ROTATE_V, ROTATE_H, TRANSLATE} TransformMode;
Camera* globalCamera;
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
int wireframeMenuValue = 0;
unsigned int modelCount = 1;
unsigned int oldModelCount = 1;
GLuint vao[50];
std::vector<GLuint> shaderPrograms;
int vaoIndex = 0;


/*
How to cycle animations:
	call update on each one
	if it ends and doesnt have a link to another one, destroy it
		otherwise switch the pointer to this one to the other one.

*/
std::list<Animation> animations;

void updateAnimations(int time){
	auto i = animations.begin();
	//dont want to call update on the linked animations.
	auto initialEnd = animations.end();
	while(i != initialEnd){
		bool hasEnded = (*i).hasEnded();
		if(hasEnded){
			if((*i).getLink() != 0)
				animations.push_back(*(*i).getLink()); //need to make sure it doesnt activate.
			i = animations.erase(i); //update iterator to be after erased oneS
		}
		else {
			(*i).update(time);
			++i;
		}
	}
}

//comparator for findmax and findmin.
inline bool compareZ(glm::vec3 &a, glm::vec3 &b) { return a.z < b.z; }

//Takes a range and produces a random value whithin that range.
float randomRanged(float a, float b) {
	float random = ((float) rand()) / (float) RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

/*
	1. Bring out both. A into positive Z axis, B into negative Z axis.
	2. Take current locations of both, swap positions excluding z axis. Duration is same for both.
	3. Push back into line.
*/

float boxWidth;
//Idea, implement transparent block

//a unit of animation, can use this to dynamically change animation speed depending on distance travel
int animationDuration = 700;
void animationSwap(int a, int b){

	Mesh* meshA = objects.at(a);
	Mesh* meshB = objects.at(b);
	float displacementA = meshA->getSize().y*.5f;
	float displacementB = meshB->getSize().y*.5f;

	glm::vec3 aPosition = objects.at(a)->getCenter();
	glm::vec3 bPosition = objects.at(b)->getCenter();
	float z = aPosition.z;

	Animation offsetA(Animation::POSITION);
	offsetA.setStart(meshA, aPosition);
	offsetA.setGoal(glm::vec3(aPosition.x, aPosition.y, z + boxWidth), animationDuration/2);
	
	Animation switchA(Animation::POSITION);
	switchA.setStart(meshA, glm::vec3(aPosition.x, aPosition.y, z + boxWidth));
	switchA.setGoal(glm::vec3(bPosition.x, aPosition.y, z + boxWidth), animationDuration);

	Animation returnA(Animation::POSITION);
	returnA.setStart(meshA, glm::vec3(bPosition.x, aPosition.y, z + boxWidth));
	returnA.setGoal(glm::vec3(bPosition.x, aPosition.y, z), animationDuration/2);
	
	Animation offsetB(Animation::POSITION);
	offsetB.setStart(meshB, bPosition);
	offsetB.setGoal(glm::vec3(bPosition.x, bPosition.y, z - boxWidth), animationDuration/2);

	Animation switchB(Animation::POSITION);
	switchB.setStart(meshB, glm::vec3(bPosition.x, bPosition.y, z - boxWidth));
	switchB.setGoal(glm::vec3(aPosition.x, bPosition.y, z - boxWidth), animationDuration);

	Animation returnB(Animation::POSITION);
	returnB.setStart(meshB, glm::vec3(aPosition.x, bPosition.y, z - boxWidth));
	returnB.setGoal(glm::vec3(aPosition.x, bPosition.y, z), animationDuration/2);

	//chain sort of in reverse because of copy by value. consider changin
	switchA.chain(returnA);
	offsetA.chain(switchA);

	switchB.chain(returnB);
	offsetB.chain(switchB);
	
	//Note that these will go out of scope, so pass a copy to mesh, not a reference 
	animations.push_back(offsetA);
	animations.push_back(offsetB);
}

//TODO: Allow custom z axis location
void makeObjects(std::vector<int> list, float height){
	float startingX = -.5f;
	float positionY = -.5f;
	boxWidth =  1.0f/list.size()/2.0f;
	float margin = boxWidth/2.0f;
	//get max value
	float yScale = height/(*std::max_element(list.begin(), list.end())); //height over the max
	for(int i = 0; i < list.size(); i++){
		float x = startingX + boxWidth*.5f +  margin*(i+1) + boxWidth*i;
		Mesh* box = new Mesh("cube.coor", "cube.poly");
		box->calculateNormals();
		box->createGLBuffer(false, vao, vaoIndex);
		box->setupShader(shaderPrograms[vaoIndex], globalCamera);
		box->removeBoundingBox();
		//scale, translate new height/2 on y axis. Perhaps need to update dimensions after scale.
		box->scaleCenter(glm::vec3(boxWidth, list.at(i)*yScale, boxWidth));
		box->moveTo(glm::vec3(x, positionY + box->getSize().y*.5f, 0.0f));
		objects.push_back(box);
		vaoIndex++;
		//translate object so that top size is at x axis, scale y, translte back up
	}
}

Animation* slide;
void init()
{
	//globalCamera = glm::perspective(35.0f, 1.0f, 0.01f, 200.0f); 
	glm::mat4 ortho = glm::ortho (-0.5f, 0.5f, -0.5f, 0.5f, 0.01f, 100.0f);
	globalCamera = new Camera(ORTHOGRAPHIC, ortho);
	globalCamera->moveTo(glm::vec3(1.0f, 1.0f, 1.0f));
	//Questions: Should length of array be limited?
	std::vector<int> unsorted;
	unsorted.push_back(1);
	unsorted.push_back(4);
	unsorted.push_back(3);
	unsorted.push_back(2);

	glGenVertexArrays(50, vao);
	for(auto i = unsorted.begin(); i < unsorted.end(); i++){
		// this is really only necessary for picking, switch to one shader later
		shaderPrograms.push_back(Angel::InitShader("vshader.glsl", "fshader.glsl"));
	}
	makeObjects(unsorted, .75f);
	slide = new Animation (Animation::POSITION);
	glm::vec3 center = objects.at(0)->getCenter();
	std::cout << "Current x " << center.x << "current y " << center.y;
	slide->setStart(objects.at(0), center);
	slide->setGoal(glm::vec3(center.x, center.y, .25f), 200);
	animationSwap(0, 1);
	/*test deletion
	objects.clear();
	std::sort(unsorted.begin(), unsorted.end());
	vaoIndex = 0;
	makeObjects(unsorted, .75f);
	*/
	glPolygonMode(GL_FRONT, GL_FILL);
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
	int time = glutGet(GLUT_ELAPSED_TIME);
	//slide->update(time);
	updateAnimations(time);
	for(auto i = objects.begin(); i < objects.end(); i++){
		//(*i)->update(time);
		(*i)->draw();
	}
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
			objCoordsOld = globalCamera->windowToWorld(winCoordsOld, viewPort);

			viewPort = glm::vec4(0.0f, 0.0f, winWidth, winHeight);
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
	// std::cout << winHeight << std::endl;
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
			float zoomFactor;
			if(differenceY < 0.0f){
				zoomFactor = .95;
			}
			else {
				zoomFactor = 1.05f;
			}
				globalCamera->scale(zoomFactor);
		}
		else if(selected != NULL){
			switch(currentTransform){
				case SCALE:
					scale = 1.0f - SCALE_FACTOR;
					if(differenceY > 0.0f)
						scale = SCALE_FACTOR + 1.0f;
					differenceY = abs(differenceY);
					if(differenceY > 1.0)
						selected->scaleCenterUniform(scale);
					break;
				case TRANSLATE:
						winCoordNew = glm::vec3(x, winHeight - y - 1, depth);
						objCoordsNew = globalCamera->windowToWorld(winCoordNew, viewPort);
						// std::cout << "xPrev: " << prevX << ", " << prevY << std::endl;
						// std::cout << objCoordsNew.x << " " << objCoordsNew.y << std::endl;
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