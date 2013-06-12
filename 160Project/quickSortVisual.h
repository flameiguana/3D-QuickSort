#pragma once

#include "Mesh.h"
#include "Animation.h"
#include <vector>
#include <stack>
#include <list>

class QuickSortVisual{
public:
	QuickSortVisual(std::vector<int> values, Camera* camera);
	void update(int time);
	void draw();
	void setPause(bool value){paused = value;}

	std::vector<Mesh*> * getObjects(){return &objects;}

private:
	std::vector<Mesh*> objects;
	std::list<Animation> animations;
	/* Algorithmn Variables */
	std::vector<int> array;
	//Stack used in iterative quickSort
	std::stack<int> stack;
	int scanner, step, left, right, pivot;
	bool poppedStack, havePivot, finished;

	/*3D variables*/
	Camera* camera;
	Mesh* comparePointer;
	float boxWidth;
	GLuint vao[50];
	std::vector<GLuint> shaderPrograms;
	int vaoIndex;

	//a unit of animation, can use this to dynamically change animation speed depending on distance traveled
	int animationDuration;
	int lastTime, myTime;
	int pivotIndex;
	bool paused;
	void swapAnimation(int a, int b);
	void swap(int a, int b);
	int partitionAnimationStep(int left, int right, int step, int& scanner, int pivotIndex);
	void updateAnimations(int time);
	void focus(int a, int b);
	void markPivot(int pivot);
	void moveComparePointer(int location);
	void quickSortStep(int& left, int& right, int& step, int& scanner);
	void makeObjects(float height);
};
