#include "Mesh.h"



inline bool comparevec3(glm::vec3 &a, glm::vec3 &b){
	return a.x == b.x && a.y == b.y && a.z == b.z;
}


template <class T>
bool contains(const std::vector<T> &vec, const T &value)
{
	return std::find(vec.begin(), vec.end(), value) != vec.end();
}

void clearQueue( std::queue<int> &q )
{
	std::queue<int> empty;
	std::swap(q, empty );
}

//splits a string
std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

//Represents a vertex.
class Mesh::Vertex{
public:
	//The list of polygons that this vertex lies in.
	std::vector<glm::vec3*> adjacentNormals;
	//The actual vertex location.
	glm::vec3 *point;
	//The vertex normal
	glm::vec3 normal;
	//glm::vec3 *surfaceNormal; cant hold surface normal because these points are shared
	//static int id;
	Vertex(glm::vec3* point):point(point){
		//Vertex::id++;
	}

	void addAdjacentNormal(glm::vec3* normal){
		adjacentNormals.push_back(normal);
	}

	//Averages normals of adjacent polygons.
	void calculateNormal(){
		for(auto i = adjacentNormals.begin(); i < adjacentNormals.end(); i++){
			normal = normal + **i;
		}
		normal = normal/(float)adjacentNormals.size();
		normal = glm::normalize(normal);
		//std::cout << normal.x << std::endl;
	}
};

//Represents a triangle face.
class Mesh::Polygon{
public:
	//friend class Mesh::Vertex;
	//The face normal;
	glm::vec3 normal;
	//raw vertex coordinates that this polygon is made of.
	std::vector<glm::vec3*> vertices;
	int size;
	//When creating the polygon, calculate normal.
	Polygon():size(0){}

	void addVertex(glm::vec3* vertex){
		vertices.push_back(vertex);
		size++;
	}

	void calculateNormal(){
		if (size < 3)
			std::cout << "Warning: can't calculate normal with less than three vertices." << std::endl;
		else
		{
			normal = glm::normalize(glm::cross(  *vertices.at(2) - *vertices.at(0), *vertices.at(1) - *vertices.at(0) ));
		}
	}

	//Checks if this polygon contains the specified point.
	bool contains(glm::vec3* point){
		for(int i = 0; i < vertices.size(); i++){
			if(comparevec3(*vertices.at(i), *point))
				return true;
		}
		return false;
	}
};

class Mesh::BBox{
public:
	GLuint vboVertices;
	GLuint iboElements;
	glm::mat4 transform;
	//call this when creating buffers, use same vao
	BBox(glm::vec3 min, glm::vec3 max, glm::vec3 centerPoint, glm::vec3 size){
		//Create a unit cube.
		GLfloat vertices[] = {
    		-0.5, -0.5, -0.5,
    		 0.5, -0.5, -0.5,
    		 0.5,  0.5, -0.5,
    		-0.5,  0.5, -0.5,
    		-0.5, -0.5,  0.5,
    		 0.5, -0.5,  0.5,
    		 0.5,  0.5,  0.5,
    		-0.5,  0.5,  0.5,
		};

		GLushort elements[] = {
			0, 1, 2, 3,
	    	4, 5, 6, 7,
	    	0, 4, 1, 5, 2, 6, 3, 7
		};

		/*
		std::cout << min.x << " " << min.y << " " << min.z << " " << std::endl;
		std::cout << max.x << " " << max.y << " " << max.z << " " << std::endl;
		*/
		//Generate the buffers
		glGenBuffers(1, &vboVertices);
		glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind buffer 

		glGenBuffers(1, &iboElements);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboElements);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		
		
		transform =  glm::translate(glm::mat4(1.0f), centerPoint) * glm::scale(glm::mat4(1.0f), size);
	}
	//pass in transformation. wanna calls this before drawing object
	//transformation matrix will need to be restored in object's draw
	void draw(GLuint vPosition_loc, GLuint mTransformation_loc, glm::mat4& parentTransformation){
		//Apply same matrix that is transforming object, and in addition apply our own
		//transform matrix.
		glm::mat4 M = parentTransformation * transform;
		glUniformMatrix4fv(mTransformation_loc, 1, GL_FALSE, glm::value_ptr(M));

		glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
		//need vposition pointer in shader (vPosition)
		glVertexAttribPointer(vPosition_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(vPosition_loc);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboElements);
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4*sizeof(GLushort)));
		glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8*sizeof(GLushort)));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//could restore original tranformation here

		glDisableVertexAttribArray(vPosition_loc);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		//Don't want to delete buffers.
	}
};
//Assigns a unique color ID to this mesh.
void Mesh::setColorID(){
	colorID[0] = globalColorID[0];
    colorID[1] = globalColorID[1];
    colorID[2] = globalColorID[2];
    globalColorID[0] += 1;
    if(globalColorID[0] > 255)
    {
         globalColorID[0] = 0;
         globalColorID[1]++;
         if(globalColorID[1] > 255)
         {
              globalColorID[1] = 0;
              globalColorID[2]++;
         }
    }
}

bool Mesh::colorMatch(unsigned char *color){
	return (colorID[0] == color[0] && colorID[1] == color[1] && colorID[2] == color[2]);
}

//Note, polygons have variable number of vertices.
Mesh::Mesh(const std::string &coords, const std::string &polys, bool isTextured){
	//TODO Create bounding box.
	//Calculate middle point
	//Stick vertices into same buffer.
	//Draw part of object (up to a certain index) using GL_TRIANGLES, and then draw the box
	this->isTextured = isTextured;
	setColorID();
	//Indicates that we know which vetices are associated with each polygon.
	smartPolys = true;
	//load the vertices. they are in order
	std::ifstream file;
	std::string unused;
	file.open(coords.c_str());
	int totalVerts;
	int indexNum;
	float x, y, z;
	char junk;
	file >> totalVerts;
	rawCoords.reserve(totalVerts);
	rawVerts.reserve(totalVerts);
	vertexList.reserve(totalVerts);
	for(int i = 0; i < totalVerts; i++){
		file >> indexNum;
		file >> junk;
		file >> x;
		file >> junk;
		file >> y;
		file >> junk;
		file >> z;
		rawCoords.push_back(glm::vec3(x, y, z));
		//these will be out of order
		vertexList.push_back(new Vertex(&rawCoords.back()));
		
		//initialize max and min
		if(i == 0){
			maxCoords = rawCoords.back();
			minCoords = rawCoords.back();
		}
		//Look for max and min points.
		if(x > maxCoords.x)
			maxCoords.x = x;
		else if(x < minCoords.x)
			minCoords.x = x;
		if(y > maxCoords.y)
			maxCoords.y = y;
		else if(y < minCoords.y)
			minCoords.y = y;
		if(z > maxCoords.z)
			maxCoords.z = z;
		else if(z < minCoords.z)
			minCoords.z = z;
	}
	size = glm::vec4(maxCoords.x - minCoords.x, maxCoords.y - minCoords.y, maxCoords.z - minCoords.z, 1.0f);
	center = glm::vec3((minCoords.x + maxCoords.x)/2.0f, (minCoords.y + maxCoords.y)/2.0f, (minCoords.z + maxCoords.z)/2.0f);
	//std::cout << "Center of figure "<< center.x << " " << center.y << " " << center.z << std::endl;
	currentCenter = center;
	file.close();
	//Load the polygons
	file.open(polys.c_str());
	int totalPolys;
	std::string section;
	std::string sectionPrev;
	std::string line;
	std::vector<std::string> splitLine;

	file >> totalPolys;
	std::getline(file, unused);

	//These are the texture coordinates of a face, taking into account that it
	//it is going to be triangulated using my algorihtmn. Basically it only works for cubes.
	std::vector<glm::vec2> originals;
	originals.push_back(glm::vec2(0.0f, 0.0f));
	originals.push_back(glm::vec2(1.0f, 0.0f));
	originals.push_back(glm::vec2(1.0f, 1.0f));
	originals.push_back(glm::vec2(0.0f, 0.0f));
	originals.push_back(glm::vec2(1.0f, 1.0f));
	originals.push_back(glm::vec2(0.0f, 1.0f));
	originals.push_back(glm::vec2(0.0f, 0.0f));

	/*
	Vertex Normals are handled correctly.
	Creates correct amount of surface normals
	*/

	//modify to make triangles
	for(int i  = 0; i < totalPolys; i++){
		Polygon* poly = new Polygon(); 
		std::getline(file, line);
		//std::cout << line << std::endl;
		split(line, ' ', splitLine);
		section = splitLine.at(0); //that label at the beginning of the part.
		int first = atoi((splitLine.at(1).c_str())) -1; //first point. Use it as first point for all triangles.
		poly->addVertex(&rawCoords.at(first));

		int current = -1;
		int previous;
		int count = 1;
		int textureIndex = 0;
		for(auto j = splitLine.begin() + 2; j < splitLine.end(); j++){
			
			previous = current;
			current = atoi((*j).c_str()) - 1;
			poly->addVertex(&rawCoords.at(current));
			
			if(count > 1){
				if(count == 2){
					poly->calculateNormal();
				}
				rawVerts.push_back(rawCoords.at(first));
				rawVerts.push_back(rawCoords.at(previous));
				rawVerts.push_back(rawCoords.at(current));

				//Very hard-coded
				textureCoords.push_back(originals.at(textureIndex++));
				if(textureIndex == originals.size())
					textureIndex = 0;
				textureCoords.push_back(originals.at(textureIndex++));
				textureCoords.push_back(originals.at(textureIndex++));

				//Since the verts are in order, it would be good to push the appropriate coordinates directly.
				surfaceNormals.push_back(poly->normal);
				surfaceNormals.push_back(poly->normal);
				surfaceNormals.push_back(poly->normal);

				vertexList.at(first)->addAdjacentNormal(&(poly->normal)); //associate poly normal with vertex
				vertexNRefs.push_back(&vertexList.at(first)->normal); //we only keep references until its time to calculate
				vertexList.at(previous)->addAdjacentNormal(&(poly->normal));
				vertexNRefs.push_back(&vertexList.at(previous)->normal);
				vertexList.at(current)->addAdjacentNormal(&(poly->normal));
				vertexNRefs.push_back(&vertexList.at(current)->normal);
			}
			count++;
		}
		polygons.push_back(poly);
		splitLine.clear();
	}
	/*
	std::cout << "Polygon Count " << polygons.size() << std::endl;
	std::cout << "Surface Normal Count " << surfaceNormals.size() << std::endl; //17832
	
	std::cout << "Point Count " << rawVerts.size() << std::endl; //17832
	std::cout << "Texture Points " << textureCoords.size() << std::endl;
	*/
	file.close();
}

//This constructor takes a vector of glm::vec3 and generates vertex and polygon objects.
Mesh::Mesh(std::vector<glm::vec3> &vertices){
	setColorID();
	smartPolys = false;

	rawVerts = vertices;
	for(int i = 0; i < vertices.size(); i++){

		if(i == 0){
			maxCoords = vertices.front();
			minCoords = vertices.front();
		}

		if(vertices.at(i).x > maxCoords.x)
			maxCoords.x = vertices.at(i).x;
		if(vertices.at(i).x < minCoords.x)
			minCoords.x = vertices.at(i).x;
		if(vertices.at(i).y > maxCoords.y)
			maxCoords.y = vertices.at(i).y;
		if(vertices.at(i).y < minCoords.y)
			minCoords.y = vertices.at(i).y;
		if(vertices.at(i).z > maxCoords.z)
			maxCoords.z = vertices.at(i).z;
		if(vertices.at(i).z < minCoords.z)
			minCoords.z = vertices.at(i).z;

		Vertex* vertex  = new Vertex(&rawVerts.at(i));
		vertexList.push_back(vertex);
		//Every three points, create a new polygon object.
		if((i + 1) % 3 == 0){
			Polygon* poly = new Polygon();
			for(int j = 0; j < 3; j++){
				poly->addVertex(&(rawVerts.at(i - 2 +  j)));
			}
			polygons.push_back(poly); //something bad happens here.
		}
	}
	center = glm::vec3((minCoords.x + maxCoords.x)/2.0f, (minCoords.y + maxCoords.y)/2.0f, (minCoords.z + maxCoords.z)/2.0f);
	//center = size/2.0f;
	std::cout << "Center of terrain "<< center.x << " " << center.y << " " << center.z << std::endl;
	currentCenter = center;
	
}

/*
	Uses two different ways of calculating normals, depending on if a file was read.
*/
void Mesh::calculateNormals()
{
	if(smartPolys == false){
		
		for(auto p = polygons.begin(); p < polygons.end(); p++){
			(**p).calculateNormal();
			for(int i = 0; i < 3;  i++){
				surfaceNormals.push_back((**p).normal);
			}
		}

		for(auto v = vertexList.begin(); v < vertexList.end(); v++){
			for(auto p = polygons.begin(); p < polygons.end(); p++){
				if((**p).contains((**v).point)){
					(**v).addAdjacentNormal(&(**p).normal);
				}
			}
		(**v).calculateNormal();
		vertexNormals.push_back((**v).normal);
		}
	}
	else{
		//surface normals were already provided. Now just calculate normal per vertex
		for(auto v = vertexList.begin(); v < vertexList.end(); v++){
			(**v).calculateNormal();
		}
		for(auto n = vertexNRefs.begin(); n < vertexNRefs.end(); n++){
			vertexNormals.push_back(**n);
		}
	}
}

//There is one vertex array object per mesh.
void Mesh::createGLBuffer(bool smooth, GLuint* vao, int index){
	//Create a vertex array
	vaoIndex = index;
	this->vao = vao;
	glBindVertexArray(vao[index]);

	boundingBox = new BBox(minCoords, maxCoords, center, glm::vec3(size));

	sizeofVertices = rawVerts.size()*sizeof(glm::vec3);
	sizeofNormals = surfaceNormals.size()*sizeof(glm::vec3);
	sizeofTCoords = 0;
	if(isTextured)
		sizeofTCoords = textureCoords.size()*sizeof(glm::vec2);
	
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeofVertices + sizeofNormals + sizeofTCoords, NULL, GL_DYNAMIC_DRAW);

	//Hold the points to draw here
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeofVertices, rawVerts.data());
	//Hold the normals here
	if(smooth){
		glBufferSubData(GL_ARRAY_BUFFER, sizeofVertices, sizeofNormals, vertexNormals.data());	
	}
	else{
		glBufferSubData(GL_ARRAY_BUFFER, sizeofVertices, sizeofNormals, surfaceNormals.data());		
	}
	
	if(isTextured){
		glBufferSubData(GL_ARRAY_BUFFER, sizeofNormals + sizeofVertices, sizeofTCoords, textureCoords.data());
	}
	//Create the bounding box buffers. Current vao is still bound.
	
	//Unbind buffers
    glBindVertexArray(0);
}

//Should pass in names as parameters.
void Mesh::setupShader(GLuint& _program, Camera* camera_){
	currentShading = FLAT;
	this->camera = camera_;
	wireframe = false;
	glBindVertexArray(vao[vaoIndex]);
	glUseProgram(_program);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	//Initialize the vertex position attribute from the vertex shader

	vPosition = glGetAttribLocation(_program, "vPosition");
	vNormal = glGetAttribLocation(_program, "vNormal"); 
	if(isTextured)
		vertexUV = glGetAttribLocation(_program, "vertexUV");

	const float maxValue = 255.0;
	//This stuff is hard coded.-------------
	glm::vec4 materialAmbient(.3, .3, .3, .2);
	materialSpecular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	glm::vec4 lightAmbient(0.0f, 0.0f, 0.0f, 0.0f);
	lightSpecular = glm::vec4 (1.0f, 1.0f, 1.0f, 1.0f); //A bright red specular color
	lightDiffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); //white

	shininess = .25f* 128.0f;
	//To get diffuse lighting, calculate angle of each triangle.
	//So get the surface normal of every three points.
	glm::vec4 ambientProduct = lightAmbient * materialAmbient;

	//Get uniform variable locations.
	lightDiffuse_loc = glGetUniformLocation(_program, "LightDiffuse");
	materialDiffuse_loc = glGetUniformLocation(_program, "DiffuseReflect");
	materialDiffuse = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	GLuint ambientProduct_loc = glGetUniformLocation(_program, "AmbientProduct");
	lightSpecular_loc = glGetUniformLocation(_program, "LightSpecular");
	materialSpecular_loc = glGetUniformLocation(_program, "MaterialSpecular");
	lightPosition_loc = glGetUniformLocation(_program, "LightPosition");
	shininess_loc = glGetUniformLocation(_program, "Shininess");
	GLuint colorID_loc = glGetUniformLocation(_program, "colorID");
	//Set uniform variables
	glUniform4fv(ambientProduct_loc, 1, glm::value_ptr(ambientProduct));
	glUniform4fv(lightDiffuse_loc, 1, glm::value_ptr(lightDiffuse));
	glUniform4fv(materialDiffuse_loc, 1, glm::value_ptr(materialDiffuse));
	glUniform4fv(lightSpecular_loc, 1, glm::value_ptr(lightSpecular));
	glUniform4fv(materialSpecular_loc, 1, glm::value_ptr(materialSpecular));
	//Dont know why this only works when negative. 4th element is zero to signify directional
	lightPosition = glm::vec4(0.0, 0.8, 1.0, 0.0);

	glUniform4fv(lightPosition_loc, 1, glm::value_ptr(lightPosition));
	glUniform4fv(colorID_loc, 1, glm::value_ptr(glm::vec4(colorID[0]/255.0f, colorID[1]/255.0f, colorID[2]/255.0f, 1.0f)));
	glUniform1f(shininess_loc, shininess);

	//Set up pointers to subroutines in shader.
	normalModeIndex = glGetSubroutineIndex(_program, GL_VERTEX_SHADER, "normalMode");
	colorKeyIndex = glGetSubroutineIndex(_program, GL_VERTEX_SHADER, "colorKeyMode");

	subroutines[0] = normalModeIndex; //default is diffuse

	mTransformation_loc = glGetUniformLocation(_program, "mTransformation");
	modelView_loc = glGetUniformLocation(_program, "ModelView");
	mProjection_loc = glGetUniformLocation(_program, "Projection");

	glUniformMatrix4fv(mTransformation_loc, 1, GL_FALSE, glm::value_ptr(mTransformation));
	this->program = _program;
    glBindVertexArray(0);
}


void Mesh::setDiffuse(glm::vec4 lightDiffuse, glm::vec4 materialDiffuse){
	this->lightDiffuse = lightDiffuse;
	this->materialDiffuse = materialDiffuse;
	//Pass to shader.
	glUseProgram(program);
	glUniform4fv(lightDiffuse_loc, 1, glm::value_ptr(lightDiffuse));
	glUniform4fv(materialDiffuse_loc, 1, glm::value_ptr(materialDiffuse));
}

void Mesh::setSpecular(glm::vec4 materialDiffuse, float shininess){
	this->materialSpecular = materialSpecular;
	this->shininess = shininess;
	glUseProgram(program);
	glUniform4fv(materialSpecular_loc, 1, glm::value_ptr(materialSpecular));
	glUniform1f(shininess_loc, shininess);
}
/*
  http://www.opengl.org/discussion_boards/showthread.php/163929-image-loading?p=1158293#post1158293
*/
void Mesh::loadTexture(const char *filename){
	isTextured = true;
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename, 0);
	FIBITMAP* image = FreeImage_Load(format, filename);
 
	FIBITMAP* temp = image;
	image = FreeImage_ConvertTo32Bits(image);
	FreeImage_Unload(temp);
 
	int w = FreeImage_GetWidth(image);
	int h = FreeImage_GetHeight(image);
 
	GLubyte* texture = new GLubyte[4*w*h];
	char* pixels = (char*)FreeImage_GetBits(image);
	//FreeImage loads in BGR format, so you need to swap some bytes(Or use GL_BGR).
 
	for(int j= 0; j< w * h; j++){
		texture[j * 4 + 0] = pixels[j * 4 + 2];
		texture[j * 4 + 1] = pixels[j * 4 + 1];
		texture[j * 4 + 2] = pixels[j * 4 + 0];
		texture[j * 4 + 3] = pixels[j * 4 + 3];
	}

	//Bind raw image to texture.
	glGenTextures(1, &textureLocation);
	glBindTexture(GL_TEXTURE_2D, textureLocation);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)texture );
 
	GLenum huboError = glGetError();
	if(huboError){
		std::cout<<"There was an error loading the texture"<< std::endl;
	}
}


/*
	Handles animations.
*/
void Mesh::draw(){
	if(wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(vao[vaoIndex]);
	glUseProgram(program);

	if(isTextured)
		glBindTexture(GL_TEXTURE_2D, textureLocation);

	glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, subroutines);

	//Bind to this array, using these buffers.
	if(drawBox)
		boundingBox->draw(vPosition, mTransformation_loc, mTransformation);
	//http://www.idevgames.com/forums/thread-551.html This keeps camera fixed in world coordinates.
	glUniform4fv(lightPosition_loc, 1, glm::value_ptr( camera->getInverse()* lightPosition ));
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glUniformMatrix4fv(modelView_loc, 1, GL_FALSE, glm::value_ptr(camera->getModelView()));
	glUniformMatrix4fv(mProjection_loc, 1, GL_FALSE, glm::value_ptr(camera->getProjection()));
	glUniformMatrix4fv(mTransformation_loc, 1, GL_FALSE, glm::value_ptr(mTransformation));
	
	//Rebind location of these things
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeofVertices));
	glEnableVertexAttribArray(vNormal);

	if(isTextured){
		glVertexAttribPointer(vertexUV, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeofVertices + sizeofNormals));
		glEnableVertexAttribArray(vertexUV);
	}

	glDrawArrays(GL_TRIANGLES, 0 , rawVerts.size());

    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vNormal);
    glBindVertexArray(0);
}


//Refills subbuffer data. Ideally we'd store it beforehand and just change index.
void Mesh::changeShading(ShadingType shading){
	glBindVertexArray(vao[vaoIndex]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	switch(shading){
		case FLAT:
			glBufferSubData(GL_ARRAY_BUFFER, sizeofVertices, sizeofNormals, surfaceNormals.data());	
			break;
		case SMOOTH:
			glBufferSubData(GL_ARRAY_BUFFER, sizeofVertices, sizeofNormals, vertexNormals.data());
			break;
	}
	currentShading = shading;
	glBindVertexArray(0);
}


//Method 1: Undo previous translation? This will require absolute coordinates.
//Method 2: Just multiply with new translation matrix. Values will accumulate.
void Mesh::translate(glm::vec3& offset){
	//apply translation directly to current transfomation matrix
	glm::mat4 translation(1.0f);
	translation = glm::translate(translation, offset);
	mTranslation = translation * mTranslation;
	mTransformation = translation * mTransformation;
	currentCenter = glm::vec3(translation * glm::vec4(currentCenter, 1.0f));
}

void Mesh::absoluteTranslate(glm::vec3& offset){
	glm::mat4 translation(1.0f);
	//offset identity matrix
	translation = glm::translate(translation, offset);
	//the accumulated translation
	mTranslation = translation * mTranslation;
	//dont accumulate translation, just use latest
	mTransformation = translation * mTransformation;
	currentCenter = glm::vec3(mTranslation * glm::vec4(currentCenter, 1.0f));
}

//Move to an absolute coordinate on the map.
void Mesh::moveTo(glm::vec3& point){
	//translate to center, then translate to point.
	mTranslation = glm::translate(glm::mat4(1.0f), point);
	mTransformation = mTranslation * mRotation * mScale *  glm::translate(glm::mat4(1.0f), -center);
	currentCenter = point;
}

void Mesh::rotate(glm::vec3& rotations, bool positive){
	changedUniform = true;
	//May have to translate to center.
	glm::mat4 allAxes =  glm::mat4(1.0f);
	float one = 1.0f;
	if(!positive)
		one = -1.0f;
	if(rotations.x != 0.0f) 
		allAxes = glm::rotate(allAxes, rotations.x, glm::vec3(one, 0.0f, 0.0f));
	if(rotations.y != 0.0f)
		allAxes = glm::rotate(allAxes, rotations.y, glm::vec3(0.0f, one, 0.0f));
	if(rotations.z != 0.0f)
		allAxes = glm::rotate(allAxes, rotations.z, glm::vec3(0.0f, 0.0f, one));
		mRotation = allAxes * mRotation;
		mTransformation = mRotation * mTransformation;
}

void Mesh::translateOrigin(){
	mTransformation = mRotation * mScale;
}

 void Mesh::translateBack(){
	mTransformation = mTranslation * mRotation * mScale;
}


//Scale, translate to center, rotate new, rotate old, translate back to object origin, translate to original location
void Mesh::rotateSelf(glm::vec3& rotations, bool positive){
	glm::mat4 tempRotations = mRotation;
	mRotation = glm::mat4(1.0f); //clear rotations
	rotate(rotations, positive); //apply new rotations
	mTransformation =  mTranslation * glm::translate(glm::mat4(1.0f), center) * tempRotations
	 * mRotation *  mScale * glm::translate(glm::mat4(1.0f), -center);
	mRotation = tempRotations * mRotation; //restore rotation
}

//only allow uniform scaling for now
void Mesh::scale(float scaling){
	glm::mat4 scale(1.0f);
	scale = glm::scale(scale, glm::vec3(scaling, scaling, scaling));
	mTransformation = scale * mTransformation;
	mScale = scale * mScale;
	size = mScale * size;
}


void Mesh::scaleCenterUniform(float scaling){
	glm::mat4 scale(1.0f);
	scale = glm::scale(scale, glm::vec3(scaling, scaling, scaling));
	mTransformation = mTranslation * glm::translate(glm::mat4(1.0f), center) * mRotation * scale * mScale * glm::translate(glm::mat4(1.0f), -center);
	mScale = scale * mScale;
}

void Mesh::anchorBottom(){
	center = glm::vec3(minCoords.x, minCoords.y, center.z);
}
void Mesh::scaleCenter(glm::vec3& scaleFactor){
	glm::mat4 scale(1.0f);
	scale = glm::scale(scale, scaleFactor);
	mTransformation = mTranslation * glm::translate(glm::mat4(1.0f), center) * mRotation * scale * mScale * glm::translate(glm::mat4(1.0f), -center);
	mScale = scale * mScale;
	size = mScale * size;
}

//Change indices of subroutines.
void Mesh::setLighting(LightingType lighting){
	switch(lighting){
		case NORMAL_MODE:
			subroutines[0] = normalModeIndex;
			break;
		case COLOR_ID:
			subroutines[0] = colorKeyIndex;
			break;
	}
}

Mesh::~Mesh(){
	//TODO, delete pointers
	glDeleteVertexArrays(1, vao);
	glDeleteBuffers(1, &vertexBuffer);
}
