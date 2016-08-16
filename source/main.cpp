#include <iostream>
#include <cmath>
#include <string>
#include <map>

// GLEW
#include <GL/glew.h>
// GLUT
#include <GL/glut.h>
// GLFW
#include <GLFW/glfw3.h>
// SOIL
#include <SOIL.h>
// GLM
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "ShaderManager.h"
#include "Camera.h"

// Window dimensions
const GLuint screenWidth = 800, screenHeight = 600;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
GLuint loadTexture(GLchar* path, bool alpha);
GLuint loadCubemap(std::vector<const GLchar*> faces);
GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil);

void Do_Movement();
void DrawCube(Shader shader);
void DoLighting(Shader shader, Shader lamp);
void DoToonShading(Shader shader);
void DoOutlineShading(Shader shader, Shader shaderSingleColor);
void DoTransparentShading(Shader shader, Shader transparent);
void DrawSkybox(Shader shader);
void DrawEnvironmentCubemaps(Shader reflection, Shader refraction);

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool enableImageEffects = false;
bool enableAntiAliasing = true;

bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

GLuint groundTexture;
GLuint faceTexture;
GLuint containerTexture;
GLuint containerSpecularTexture;
GLuint grassTexture;
std::vector<const GLchar*> skyboxFaces;
GLuint skyboxTexture;

GLuint VBOs[5], VAOs[5];

glm::vec3 cubePositions[] = {
	glm::vec3(0.0f,  0.0f,  0.0f),
	glm::vec3(2.0f,  5.0f, -15.0f),
	glm::vec3(-1.5f, -2.2f, -2.5f),
	glm::vec3(-3.8f, -2.0f, -12.3f),
	glm::vec3(2.4f, -0.4f, -3.5f),
	glm::vec3(-1.7f,  3.0f, -7.5f),
	glm::vec3(1.3f, -2.0f, -2.5f),
	glm::vec3(1.5f,  2.0f, -2.5f),
	glm::vec3(1.5f,  0.2f, -1.5f),
	glm::vec3(-1.3f,  1.0f, -1.5f)
};
glm::vec3 grassPositions[] = {
	glm::vec3(-1.5f,  0.0f, -0.48f),
	glm::vec3(1.5f,  0.0f,  0.51f),
	glm::vec3(0.0f,  0.0f,  0.7f),
	glm::vec3(-0.3f,  0.0f, -2.3f),
	glm::vec3(0.5f,  0.0f, -0.6f)
};
glm::vec3 pointLightPositions[] = {
	glm::vec3(0.7f,  0.2f,  2.0f),
	glm::vec3(2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3(0.0f,  0.0f, -3.0f)
};
glm::vec3 pointLightColors[] = {
	glm::vec3(1.0f,  0.0f,  0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f,  0.0f, 1.0f),
	glm::vec3(0.0f,  0.8f, 1.0f)
};

glm::mat3 GetNormalMatrix(glm::mat4 matrix) {
	return glm::mat3(glm::transpose(glm::inverse(matrix)));
}
// The MAIN function, from here we start the application and run the game loop
int main()
{
	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	//glfwWindowHint(GLFW_SAMPLES, 4);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Shader Tests", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	// Define the viewport dimensions
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);
	// Pass depth test is fragment z is less or equal to current depth value
	glDepthFunc(GL_LEQUAL);
	// Outlines
	/*glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);*/
	// Transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	// Anti-Aliasing
	glEnable(GL_MULTISAMPLE);

	ShaderManager* shaderManager = new ShaderManager();
	shaderManager->InitializeShaders();
	Shader defaultShader = shaderManager->m_pDefault;
	Shader basicShader = shaderManager->m_pBasic;
	Shader lightShader = shaderManager->m_pLighting;
	Shader lampShader = shaderManager->m_pLamp;
	Shader toonShader = shaderManager->m_pToon;
	Shader hatchShader = shaderManager->m_pHatch;
	Shader outlineshader = shaderManager->m_pOutline;
	Shader transparentShader = shaderManager->m_pTransparent;
	Shader screenShader = shaderManager->m_pScreen;
	Shader skyboxShader = shaderManager->m_pSkybox;
	Shader reflectShader = shaderManager->m_pReflection;
	Shader refractShader = shaderManager->m_pRefraction;
	Shader normalShader = shaderManager->m_pNormal;

#pragma region Object Initialization
	// Set up vertex data (and buffer(s)) and attribute pointers
	GLfloat vertices[] = {
		// v c t
		-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
		0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

		0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f
	};
	// Set up vertex data (and buffer(s)) and attribute pointers
	GLfloat cubeVertices[] = {
		// Positions           // Normals           // Texture Coords
		// Back face
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
		0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, // bottom-right         
		0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f, // top-right
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, // bottom-left
		-0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f, // top-left
		// Front face
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // bottom-left
		0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // bottom-right
		0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // top-right
		0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // top-right
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // top-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // bottom-left
		// Left face
		-0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f, // top-right
		-0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f, // top-left
		-0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, // bottom-right
		-0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f, // top-right
		// Right face
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, // top-left
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, // bottom-right
		0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, // top-right         
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, // bottom-right
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, // top-left
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, // bottom-left     
		// Bottom face
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f, // top-left
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, // bottom-left
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f, // bottom-right
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f, // top-right
		// Top face
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, // top-left
		0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, // bottom-right
		0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, // top-right     
		0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, // bottom-right
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, // top-left
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f  // bottom-left        
	};
	GLfloat cubeNormals[] = {
		// Normals
		// Back face
		0.0f, 0.0f, -1.0f, // Bottom-left
		0.0f, 0.0f, -1.0f, // top-right
		0.0f, 0.0f, -1.0f, // bottom-right         
		0.0f, 0.0f, -1.0f, // top-right
		0.0f, 0.0f, -1.0f, // bottom-left
		0.0f, 0.0f, -1.0f, // top-left
		// Front face
		0.0f, 0.0f, 1.0f, // bottom-left
		0.0f, 0.0f, 1.0f, // bottom-right
		0.0f, 0.0f, 1.0f, // top-right
		0.0f, 0.0f, 1.0f, // top-right
		0.0f, 0.0f, 1.0f, // top-left
		0.0f, 0.0f, 1.0f, // bottom-left
		// Left face
		-1.0f, 0.0f, 0.0f, // top-right
		-1.0f, 0.0f, 0.0f, // top-left
		-1.0f, 0.0f, 0.0f, // bottom-left
		-1.0f, 0.0f, 0.0f, // bottom-left
		-1.0f, 0.0f, 0.0f, // bottom-right
		-1.0f, 0.0f, 0.0f, // top-right
		// Right face
		1.0f, 0.0f, 0.0f, // top-left
		1.0f, 0.0f, 0.0f, // bottom-right
		1.0f, 0.0f, 0.0f, // top-right         
		1.0f, 0.0f, 0.0f, // bottom-right
		1.0f, 0.0f, 0.0f, // top-left
		1.0f, 0.0f, 0.0f, // bottom-left     
		// Bottom face
		0.0f, -1.0f, 0.0f, // top-right
		0.0f, -1.0f, 0.0f, // top-left
		0.0f, -1.0f, 0.0f, // bottom-left
		0.0f, -1.0f, 0.0f, // bottom-left
		0.0f, -1.0f, 0.0f, // bottom-right
		0.0f, -1.0f, 0.0f, // top-right
		// Top face
		0.0f, 1.0f, 0.0f, // top-left
		0.0f, 1.0f, 0.0f, // bottom-right
		0.0f, 1.0f, 0.0f, // top-right     
		0.0f, 1.0f, 0.0f, // bottom-right
		0.0f, 1.0f, 0.0f, // top-left
		0.0f, 1.0f, 0.0f // bottom-left  
	};
	GLfloat planeVertices[] = {
		// Positions            // Texture Coords (note we set these higher than 1 that together with GL_REPEAT (as texture wrapping mode) will cause the floor texture to repeat)
		-5.0f, -0.5f, -5.0f,  0.0f, 10.0f, // top-left
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f, // bottom-left
		5.0f,  -0.5f,  5.0f,  10.0f, 0.0f, // bottom-right

		5.0f,  -0.5f,  5.0f,  10.0f, 0.0f, // bottom-right
		5.0f,  -0.5f, -5.0f,  10.0f, 10.0f, // top-right
		-5.0f, -0.5f, -5.0f,  0.0f, 10.0f // top-left
	};
	GLfloat grassTransparentVertices[] = {
		// Positions         // Texture Coords (swapped y coordinates because texture is flipped upside down)
		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
		1.0f,  0.5f,  0.0f,  1.0f,  0.0f
	};
	// Post-Processing Quad
	GLfloat ndcQuadVerts[] = {
		// Positions  
		-1.0f,  1.0f, 
		-1.0f, -1.0f, 
		1.0f, -1.0f,

		-1.0f,  1.0f,
		1.0f, -1.0f,
		1.0f,  1.0f
	};

	GLfloat ndcQuadTexCoords[] = {
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};
	// Skybox Vertices
	GLfloat skyboxVertices[] = {
		// Positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	glGenVertexArrays(6, VAOs);
	glGenBuffers(6, VBOs);

	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAOs[0]);
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
		
		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		// Color attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		// Texture attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
	glBindVertexArray(0); // Unbind VAO Not EBO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
	
	// CUBE CONTAINER, LAMP, CUBES
	glBindVertexArray(VAOs[1]);
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);	
	glBindVertexArray(0); 
	// PLANE
	glBindVertexArray(VAOs[2]);
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
	glBindVertexArray(0);
	// GRASS PLANE
	glBindVertexArray(VAOs[3]);
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(grassTransparentVertices), &grassTransparentVertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
	glBindVertexArray(0);
	// NDC QUAD 
	glBindVertexArray(VAOs[4]);
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[4]); 
		glBufferData(GL_ARRAY_BUFFER, sizeof(ndcQuadVerts) + sizeof(ndcQuadTexCoords), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ndcQuadVerts), &ndcQuadVerts); 
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(ndcQuadVerts), sizeof(ndcQuadTexCoords), &ndcQuadTexCoords);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)(sizeof(ndcQuadVerts)));
		glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	// SKYBOX CUBEMAP
	glBindVertexArray(VAOs[5]);
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[5]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// Load and create a texture 
	groundTexture = loadTexture("../../media/textures/groundgrass.jpg", false);
	faceTexture = loadTexture("../../media/textures/awesomeface.png", true);
	containerTexture = loadTexture("../../media/textures/container2.png", false);
	containerSpecularTexture = loadTexture("../../media/textures/container2_specular.png", false);
	grassTexture = loadTexture("../../media/textures/grass.png", true);

	// In certain order 
	skyboxFaces.push_back("../../media/textures/skybox/right.jpg");
	skyboxFaces.push_back("../../media/textures/skybox/left.jpg");
	skyboxFaces.push_back("../../media/textures/skybox/top.jpg");
	skyboxFaces.push_back("../../media/textures/skybox/bottom.jpg");
	skyboxFaces.push_back("../../media/textures/skybox/back.jpg");
	skyboxFaces.push_back("../../media/textures/skybox/front.jpg");
	skyboxTexture = loadCubemap(skyboxFaces);
#pragma endregion
	// Anti-Aliasing FrameBuffer
	GLuint FBOMS;
	glGenFramebuffers(1, &FBOMS);
	glBindFramebuffer(GL_FRAMEBUFFER, FBOMS);

	GLuint multisampleTextureColorBuffer;
	glGenTextures(1, &multisampleTextureColorBuffer);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleTextureColorBuffer);	
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, screenWidth, screenHeight, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, multisampleTextureColorBuffer, 0);
	// Anti-Aliasing RenderBuffer
	GLuint RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER::NOT COMPLETE!\n";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Down Sampled FrameBuffer
	GLuint FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	GLuint textureColorBuffer = generateAttachmentTexture(false, false);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER::NOT COMPLETE!\n";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		Do_Movement();

		if (enableImageEffects == true) {
			if (enableAntiAliasing == false) {
				/////////////////////////////////////////////////////
				// Bind to custom framebuffer and draw the 
				// scene.
				// //////////////////////////////////////////////////
				glBindFramebuffer(GL_FRAMEBUFFER, FBO);
				glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);
				// Render to Texture
				DrawSkybox(skyboxShader);
				DoToonShading(toonShader);

				/////////////////////////////////////////////////////
				// Bind to default framebuffer again and draw the 
				// quad plane with attched screen texture.
				// //////////////////////////////////////////////////
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				// Clear all relevant buffers
				glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT); // Clear color buffer to redraw 2nd pass
				// Draw normal scene
				glDisable(GL_DEPTH_TEST); // Don't care about depth information when rendering a single quad

				// Draw Screen
				screenShader.Use();
				glBindVertexArray(VAOs[4]);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, textureColorBuffer);	// Use the color attachment texture as the texture of the quad plane
					glUniform1i(glGetUniformLocation(screenShader.Program, "customTexture"), 0);
					glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindVertexArray(0);
			}
			else
			{
				glBindFramebuffer(GL_FRAMEBUFFER, FBOMS);
				glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);
				// Render
				DoToonShading(toonShader);

				glBindFramebuffer(GL_READ_FRAMEBUFFER, FBOMS);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
				glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glClear(GL_COLOR_BUFFER_BIT);
				glDisable(GL_DEPTH_TEST);
				// Render Quad
				screenShader.Use();
				glBindVertexArray(VAOs[4]);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
					glUniform1i(glGetUniformLocation(screenShader.Program, "customTexture"), 0);
					glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindVertexArray(0);
			}
		}
		else {

			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			DrawSkybox(skyboxShader);
			DrawEnvironmentCubemaps(reflectShader, refractShader);
			//DoLighting(lightShader, lampShader);
			//DoToonShading(toonShader);
			//DrawCube(basicShader);
			//DoLighting(lightShader, lampShader);
			//DoLighting(normalShader, normalShader);
			//DoToonShading(normalShader);
			//DoOutlineShading(defaultShader, outlineshader);
		}

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(6, VAOs);
	glDeleteBuffers(6, VBOs);
	glDeleteFramebuffers(1, &FBO);
	glDeleteFramebuffers(1, &FBOMS);
	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}
void DrawCube(Shader shader) {
	shader.Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, groundTexture);
	glUniform1i(glGetUniformLocation(shader.Program, "ourTexture"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, faceTexture);
	glUniform1i(glGetUniformLocation(shader.Program, "ourTexture1"), 1);

	// Create camera transformation
	glm::mat4 view;
	view = camera.GetViewMatrix();
	glm::mat4 projection;
	projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
	// Get the uniform locations
	GLint modelLoc = glGetUniformLocation(shader.Program, "model");
	GLint viewLoc = glGetUniformLocation(shader.Program, "view");
	GLint projLoc = glGetUniformLocation(shader.Program, "projection");
	// Pass the matrices to the shader
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(VAOs[0]);
	for (GLuint i = 0; i < 10; i++)
	{
		// Calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 model;
		model = glm::translate(model, cubePositions[i]);
		GLfloat angle = 20.0f * i;
		model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindVertexArray(0);
	glUseProgram(0);
}
void DoLighting(Shader shader, Shader lamp) {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat3 normal;

	shader.Use();

	bool useTextures = true;
	GLuint useTexLoc = glGetUniformLocation(shader.Program, "useTexture");
	glUniform1i(useTexLoc, useTextures);

	// Material
	GLint matAmbientLoc = glGetUniformLocation(shader.Program, "material.ambient");
	GLint matDiffuseLoc = glGetUniformLocation(shader.Program, "material.diffuse");
	GLint matDiffuseTexLoc = glGetUniformLocation(shader.Program, "material.diffuseTexture");
	GLint matSpecularLoc = glGetUniformLocation(shader.Program, "material.specular");
	GLint matShineLoc = glGetUniformLocation(shader.Program, "material.shininess");

	glUniform3f(matAmbientLoc, 1.0f, 0.5f, 0.31f);
	glUniform3f(matDiffuseLoc, 0.0f, 0.5f, 0.0f);

	glUniform1i(glGetUniformLocation(shader.Program, "material.diffuseTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, containerTexture);

	glUniform1i(glGetUniformLocation(shader.Program, "material.specularTexture"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, containerSpecularTexture);

	glUniform3f(matSpecularLoc, 0.5f, 0.5f, 0.5f);
	glUniform1f(matShineLoc, 32.0f);

	// Directional Light
	glm::vec3 dirLightDir(-0.2f, -1.0f, -0.3f);
	GLint dirLightDirLoc = glGetUniformLocation(shader.Program, "dirLight.direction");
	GLint dirLightAmbientLoc = glGetUniformLocation(shader.Program, "dirLight.ambient");
	GLint dirLightDiffuseLoc = glGetUniformLocation(shader.Program, "dirLight.diffuse");
	GLint dirLightSpecularLoc = glGetUniformLocation(shader.Program, "dirLight.specular");
	
	glUniform3f(dirLightDirLoc, dirLightDir.x, dirLightDir.y, dirLightDir.z);
	glUniform3f(dirLightAmbientLoc, 0.01f, 0.01f, 0.01f);
	glUniform3f(dirLightDiffuseLoc, 0.0f, 0.1f, 0.9f);
	glUniform3f(dirLightSpecularLoc, 0.5f, 0.5f, 0.5f);

	// Point Lights
	for (int i = 0; i < 4; i++) {
		std::string positionLoc = "pointLights[" + std::to_string(i) + "].position";
		std::string ambientLoc = "pointLights[" + std::to_string(i) + "].ambient";
		std::string diffuseLoc = "pointLights[" + std::to_string(i) + "].diffuse";
		std::string specularLoc = "pointLights[" + std::to_string(i) + "].specular";
		std::string constantLoc = "pointLights[" + std::to_string(i) + "].constant";
		std::string linearLoc = "pointLights[" + std::to_string(i) + "].linear";
		std::string quadraticLoc = "pointLights[" + std::to_string(i) + "].quadratic";

		GLint pointLightPosLoc = glGetUniformLocation(shader.Program, positionLoc.c_str());
		GLint pointLightAmbientLoc = glGetUniformLocation(shader.Program, ambientLoc.c_str());
		GLint pointLightDiffuseLoc = glGetUniformLocation(shader.Program, diffuseLoc.c_str());
		GLint pointLightSpecularLoc = glGetUniformLocation(shader.Program, specularLoc.c_str());
		GLint pointLightConstantLoc = glGetUniformLocation(shader.Program, constantLoc.c_str());
		GLint pointLightLinearLoc = glGetUniformLocation(shader.Program, linearLoc.c_str());
		GLint pointLightQuadraticLoc = glGetUniformLocation(shader.Program, quadraticLoc.c_str());

		glUniform3f(pointLightPosLoc, pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);
		glUniform3f(pointLightAmbientLoc, pointLightColors[i].x* 0.1f, pointLightColors[i].y* 0.1f, pointLightColors[i].z * 0.1f);
		glUniform3f(pointLightDiffuseLoc, pointLightColors[i].x, pointLightColors[i].y, pointLightColors[i].z);
		glUniform3f(pointLightSpecularLoc, 1.0f, 1.0f, 1.0f);
		glUniform1f(pointLightConstantLoc, 1.0f);
		glUniform1f(pointLightLinearLoc, 0.09f);
		glUniform1f(pointLightQuadraticLoc, 0.032f);

	}
	
	// SpotLight
	GLint spotLightPosLoc = glGetUniformLocation(shader.Program, "spotLight.position");
	GLint spotLightDirLoc = glGetUniformLocation(shader.Program, "spotLight.direction");
	GLint spotLightAmbientLoc = glGetUniformLocation(shader.Program, "spotLight.ambient");
	GLint spotLightDiffuseLoc = glGetUniformLocation(shader.Program, "spotLight.diffuse");
	GLint spotLightSpecularLoc = glGetUniformLocation(shader.Program, "spotLight.specular");
	GLint spotLightConstantLoc = glGetUniformLocation(shader.Program, "spotLight.constant");
	GLint spotLightLinearLoc = glGetUniformLocation(shader.Program, "spotLight.linear");
	GLint spotLightQuadraticLoc = glGetUniformLocation(shader.Program, "spotLight.quadratic");
	GLint spotLightCutOffLoc = glGetUniformLocation(shader.Program, "spotLight.cutOff");
	GLint spotLightOutterCutOffLoc = glGetUniformLocation(shader.Program, "spotLight.outterCutOff");

	glUniform3f(spotLightPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
	glUniform3f(spotLightDirLoc, camera.Front.x, camera.Front.y, camera.Front.z);
	glUniform3f(spotLightAmbientLoc, 0.0f, 0.0f, 0.0f);
	glUniform3f(spotLightDiffuseLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(spotLightSpecularLoc, 1.0f, 1.0f, 1.0f);
	glUniform1f(spotLightConstantLoc, 1.0f);
	glUniform1f(spotLightLinearLoc, 0.09f);
	glUniform1f(spotLightQuadraticLoc, 0.032f);
	glUniform1f(spotLightCutOffLoc, glm::cos(glm::radians(12.5f)));
	glUniform1f(spotLightOutterCutOffLoc, glm::cos(glm::radians(15.0f)));

	// Create camera transformations
	view = camera.GetViewMatrix();
	projection = glm::perspective(camera.Zoom, (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);

	// Get the uniform locations
	GLint modelLoc = glGetUniformLocation(shader.Program, "model");
	GLint viewLoc = glGetUniformLocation(shader.Program, "view");
	GLint projLoc = glGetUniformLocation(shader.Program, "projection");
	GLint normalLoc = glGetUniformLocation(shader.Program, "normal");
	// Pass the matrices to the shader
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Draw the container (using container's vertex attributes)
	glBindVertexArray(VAOs[1]);
		for (int i = 0; i < 10; i++) {
			model = glm::mat4();
			model = glm::translate(model, cubePositions[i]);
			GLfloat angle = 20.0f * i;
			model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
			normal = GetNormalMatrix(view * model);
			glUniformMatrix3fv(normalLoc, 1, GL_FALSE, glm::value_ptr(normal));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

	lamp.Use(); 
	// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
	modelLoc = glGetUniformLocation(lamp.Program, "model");
	viewLoc = glGetUniformLocation(lamp.Program, "view");
	projLoc = glGetUniformLocation(lamp.Program, "projection");
	// Set matrices
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	for (int i = 0; i < 4; i++) {
		model = glm::mat4();
		model = glm::translate(model, pointLightPositions[i]);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		normal = GetNormalMatrix(view * model);
		glUniformMatrix3fv(normalLoc, 1, GL_FALSE, glm::value_ptr(normal));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		GLint customColorLoc = glGetUniformLocation(lamp.Program, "customColor");
		glUniform3f(customColorLoc, pointLightColors[i].x, pointLightColors[i].y, pointLightColors[i].z);
		// Draw the light object (using light's vertex attributes)
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindVertexArray(0);
	glUseProgram(0);
}
void DoToonShading(Shader shader) {
	shader.Use();
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);

	
	GLuint modelLoc = glGetUniformLocation(shader.Program, "model");
	GLuint viewLoc = glGetUniformLocation(shader.Program, "view");
	GLuint projectionLoc = glGetUniformLocation(shader.Program, "projection");
	GLuint normalLoc = glGetUniformLocation(shader.Program, "normal");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	GLuint lightDirLoc = glGetUniformLocation(shader.Program, "lightDir");
	glUniform3f(lightDirLoc, -0.2f, -1.0f, -0.3f);

	GLuint customColorLoc = glGetUniformLocation(shader.Program, "customColor");
	glUniform3f(customColorLoc, 0.2f, 0.0f, 1.0f);

	glBindVertexArray(VAOs[1]);
		for (GLuint i = 0; i < 10; i++){
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			glm::mat3 normal;
			model = glm::translate(model, cubePositions[i]);
			GLfloat angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			normal = GetNormalMatrix(view * model);
			glUniformMatrix3fv(normalLoc, 1, GL_FALSE, glm::value_ptr(normal));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	glBindVertexArray(0);
	glUseProgram(0);
}
void DoOutlineShading(Shader shader, Shader shaderSingleColor) {
	// Set uniforms
	shaderSingleColor.Use();
	glm::mat4 model;
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(shaderSingleColor.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderSingleColor.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	shader.Use();
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// Draw floor as normal, we only care about the containers. The floor should NOT fill the stencil buffer so we set its mask to 0x00
	glStencilMask(0x00);
	// Floor
	glBindVertexArray(VAOs[2]);
		glBindTexture(GL_TEXTURE_2D, groundTexture);
		model = glm::mat4();
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	// ===============
	// 1st. Render pass, draw objects as normal, filling the stencil buffer
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	// Cubes
	glBindVertexArray(VAOs[1]);
		glBindTexture(GL_TEXTURE_2D, containerTexture);
		model = glm::translate(model, glm::vec3(-1.0f, 0.001f, -1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(2.0f, 0.001f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	// ===============
	// 2nd. Render pass, now draw slightly scaled versions of the objects, this time disabling stencil writing.
	// Because stencil buffer is now filled with several 1s. The parts of the buffer that are 1 are now not drawn, thus only drawing 
	// the objects' size differences, making it look like borders.
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glDisable(GL_DEPTH_TEST);
	shaderSingleColor.Use();
	GLfloat scale = 1.1;
	// Cubes
	glBindVertexArray(VAOs[1]);
		glBindTexture(GL_TEXTURE_2D, containerTexture);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(-1.0f, 0.001f, -1.0f));
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		glUniformMatrix4fv(glGetUniformLocation(shaderSingleColor.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(2.0f, 0.001f, 0.0f));
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		glUniformMatrix4fv(glGetUniformLocation(shaderSingleColor.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glStencilMask(0xFF);
	glEnable(GL_DEPTH_TEST);
}
void DoTransparentShading(Shader shader, Shader transparent) {
	// Set uniforms
	glm::mat4 model;
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
	transparent.Use();
	glUniformMatrix4fv(glGetUniformLocation(transparent.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(transparent.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	shader.Use();
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// Floor
	glBindVertexArray(VAOs[2]);
		glBindTexture(GL_TEXTURE_2D, groundTexture);
		model = glm::mat4();
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	// Cubes
	glBindVertexArray(VAOs[1]);
		glBindTexture(GL_TEXTURE_2D, containerTexture);
		model = glm::translate(model, glm::vec3(-1.0f, 0.001f, -1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(2.0f, 0.001f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	// Grass
	glDisable(GL_CULL_FACE);
	transparent.Use();
	// Sort grass back to front
	std::map<GLfloat, glm::vec3> sorted;
	for (GLuint i = 0; i < 5; i++)
	{
		GLfloat distance = glm::length(camera.Position - grassPositions[i]);
		sorted[distance] = grassPositions[i];
	}
	glBindVertexArray(VAOs[3]);
		glBindTexture(GL_TEXTURE_2D, grassTexture);
		for (std::map<GLfloat, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it){
			model = glm::mat4();
			model = glm::translate(model, it->second);
			glUniformMatrix4fv(glGetUniformLocation(transparent.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	glBindVertexArray(0);
	glEnable(GL_CULL_FACE);

}
void DrawSkybox(Shader shader) {
	// Ignore depth buffer while drawing skybox
	glDepthMask(GL_FALSE); 

	shader.Use();
	glm::mat4 view;
	glm::mat4 projection;
	
	// Dont transform skybox only rotate
	view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
	projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(VAOs[5]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glUniform1i(glGetUniformLocation(shader.Program, "cubemap"), 0);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthMask(GL_TRUE);
}
void DrawEnvironmentCubemaps(Shader reflection, Shader refraction) {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat3 normal;
	glm::vec3 cameraPos;

	view = camera.GetViewMatrix();
	projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
	cameraPos = glm::vec3(camera.Position.x, camera.Position.y, camera.Position.z);

	glBindVertexArray(VAOs[1]);
		// REFLECTION
		reflection.Use();
		glUniformMatrix4fv(glGetUniformLocation(reflection.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(reflection.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glUniform1i(glGetUniformLocation(reflection.Program, "cubemap"), 0);

		glUniform3f(glGetUniformLocation(reflection.Program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

		for (int i = 0; i < 5; i++) {
			model = glm::mat4();
			model = glm::translate(model, cubePositions[i]);
			GLfloat angle = 20.0f * i;
			model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
			normal = GetNormalMatrix((model));
			glUniformMatrix3fv(glGetUniformLocation(reflection.Program, "normal"), 1, GL_FALSE, glm::value_ptr(normal));
			glUniformMatrix4fv(glGetUniformLocation(reflection.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		// REFRACTION
		refraction.Use();
		glUniformMatrix4fv(glGetUniformLocation(refraction.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(refraction.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glUniform1i(glGetUniformLocation(refraction.Program, "cubemap"), 0);

		glUniform3f(glGetUniformLocation(refraction.Program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

		for (int i = 5; i < 10; i++) {
			model = glm::mat4();
			model = glm::translate(model, cubePositions[i]);
			GLfloat angle = 20.0f * i;
			model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
			normal = GetNormalMatrix((model));
			glUniformMatrix3fv(glGetUniformLocation(refraction.Program, "normal"), 1, GL_FALSE, glm::value_ptr(normal));
			glUniformMatrix4fv(glGetUniformLocation(refraction.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	glBindVertexArray(0);
}
GLuint loadTexture(GLchar* path, bool alpha)
{
	//Generate texture ID and load texture data with SOIL
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(path, &width, &height, 0, alpha ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB);

	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, alpha ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, alpha ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);

	return textureID;
}
GLuint loadCubemap(std::vector<const GLchar*> faces) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	unsigned char* image;
	int width, height;

	for (int i = 0; i < faces.size(); i++) {
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}
// Generates a texture that is suited for attachments to a framebuffer
GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil)
{
	// What enum to use?
	GLenum attachment_type;
	if (!depth && !stencil)
		attachment_type = GL_RGB;
	else if (depth && !stencil)
		attachment_type = GL_DEPTH_COMPONENT;
	else if (!depth && stencil)
		attachment_type = GL_STENCIL_INDEX;

	//Generate texture ID and load texture data 
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (!depth && !stencil)
		glTexImage2D(GL_TEXTURE_2D, 0, attachment_type, screenWidth, screenHeight, 0, attachment_type, GL_UNSIGNED_BYTE, NULL);
	else // Using both a stencil and depth test, needs special format arguments
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screenWidth, screenHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}
// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
// Moves/alters the camera positions based on user input
void Do_Movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
	
}
