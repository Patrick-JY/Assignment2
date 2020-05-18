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
	GLfloat color[3];
} Vertex;

//mesh properties
typedef struct Mesh
{
	Vertex* pMeshVertices; //points to mesh vertices
	GLint numberOfVertices; //number of vertices in the mesh
} Mesh;

//Global Vars
const int vbo_vao_number = 7; //needed to make sure I clean up everything properly

Mesh nucleusMesh;
Mesh electronMesh[3];
Mesh orbitPathMesh[3]; 
GLuint g_VBO[vbo_vao_number];
GLuint g_VAO[vbo_vao_number];
GLuint g_shaderProgramID = 0;
GLuint g_MVP_Index = 0;
glm::mat4 nucleusMatrix;
glm::mat4 electronMatrixArray[3]; //making these an array because it seems more convenient
glm::mat4 orbitPathsMatrixArray[3]; //same ^
glm::mat4 g_viewMatrix;
glm::mat4 g_projectionMatrix;

GLuint g_windowWidth = 800; //window dimensions
GLuint g_windowHeight = 600;

float electronScale = 0.2f;

bool wireFrame = false;

bool draw_orbit(Mesh* mesh,float centre_x , float centre_y, float centre_z) {
	
	
	mesh->pMeshVertices = new Vertex[MAX_VERTICES];
	mesh->numberOfVertices = MAX_VERTICES;
	float scale_factor = static_cast<float>(g_windowHeight) / g_windowWidth;
	
	
	
	for (int i = 0; i < MAX_VERTICES;i++) {
		float angle = ((float)M_PI * 2.0f * (float)i) / float(CIRCLE_SLICES);
		

		//drawing the circle
		mesh->pMeshVertices[i].position[0] = 2.0f * cos(angle);
		mesh->pMeshVertices[i].position[2] = 2.0f * sin(angle);
		mesh->pMeshVertices[i].position[1] = 0.0f;
		
		//setting the color of the vertices
		mesh->pMeshVertices[i].color[0] = 1.0f;
		mesh->pMeshVertices[i].color[1] = 0.0f;
		mesh->pMeshVertices[i].color[2] = 0.0f;
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
	const aiScene* pScene = aiImportFile(fileName, aiProcess_Triangulate);

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

			// (CHANGE THIS) since we have no lighting, give each vertex a random colour (CHANGE THIS)
			mesh->pMeshVertices[i].color[0] = static_cast<double>(rand()) / RAND_MAX;
			mesh->pMeshVertices[i].color[1] = static_cast<double>(rand()) / RAND_MAX;
			mesh->pMeshVertices[i].color[2] = static_cast<double>(rand()) / RAND_MAX;
		}
	}

	//release the scene
	aiReleaseImport(pScene);

	return true;


}


static void init(GLFWwindow* window) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);

	//create and compile our GLSL program from the shader files
	g_shaderProgramID = loadShaders("VS.vert", "ColorFS.frag");

	//find the location of shader vars
	GLuint positionIndex = glGetAttribLocation(g_shaderProgramID, "aPosition");
	GLuint colorIndex = glGetAttribLocation(g_shaderProgramID, "aColor");
	g_MVP_Index = glGetUniformLocation(g_shaderProgramID, "uModelViewProjectionMatrix");


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
	
	
	


	//Creating the nucleus
	
	//load nucleus Mesh
	nucleusMesh.pMeshVertices = NULL;
	load_mesh("models/sphere.obj", &nucleusMesh);


	glBindVertexArray(g_VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * nucleusMesh.numberOfVertices, nucleusMesh.pMeshVertices, GL_STATIC_DRAW);


	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));

	glEnableVertexAttribArray(positionIndex); // enable vertex attributes
	glEnableVertexAttribArray(colorIndex);




	//creating electrons
	//load electron Meshes

	for (int i = 0; i < 3; i++) {
		electronMesh[i].pMeshVertices = NULL;
		load_mesh("models/sphere.obj", &electronMesh[i]);

		glBindVertexArray(g_VAO[i+1]);
		glBindBuffer(GL_ARRAY_BUFFER, g_VBO[i+1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * electronMesh[i].numberOfVertices, electronMesh[i].pMeshVertices, GL_STATIC_DRAW);


		glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
		glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));

		glEnableVertexAttribArray(positionIndex); // enable vertex attributes
		glEnableVertexAttribArray(colorIndex);
	}


	//orbit Paths
	orbitPathMesh[0].pMeshVertices = NULL;
	draw_orbit(&orbitPathMesh[0], 0.0f, 0.0f, 0.0f);
	glBindVertexArray(g_VAO[4]);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * orbitPathMesh[0].numberOfVertices, orbitPathMesh[0].pMeshVertices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));

	glEnableVertexAttribArray(positionIndex); // enable vertex attributes
	glEnableVertexAttribArray(colorIndex);


}


// function used to update the scene

static void update_scene() {
	
	//currently moves the electrons to where they start in the scene
	electronMatrixArray[0] = glm::translate(vec3(2.0f, 1.0f, 0.0f))* glm::scale(glm::vec3(electronScale, electronScale, electronScale));;
	electronMatrixArray[1] = glm::translate(vec3(-2.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(electronScale, electronScale, electronScale));;
	electronMatrixArray[2] = glm::translate(vec3(0.0f, -2.0f, 0.0f)) * glm::scale(glm::vec3(electronScale, electronScale, electronScale));;
	orbitPathsMatrixArray[0] = glm::rotate(0.01f, vec3(1.0f, 0.0f, 0.0f));

}



static void render_scene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// clear colour buffer and depth buffer

	glUseProgram(g_shaderProgramID);	// use the shaders associated with the shader program

	glBindVertexArray(g_VAO[0]);		// make VAO active

	glm::mat4 MVP = g_projectionMatrix * g_viewMatrix * nucleusMatrix;
	// set uniform model transformation matrix
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);

	glDrawArrays(GL_TRIANGLES, 0, nucleusMesh.numberOfVertices);

	for (int i = 0; i < 3; i++) {
		glBindVertexArray(g_VAO[i+1]);

		MVP = g_projectionMatrix * g_viewMatrix * electronMatrixArray[i];
		// set uniform model transformation matrix
		glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);

		glDrawArrays(GL_TRIANGLES, 0, electronMesh[i].numberOfVertices);
	}

	glBindVertexArray(g_VAO[4]);
	MVP = g_projectionMatrix * g_viewMatrix * orbitPathsMatrixArray[0];
	glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_LINE_STRIP, 0, orbitPathMesh[0].numberOfVertices);


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

//main method
int main(void)
{
	GLFWwindow* window = NULL;	// pointer to a GLFW window handle
	
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
	glDeleteBuffers(vbo_vao_number, g_VBO);
	glDeleteVertexArrays(vbo_vao_number, g_VAO);

	// uninitialise tweak bar
	

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}
