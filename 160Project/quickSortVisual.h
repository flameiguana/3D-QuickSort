#pragma once

#include "Mesh.h"
#include "Animation.h"
#include "Angel.h"
#include <vector>
#include <stack>
#include <list>
#include <cmath>
#include <memory>

class QuickSortVisual{
public:
	QuickSortVisual(const std::vector<int>& values, Camera* camera);
	~QuickSortVisual();
	void update(int time);
	void draw();
	void setPause(bool value){paused = value;}
	//scales animation speed. read as a percentange
	void scaleSpeed(float speed){
		animationScale = 100 / speed;
		animationDuration = ANIMATION_UNIT  / (float)array.size() * animationScale;
	}
	void stepOnce(){stepMode = true;}
	std::vector<std::shared_ptr<Mesh>> * getObjects(){return &objects;}

private:
	std::vector<std::shared_ptr<Mesh>> objects;
	std::list<Animation> animations;
	/* Algorithmn Variables */
	std::vector<int> array;
	//Stack used in iterative quickSort
	std::stack<int> stack;
	int scanner, step, left, right, pivot;
	bool poppedStack, havePivot, finished;

	/*3D variables*/
	Camera* camera;
	Mesh compareIndicator;
	Mesh indexIndicator;
	Mesh blank;
	float boxWidth, height, yScale;
	
	GLuint* vao; //pointer to array of vertex array objects
	std::vector<GLuint> shaderPrograms;
	int vaoIndex;

	//a unit of animation, can use this to dynamically change animation speed depending on distance traveled
	static const float ANIMATION_UNIT;
	float animationDuration;
	float animationScale;
	int lastTime, myTime;
	int previousScanner;
	int pivotIndex;
	bool paused;
	
	bool stepMode;
	void swapAnimation(int a, int b);
	void swap(int a, int b);
	int partitionAnimationStep(int left, int right, int step, int& scanner, int pivotIndex);
	void updateAnimations(int time);
	void focus(int a, int b);
	void markPivot(int pivot);
	void moveCompareIndicator(int original, int destination, bool animated = true);
	void moveIndexIndicator(int location, bool delay,  bool animated = true);
	void quickSortStep(int& left, int& right, int& step, int& scanner);
	void makeObjects(float height);
};
