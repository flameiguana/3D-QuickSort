#include "QuickSortVisual.h"

//TODO: Allow custom z axis location
void QuickSortVisual::makeObjects(float height){
	float startingX = -.5f;
	float positionY = -.5f;
	float margin = boxWidth/2.0f;
	//get max value
	for(int i = 0; i < array.size(); i++){
		float x = startingX + boxWidth*.5f +  margin*(i+1) + boxWidth*i;
		//Allow custom objects?
		Mesh* box = new Mesh("cube.coor", "cube.poly");
		box->calculateNormals();
		box->createGLBuffer(false, vao, vaoIndex);
		box->setupShader(shaderPrograms[i], camera);
		box->removeBoundingBox();
		//scale, translate new height/2 on y axis. Perhaps need to update dimensions after scale.
		box->scaleCenter(glm::vec3(boxWidth, array.at(i)*yScale, boxWidth));
		box->moveTo(glm::vec3(x, positionY + box->getSize().y*.5f, 0.0f));
		objects.push_back(box);
		vaoIndex++;
		//translate object so that top size is at x axis, scale y, translte back up
	}
}

void QuickSortVisual::moveCompareIndicator(int location, bool animated)
{

	glm::vec3 goalPosition = objects.at(location)->getCenter();
	goalPosition.y +=  objects.at(location)->getSize().y*.5f + boxWidth;
	Animation move(Animation::POSITION);

	if(!animated){
		compareIndicator->moveTo(goalPosition);
		return;
	}

	move.setStart(compareIndicator, compareIndicator->getCenter());
	move.setGoal(goalPosition, animationDuration * 1.5f, Animation::ELASTIC_OUT);
	animations.push_back(move);
}

QuickSortVisual::QuickSortVisual(std::vector<int> values, Camera* camera):camera(camera){
	array = values;
	animationDuration = 700;
	poppedStack = false;
	havePivot = false;
	finished = false;
	vaoIndex = 0;
	left = 0;
	step = -1;
	scanner = 0;
	paused = false;
	myTime = 0;
	lastTime = 0;
	glGenVertexArrays(50, vao);
	//Compute various dimensions
	height = .75f;
	yScale = height/(*std::max_element(array.begin(), array.end())); //height over the max
	boxWidth =  1.0f/array.size()/2.0f;

	GLuint texturedShader = Angel::InitShader("textured.vert", "textured.frag");
	compareIndicator = new Mesh("cube.coor", "cube.poly", true);
	compareIndicator->calculateNormals();
	compareIndicator->createGLBuffer(false, vao, vaoIndex++);
	compareIndicator->loadTexture("indicator.png");
	compareIndicator->setupShader(texturedShader, camera);
	compareIndicator->removeBoundingBox();

	compareIndicator->scaleCenter(glm::vec3(boxWidth, boxWidth, boxWidth));
	
	for(auto i = array.begin(); i < array.end(); i++){
		// this is really only necessary for picking, switch to one shader later
		shaderPrograms.push_back(Angel::InitShader("vshader.vert", "fshader.frag"));
	}

	makeObjects(height);
	moveCompareIndicator(0, false);

	//Initialize quicksort algoritmn
	right = array.size() - 1;
	stack.push(left);
	stack.push(right);
}

void QuickSortVisual::swap(int a, int b){
	int temp = array.at(a);
	array.at(a) = array.at(b);
	array.at(b) = temp;
	Mesh* tempMesh = objects.at(a);
	objects.at(a) = objects.at(b);
	objects.at(b) = tempMesh;
}

void QuickSortVisual::markPivot(int i){
	objects.at(i)->setSpecular(glm::vec4(0.297254f,	0.30829f, 0.306678f, 1.0f), .1f*128.0f);
	objects.at(i)->setDiffuse(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.396f, 0.74151f, 0.69102f, 1.0f));
}

//Make this fancier later on.
void QuickSortVisual::focus(int a, int b){
	for(int i = 0; i < objects.size(); i++){
		if(i >= a && i <= b)
			objects.at(i)->setAlpha(1.0f);
		else
			objects.at(i)->setAlpha(.5f);
	}
}

/*
	1. Bring out both. A into positive Z axis, B into negative Z axis.
	2. Take current locations of both, swap positions excluding z axis. Duration is same for both.
	3. Push back into line.
*/
void QuickSortVisual::swapAnimation(int a, int b){

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
	
	animations.push_back(offsetA);
	animations.push_back(offsetB);
}

//this will only be called when animation queue is empty;
int QuickSortVisual::partitionAnimationStep(int left, int right, int step, int& scanner, int pivotIndex){
	
	markPivot(pivotIndex);
	moveCompareIndicator(scanner);
	if(step == -1){
		
		swap(pivotIndex, right);
		swapAnimation(pivotIndex, right);
		return scanner;
	}
	int pivotValue = array.at(right);
	//instead of having for loop, iterate by steps
	int i = left + step;
	if(i < right){
		std::cout << "Check if scanner <= pivotValue. " << std::endl;

		if(array.at(i) <= pivotValue){
			//highlight swapped ones
			//if i == scanner, don't do swap animation
			swapAnimation(i, scanner);
			std::cout << "Swapping array[" << i << "] with array[" << scanner <<"]" << std::endl;
			swap(i, scanner);
			scanner++;
			
		}
	}
	//i == rightfi
	else {
		swapAnimation(right, scanner);
		swap(right, scanner);
	}
	return scanner;
}

//A non recursive version of this shizzle
void QuickSortVisual::quickSortStep(int& left, int& right, int& step, int& scanner){
	//This is equivalent to while !stack.empty(), but it would be bad to check while partition
	//animation is in progress.
	if(!finished){
		if(!poppedStack){
			right = stack.top();
			stack.pop();
			left = stack.top();
			stack.pop();
			scanner = left; //Start partition function's scanner at 0
			focus(left, right);
			poppedStack = true;
		}
		if(step == -1){
			//get a random pivot
			//pivotIndex = left + (std::rand() % (right - left + 1));
			//For simplicity, always pick rightmost as pivot
			pivotIndex = right;
			//if we put back random, get rid of this
			step++;
		}
		//This means that partitionAnimation has finished.
		if(step > right - left){ //the extra step is for first pivot switch
			step = -1;
			std::cout << "Pivot is " << pivot << std::endl;
			poppedStack = false;
			std::cout << "Ready to Recurse" << std::endl;   
			havePivot = true;
			markPivot(pivot);
		}

		else
			pivot = partitionAnimationStep(left, right, step++, scanner, pivotIndex);
		//the value of this pivot value is ignored until here.
		if(havePivot){
			//Right side of pivot is added to stack if there are elements to right of it.
			if(pivot + 1 < right)
			{
				stack.push(pivot + 1);
				stack.push(right);
			}
			
			//Left side of pivot is added to stack in order to be partitioned.
			//It is equivalent to calling quicksort recursively.
			if(pivot -  1> left)
			{
				stack.push(left);
				stack.push(pivot - 1);
			}
			
			havePivot =  false;
			
			if(stack.empty())
				finished = true;
		}
	}
	else 
	{
		std::cout << "Done." << std::endl;
	}
}


/*
How to cycle animations:
	call update on each one
	if it ends and doesnt have a link to another one, destroy it
		otherwise switch the pointer to this one to the other one.

*/
void QuickSortVisual::updateAnimations(int time){
	auto i = animations.begin();
	//dont want to call update on the linked animations.
	std::list<Animation> links;
	while(i != animations.end()){
		// std::cout << "still animating" << std::endl;
		bool hasEnded = (*i).hasEnded();
		if(hasEnded){
			if((*i).containsLink())
				links.push_back(*(*i).getLink()); //need to make sure it doesnt activate.
			i = animations.erase(i); //update iterator to be after erased oneS
		}
		else {
			(*i).update(time);
			++i;
		}
	}
	//merge list of animation links
	animations.splice(animations.begin(), links);
}

void QuickSortVisual::update(int realTime){
	if(!paused){
		// Allows us to continue animation were we left of.
		myTime += realTime - lastTime;  
		updateAnimations(myTime);
		if(animations.size() == 0 && !finished){
			quickSortStep(left, right, step, scanner);
		}
	}
	lastTime = realTime;
}


void QuickSortVisual::draw(){

	compareIndicator->draw();
	for(auto i = objects.begin(); i < objects.end(); i++){
		(*i)->draw();
	}
}
