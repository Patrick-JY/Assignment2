//was having some trouble with includes (string was apparently being defined twice) 
//so this is taken from lab 5
#define _USE_MATH_DEFINES
#include <cstdio>		// for C++ i/o
#include <iostream>
#include <string>
#include <cstddef>
#include <math.h>

using namespace std;	// to avoid having to use std::

#include <GLEW/glew.h>	// include GLEW
#include <GLFW/glfw3.h>	// include GLFW (which includes the OpenGL header)
#include <glm/glm.hpp>	// include GLM (ideally should only use the GLM headers that are actually used)
#include <glm/gtx/transform.hpp>
using namespace glm;	// to avoid having to use glm::

#include <AntTweakBar.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "shader.h"
//definitions
#define CIRCLE_SLICES 64
#define MAX_VERTICES (CIRCLE_SLICES) * 3




//structs
//vertex stuct
typedef struct Vertex
{
	GLfloat position[3];
	GLfloat normal[3];
} Vertex;

//mesh properties
typedef struct Mesh
{
	Vertex* pMeshVertices; //points to mesh vertices
	GLint numberOfVertices; //number of vertices in the mesh
	GLint* pMeshIndices;
	GLint numberOfFaces;
} Mesh;

// light and material structs
typedef struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

typedef struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};


//Global Vars
const int vbo_vao_number = 7; //needed to make sure I clean up everything properly

Mesh nucleusMesh;
Mesh electronMesh[3];
Mesh orbitPathMesh[3];

GLuint g_IBO[vbo_vao_number];			// index buffer object identifier
GLuint g_VBO[vbo_vao_number];
GLuint g_VAO[vbo_vao_number];
GLuint g_shaderProgramID = 0;
GLuint orbit_shaderProgram = 0;
GLuint g_MVP_Index = 0;
GLuint orbit_MVP_Index = 0;
GLuint g_MV_Index = 0;
GLuint g_V_Index = 0;
GLuint g_lightPositionIndex = 0;
GLuint g_lightAmbientIndex = 0;
GLuint g_lightDiffuseIndex = 0;
GLuint g_lightSpecularIndex = 0;
GLuint g_materialAmbientIndex = 0;
GLuint g_materialDiffuseIndex = 0;
GLuint g_materialSpecularIndex = 0;
GLuint g_materialShininessIndex = 0;

GLuint electron_materialAmbient = 0;
GLuint electron_materialDiffuseIndex = 0;
GLuint electron_materialSpecularIndex = 0;
GLuint electron_materialShininessIndex = 0;

glm::mat4 nucleusMatrix;
glm::mat4 electronMatrixArray[3]; //making these an array because it seems more convenient
glm::mat4 orbitPathsMatrixArray[3]; //same ^
glm::mat4 g_viewMatrix;
glm::mat4 g_projectionMatrix;

Light g_light;	//light properties

// material properties
Material g_material; //material for nucleus
//Material orbit_material; //material for orbits currently unused since I opted to just use a different shader program
Material electron_material;

GLuint g_windowWidth = 800; //window dimensions
GLuint g_windowHeight = 600;

float electronScale = 0.2f;			//I like to render everything the same and then change after to fit the scene
float g_orbitSpeed = 0.3f;


bool wireFrame = false;

bool draw_orbit(Mesh* mesh) {
	
	
	mesh->pMeshVertices = new Vertex[MAX_VERTICES];
	mesh->numberOfVertices = MAX_VERTICES;
	float scale_factor = static_cast<float>(g_windowHeight) / g_windowWidth;
	
	
	
	for (int i = 0; i < MAX_VERTICES;i++) {
		float angle = ((float)M_PI * 2.0f * (float)i) / float(CIRCLE_SLICES);
		

		//drawing the circle
		mesh->pMeshVertices[i].position[0] = 2.0f * cos(angle);
		mesh->pMeshVertices[i].position[2] = 2.0f * sin(angle);
		mesh->pMeshVertices[i].position[1] = 0.0f;
		
		//setting the color of the vertices, the variable is still "normal" however I am using a different shader for the orbit paths which treats it as colour
		mesh->pMeshVertices[i].normal[0] = 0.5f;
		mesh->pMeshVertices[i].normal[1] = 0.0f;
		mesh->pMeshVertices[i].normal[2] = 0.0f;
		cout << "x:";
		cout << mesh->pMeshVertices[i].position[0] << " ";
		cout << "y:";
		cout << mesh->pMeshVertices[i].position[1] << endl;
		
	}
	
	return true;
}


//mesh load function as seen in tuts
bool load_mesh(const char* fileName, Mesh* mesh) {
	// load file with assimp
	const aiScene* pScene = aiImportFile(fileName, aiProcess_Triangulate | aiProcess_GenSmoothNormals
	| aiProcess_JoinIdenticalVertices);

	
	//checks whether the scene was loaded
	if (!pScene) {
		cout << "Could not load mesh." << endl;
		return false;
	}

	//get pointer to mesh 0
	const aiMesh* pMesh = pScene->mMeshes[0];

	//store number of mesh vertices
	mesh->numberOfVertices = pMesh->mNumVertices;

	// if mesh contains vertex coordinates

	if (pMesh->HasPositions()) {
		mesh->pMeshVertices = new Vertex[pMesh->mNumVertices];

		for (int i = 0; i < pMesh->mNumVertices; i++) {
			const aiVector3D* pVertexPos = &(pMesh->mVertices[i]);

			mesh->pMeshVertices[i].position[0] = (GLfloat)pVertexPos->x;
			mesh->pMeshVertices[i].position[1] = (GLfloat)pVertexPos->y;
			mesh->pMeshVertices[i].position[2] = (GLfloat)pVertexPos->z;

		}
	}
	
	// if mesh contains normals
	if (pMesh->HasNormals())
	{
		// read normals and store in the array
		for (int i = 0; i < pMesh->mNumVertices; i++)
		{
			const aiVector3D* pVertexNormal = &(pMesh->mNormals[i]);

			mesh->pMeshVertices[i].normal[0] = (GLfloat)pVertexNormal->x;
			mesh->pMeshVertices[i].normal[1] = (GLfloat)pVertexNormal->y;
			mesh->pMeshVertices[i].normal[2] = (GLfloat)pVertexNormal->z;
		}
	}
	
	// if mesh contains faces
	if (pMesh->HasFaces())
	{
		// store number of mesh faces
		mesh->numberOfFaces = pMesh->mNumFaces;

		// allocate memory for vertices
		mesh->pMeshIndices = new GLint[pMesh->mNumFaces * 3];

		// read normals and store in the array
		for (int i = 0; i < pMesh->mNumFaces; i++)
		{
			const aiFace* pFace = &(pMesh->mFaces[i]);

			mesh->pMeshIndices[i * 3] = (GLint)pFace->mIndices[0];
			mesh->pMeshIndices[i * 3 + 1] = (GLint)pFace->mIndices[1];
			mesh->pMeshIndices[i * 3 + 2] = (GLint)pFace->mIndices[2];
		}
	}

	//release the scene
	aiReleaseImport(pScene);

	return true;


}


static void init(GLFWwindow* window) {
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);

	//create and compile our GLSL programs from the shader files
	g_shaderProgramID = loadShaders("VS.vert", "ColorFS.frag");
	orbit_shaderProgram = loadShaders("simpleColorVS.vert", "simpleColorFS.frag");

	//find the location of shader vars
	GLuint positionIndex = glGetAttribLocation(g_shaderProgramID, "aPosition");
	GLuint normalIndex = glGetAttribLocation(g_shaderProgramID, "aNormal");
	g_MVP_Index = glGetUniformLocation(g_shaderProgramID, "uModelViewProjectionMatrix");
	g_MV_Index = glGetUniformLocation(g_shaderProgramID, "uModelViewMatrix");
	g_V_Index = glGetUniformLocation(g_shaderProgramID, "uViewMatrix");


	g_lightPositionIndex = glGetUniformLocation(g_shaderProgramID, "uLight.position");
	g_lightAmbientIndex = glGetUniformLocation(g_shaderProgramID, "uLight.ambient");
	g_lightDiffuseIndex = glGetUniformLocation(g_shaderProgramID, "uLight.diffuse");
	g_lightSpecularIndex = glGetUniformLocation(g_shaderProgramID, "uLight.specular");


	g_materialAmbientIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.ambient");
	g_materialDiffuseIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.diffuse");
	g_materialSpecularIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.specular");
	g_materialShininessIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.shininess");

	electron_materialAmbient = glGetUniformLocation(g_shaderProgramID, "uMaterial.ambient");
	electron_materialDiffuseIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.diffuse");
	electron_materialSpecularIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.specular");
	electron_materialShininessIndex = glGetUniformLocation(g_shaderProgramID, "uMaterial.shininess");


	GLuint orbitPositionIndex = glGetAttribLocation(orbit_shaderProgram, "aPosition");
	GLuint orbitColorIndex = glGetAttribLocation(orbit_shaderProgram, "aColor");
	orbit_MVP_Index = glGetUniformLocation(orbit_shaderProgram, "uModelViewProjectionMatrix");

	//init model matrices
	nucleusMatrix = mat4(1.0f);
	electronMatrixArray[0] = mat4(1.0f);
	electronMatrixArray[1] = mat4(1.0f);
	electronMatrixArray[2] = mat4(1.0f);
	orbitPathsMatrixArray[0] = mat4(1.0f);
	orbitPathsMatrixArray[1] = mat4(1.0f);
	orbitPathsMatrixArray[2] = mat4(1.0f);

	//init view matrix
	g_viewMatrix = lookAt(vec3(0.0f, 0.0f, 6.0f), vec3(0.0f, 0.0f, 5.0f), vec3(0.0f, 1.0f, 0.0f));

	int width;
	int height;

	glfwGetFramebufferSize(window, &width, &height);
	float aspectRatio = static_cast<float>(width) / height;

	//initialise projection matrix
	g_projectionMatrix = perspective(radians(45.0f), aspectRatio, 0.1f, 100.0f);
	//initialise vbo and vao
	glGenBuffers(vbo_vao_number, g_VBO);
	glGenVertexArrays(vbo_vao_number, g_VAO);
	glGenBuffers(vbo_vao_number, g_IBO);
	
	// initialise point light properties
	g_light.position = glm::vec3(10.0f, 10.0f, 10.0f);
	g_light.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	g_light.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	g_light.specular = glm::vec3(1.0f, 1.0f, 1.0f);

	// initialise material properties
	g_material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
	g_material.diffuse = glm::vec3(0.2f, 0.7f, 1.0f);
	g_material.ambient = glm::vec3(0.2f, 0.7f, 1.0f);
	g_material.shininess = 10.0f;
	
	/*
	orbit_material.ambient = vec3(1.0f, 1.0f, 1.0f);
	orbit_material.diffuse = vec3(1.0f, 0.0f, 0.0f);
	orbit_material.specular = vec3(1.0f, 0.0f, 0.0f);
	orbit_material.shininess = 0.0f;
	*/

	electron_material.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	electron_material.ambient = glm::vec3(1.0f, 0.2f, 0.2f);
	electron_material.specular = glm::vec3(0.6f, 0.1f, 1.0f);
	electron_material.shininess = 10.0f;

	//Creating the nucleus
	
	//load nucleus Mesh
	nucleusMesh.pMeshVertices = NULL;
	load_mesh("models/sphere.obj", &nucleusMesh);


	glBindVertexArray(g_VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * nucleusMesh.numberOfVertices, nucleusMesh.pMeshVertices, GL_STATIC_DRAW);

	// generate identifier for IBO and copy data to GPU
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * 3 * nucleusMesh.numberOfFaces, nucleusMesh.pMeshIndices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO[0]);
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

	glEnableVertexAttribArray(positionIndex); // enable vertex attributes
	glEnableVertexAttribArray(normalIndex);




	//creating electrons
	//load electron Meshes

	for (int i = 0; i < 3; i++) {
		electronMesh[i].pMeshVertices = NULL;
		load_mesh("models/sphere.obj", &electronMesh[i]);

		glBindVertexArray(g_VAO[i+1]);
		glBindBuffer(GL_ARRAY_BUFFER, g_VBO[i+1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * electronMesh[i].numberOfVertices, electronMesh[i].pMeshVertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO[i+1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * 3 * electronMesh[i].numberOfFaces, electronMesh[i].pMeshIndices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, g_VBO[i+1]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO[i+1]);
		glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
		glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

		glEnableVertexAttribArray(positionIndex); // enable vertex attributes
		glEnableVertexAttribArray(normalIndex);
	}


	
	
	//orbit Paths
	for (int i = 0; i < 3; i++) {
		orbitPathMesh[0].pMeshVertices = NULL;
		draw_orbit(&orbitPathMesh[i]);
		glBindVertexArray(g_VAO[i+4]);
		glBindBuffer(GL_ARRAY_BUFFER, g_VBO[i+4]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * orbitPathMesh[i].numberOfVertices, orbitPathMesh[i].pMeshVertices, GL_STATIC_DRAW);

		


		glBindBuffer(GL_ARRAY_BUFFER, g_VBO[i + 4]);
		
		glVertexAttribPointer(orbitPositionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
		glVertexAttribPointer(orbitColorIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

		glEnableVertexAttribArray(orbitPositionIndex); // enable vertex attributes
		glEnableVertexAttribArray(orbitColorIndex);
	}
	//putting the camera at the correct angle
	g_projectionMatrix = rotate(g_projectionMatrix, 0.5f, vec3(1.0f, 0.0f, 0.0f));
	g_projectionMatrix = translate(g_projectionMatrix, vec3(0.0f, -3.0f, 0.0f));
}


// function used to update the scene

static void update_scene() {
	static float orbitAngle = 0.0f;
	float scaleFactor = 0.1;
	//currently moves the electrons to where they start in the scene and also making them orbit

	

	orbitAngle += g_orbitSpeed * scaleFactor;
	electronMatrixArray[1] = glm::rotate(orbitAngle, vec3(0.0f, 1.0f, 0.0f)) * glm::translate(vec3(2.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(electronScale, electronScale, electronScale));
	electronMatrixArray[2] = glm::rotate(orbitAngle,vec3(-1.0f,1.0f,0.0f))* glm::translate(vec3(0.0f, -0.0f, 2.0f))* glm::scale(glm::vec3(electronScale, electronScale, electronScale));
	orbitPathsMatrixArray[0] = glm::rotate(1.0f, vec3(0.0f, 1.0f, 0.0f));
	orbitPathsMatrixArray[1] = glm::rotate(1.0f, vec3(-2.0f, 5.0f, 6.5f));
	orbitPathsMatrixArray[2] = glm::rotate(1.0f, vec3(2.0f,5.0f, -6.5f));


	electronMatrixArray[0] = glm::rotate(orbitAngle,vec3(1.0f,1.0f,0.0f)) *  glm::translate(vec3(-1.4f, 1.4f, 0.0f))
		* glm::scale(glm::vec3(electronScale, electronScale, electronScale));

}

static void render_scene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// clear colour buffer and depth buffer

	glUseProgram(g_shaderProgramID);	// use the shaders associated with the shader program

	glBindVertexArray(g_VAO[0]);		// make VAO active
	
	glm::mat4 MVP = g_projectionMatrix * g_viewMatrix * nucleusMatrix;
	glm::mat4 MV = g_viewMatrix * nucleusMatrix;
	// set uniform model transformation matrix
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
	glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrix[0][0]);

	glUniform3fv(g_lightPositionIndex, 1, &g_light.position[0]);
	glUniform3fv(g_lightAmbientIndex, 1, &g_light.ambient[0]);
	glUniform3fv(g_lightDiffuseIndex, 1, &g_light.diffuse[0]);
	glUniform3fv(g_lightSpecularIndex, 1, &g_light.specular[0]);

	glUniform3fv(g_materialAmbientIndex, 1, &g_material.ambient[0]);
	glUniform3fv(g_materialDiffuseIndex, 1, &g_material.diffuse[0]);
	glUniform3fv(g_materialSpecularIndex, 1, &g_material.specular[0]);
	glUniform1fv(g_materialShininessIndex, 1, &g_material.shininess);
	
	glDrawElements(GL_TRIANGLES, nucleusMesh.numberOfFaces*3,GL_UNSIGNED_INT,0);

	for (int i = 0; i < 3; i++) {
		glBindVertexArray(g_VAO[i+1]);

		MVP = g_projectionMatrix * g_viewMatrix * electronMatrixArray[i];
		MV = g_viewMatrix * electronMatrixArray[i];
		// set uniform model transformation matrix
		glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(g_MV_Index, 1, GL_FALSE, &MV[0][0]);
		glUniformMatrix4fv(g_V_Index, 1, GL_FALSE, &g_viewMatrix[0][0]);

		glUniform3fv(g_lightPositionIndex, 1, &g_light.position[0]);
		glUniform3fv(g_lightAmbientIndex, 1, &g_light.ambient[0]);
		glUniform3fv(g_lightDiffuseIndex, 1, &g_light.diffuse[0]);
		glUniform3fv(g_lightSpecularIndex, 1, &g_light.specular[0]);

		glUniform3fv(electron_materialAmbient, 1, &electron_material.ambient[0]);
		glUniform3fv(electron_materialDiffuseIndex, 1, &electron_material.diffuse[0]);
		glUniform3fv(electron_materialSpecularIndex, 1, &electron_material.specular[0]);
		glUniform1fv(electron_materialShininessIndex, 1, &electron_material.shininess);

		glDrawElements(GL_TRIANGLES, electronMesh[i].numberOfFaces * 3, GL_UNSIGNED_INT, 0);
	}

	

	glUseProgram(orbit_shaderProgram);
	for (int i = 0; i < 3; i++) {
		
		glBindVertexArray(g_VAO[i+4]);
		MVP = g_projectionMatrix * g_viewMatrix * orbitPathsMatrixArray[i];
		
		// set uniform model transformation matrix
		glUniformMatrix4fv(orbit_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
		
		glDrawArrays(GL_LINE_STRIP, 0, orbitPathMesh[i].numberOfVertices);
	
	}
	


	glFlush();	// flush the pipeline
}


//various callback funcs

//key callback
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// quit if the ESCAPE key was press
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
}

// error callback function
static void error_callback(int error, const char* description)
{
	cerr << description << endl;	// output error description
}



static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// pass mouse data to tweak bar
	TwEventMousePosGLFW(xpos, ypos);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	TwEventMouseButtonGLFW(button, action);
}



//main method
int main(void)
{
	GLFWwindow* window = NULL;	// pointer to a GLFW window handle
	TwBar* TweakBar;

	double lastUpdateTime = glfwGetTime();	// last update time
	double elapsedTime = lastUpdateTime;	// time elapsed since last update
	double frameTime = 0.0f;				// frame time
	int frameCount = 0;						// number of frames since last update
	int FPS = 0;							// frames per second

	glfwSetErrorCallback(error_callback);	// set error callback function

	// initialise GLFW
	if (!glfwInit())
	{
		// if failed to initialise GLFW
		exit(EXIT_FAILURE);
	}

	// minimum OpenGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// create a window and its OpenGL context
	window = glfwCreateWindow(g_windowWidth, g_windowHeight, "Tutorial", NULL, NULL);

	// if failed to create window
	if (window == NULL)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);	// set window context as the current context
	glfwSwapInterval(1);			// swap buffer interval

	// initialise GLEW
	if (glewInit() != GLEW_OK)
	{
		// if failed to initialise GLEW
		cerr << "GLEW initialisation failed" << endl;
		exit(EXIT_FAILURE);
	}

	// set key callback function
	glfwSetKeyCallback(window, key_callback);
	


	

	


	// initialise rendering states
	init(window);

	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		update_scene();		// update the scene

		if (wireFrame)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		render_scene();		// render the scene

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		

		glfwSwapBuffers(window);	// swap buffers
		glfwPollEvents();			// poll for events

		frameCount++;
		elapsedTime = glfwGetTime() - lastUpdateTime;	// current time - last update time

		if (elapsedTime >= 1.0f)	// if time since last update >= to 1 second
		{
			frameTime = 1.0f / frameCount;	// calculate frame time

			string str = "FPS = " + to_string(frameCount) + "; FT = " + to_string(frameTime);

			glfwSetWindowTitle(window, str.c_str());	// update window title

			FPS = frameCount;
			frameCount = 0;					// reset frame count
			lastUpdateTime += elapsedTime;	// update last update time
		}
	}

	// clean up
	if (nucleusMesh.pMeshVertices)
		delete[] nucleusMesh.pMeshVertices;

	if (electronMesh[0].pMeshVertices) {
		for (int i = 0; i < 3; i++) {
			delete[] electronMesh[i].pMeshVertices;
		}
	
	}
	

	glDeleteProgram(g_shaderProgramID);
	glDeleteProgram(orbit_shaderProgram);
	glDeleteBuffers(vbo_vao_number, g_IBO);
	glDeleteBuffers(vbo_vao_number, g_VBO);
	glDeleteVertexArrays(vbo_vao_number, g_VAO);

	// uninitialise tweak bar
	

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}
