#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// include the provided basic shape meshes code
#include "meshes.h"
#include <learnOpengl/camera.h> // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
	const char* const WINDOW_TITLE = "Final Project - Jonathan Rissew"; // Macro for window title

	// Variables for window width and height
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 800;

	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nIndices;    // Number of indices of the mesh
	};

	const char* couchTex = "./resources/textures/couch.jpg";
	const char* metalTex = "./resources/textures/metal.jpg";
	const char* woodFloorTex = "./resources/textures/woodfloor.jpg";
	GLuint gCouchTexId;
	GLuint gMetalTexId;
	GLuint gWoodFloorTexId;
	glm::vec2 gUVScale(5.0f, 5.0f);
	GLint gTexWrapMode = GL_REPEAT;


	// Main GLFW window
	GLFWwindow* gWindow = nullptr;
	// Shader program
	GLuint gProgramId;
	GLuint gLampProgramId;

	//Shape Meshes from Professor Brian
	Meshes meshes;

	enum class Shape {
		CUBE,
		CYLINDER,
		PLANE
	};
}

// camera
Camera gCamera(glm::vec3(0.0f, 3.0f, 20.0f));
// camera speed moved here to be accessible by multiple functions
// const was also removed so that it may be modifiable
float cameraSpeed = 2.5f;
const float cameraSpeedMax = 10.0f;
const float cameraSpeedMin = 0.1f;
float gLastX = WINDOW_WIDTH / 2.0f;
float gLastY = WINDOW_HEIGHT / 2.0f;
bool gFirstMouse = true;
bool isPerspective = true;

// timing
float gDeltaTime = 0.0f; // time between current frame and last frame
float gLastFrame = 0.0f;

// Subject position and scale
glm::vec3 gCubePosition(0.0f, 0.0f, 0.0f);
glm::vec3 gCubeScale(2.0f);

// Cube and light color
glm::vec3 gObjectColor(1.f, 1.0f, 1.0f);
glm::vec3 gLightColor(0.8f, 0.8f, 0.9f);
glm::vec3 gLightColor2(0.5f, 0.5f, 0.5f);

// Light position and scale
glm::vec3 gLightPosition(4.0f, 6.0f, -4.5f);
glm::vec3 gLightPosition2(-3.1f, 5.8f, 1.0f);

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
void MakeShape(GLuint p_texId, glm::vec3 p_scale, float p_rotAmt, glm::vec3 p_rotation, glm::vec3 p_translation, GLint p_modelLoc, Shape p_shape);
////////////////////////////////////////////////////////////////////////////////////////
// SHADER CODE
/* Vertex Shader Source Code*/
/* Cube Vertex Shader Source Code*/
const GLchar* cubeVertexShaderSource = GLSL(440,

	layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

	vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

	vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
	vertexTextureCoordinate = textureCoordinate;
}
);


/* Cube Fragment Shader Source Code*/
const GLchar* cubeFragmentShaderSource = GLSL(440,

	in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 lightColor2;
uniform vec3 lightPos2;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
	/*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

		//Calculate Ambient lighting*/
	float ambientStrength = 0.2f; // Set ambient or global lighting strength
	vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

	//Calculate Diffuse lighting*/
	vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
	vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse = impact * lightColor; // Generate diffuse light color

	vec3 lightDirection2 = normalize(lightPos2 - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact2 = max(dot(norm, lightDirection2), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse2 = impact2 * lightColor2; // Generate diffuse light color

	//Calculate Specular lighting*/
	float specularIntensity = 1.0f; // Set specular light strength
	float highlightSize = 16.0f; // Set specular highlight size
	float specularIntensity2 = 0.1f; // Set specular light strength
	float highlightSize2 = 16.0f; // Set specular highlight size
	vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
	vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
	vec3 reflectDir2 = reflect(-lightDirection2, norm);// Calculate reflection vector
	//Calculate specular component
	float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
	vec3 specular = specularIntensity * specularComponent * lightColor;

	//Calculate specular component
	float specularComponent2 = pow(max(dot(viewDir, reflectDir2), 0.0), highlightSize2);
	vec3 specular2 = specularIntensity2 * specularComponent2 * lightColor2;

	// Texture holds the color to be used for all three components
	vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

	// Calculate phong result
	vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;
	vec3 phong2 = (ambient + diffuse2 + specular2) * textureColor.xyz;

	fragmentColor = vec4(phong + phong2, 1.0); // Send lighting results to GPU
}
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

	layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

		//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

	out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
	fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);
///////////////////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[])
{
	if (!UInitialize(argc, argv, &gWindow))
		return EXIT_FAILURE;

	// Create the basic shape meshes for use
	meshes.CreateMeshes();

	// Create the shader program
	if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gProgramId))
		return EXIT_FAILURE;

	// Load textures
	
	if (!UCreateTexture(couchTex, gCouchTexId))
	{
		cout << "Failed to load texture " << couchTex << endl;
		return EXIT_FAILURE;
	}
	if (!UCreateTexture(metalTex, gMetalTexId))
	{
		cout << "Failed to load texture " << metalTex << endl;
		return EXIT_FAILURE;
	}
	if (!UCreateTexture(woodFloorTex, gWoodFloorTexId))
	{
		cout << "Failed to load texture " << woodFloorTex << endl;
		return EXIT_FAILURE;
	}
	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	glUseProgram(gProgramId);
	// We set the texture as texture unit 0
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);
	// Sets the background color of the window to black (it will be implicitely used by glClear)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(gWindow))
	{
		// per-frame timing
		// --------------------
		float currentFrame = glfwGetTime();
		gDeltaTime = currentFrame - gLastFrame;
		gLastFrame = currentFrame;

		// input
		// -----
		UProcessInput(gWindow);

		// Render this frame
		URender();

		glfwPollEvents();
	}

	// Release mesh data
	//UDestroyMesh(gMesh);
	meshes.DestroyMeshes();

	// Release texture
	UDestroyTexture(gCouchTexId);
	UDestroyTexture(gMetalTexId);
	UDestroyTexture(gWoodFloorTexId);

	// Release shader program
	UDestroyShaderProgram(gProgramId);

	exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
	// GLFW: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW: window creation
	// ---------------------
	*window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, UResizeWindow);
	glfwSetCursorPosCallback(*window, UMousePositionCallback);
	glfwSetScrollCallback(*window, UMouseScrollCallback);

	// Capture mouse for mouse movement control
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLEW: initialize
	// ----------------
	// Note: if using GLEW version 1.13 or earlier
	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();

	if (GLEW_OK != GlewInitResult)
	{
		std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
		return false;
	}

	// Displays GPU OpenGL version
	cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

	return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
	// Exit program on escape key
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		isPerspective = true;
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		isPerspective = false;

	// Apply cameraSpeed which can be modified with scroll wheel to
	// the built in gCamera speed value
	gCamera.MovementSpeed = cameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		gCamera.ProcessKeyboard(LEFT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		gCamera.ProcessKeyboard(UP, gDeltaTime);
		//gCamera.Position.y += cameraSpeed * gDeltaTime;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		gCamera.ProcessKeyboard(DOWN, gDeltaTime);
		//gCamera.Position.y -= cameraSpeed * gDeltaTime;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (gFirstMouse)
	{
		gLastX = xpos;
		gLastY = ypos;
		gFirstMouse = false;
	}

	float xoffset = xpos - gLastX;
	float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

	gLastX = xpos;
	gLastY = ypos;

	if (isPerspective)
		gCamera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	// This code was changing zoom which does make the camera faster in appearance
	//gCamera.ProcessMouseScroll(yoffset);

	// I added this line to actually make the speed of camera movement faster with scrolling
	float offset = (float)yoffset;
	cameraSpeed += offset;
	std::cout << offset;

	if (cameraSpeed > cameraSpeedMax)
		cameraSpeed = cameraSpeedMax;
	else if (cameraSpeed < cameraSpeedMin)
		cameraSpeed = cameraSpeedMin;

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void MakeShape(GLuint p_texId, glm::vec3 p_scale, float p_rotAmt, glm::vec3 p_rotation, glm::vec3 p_translation, GLint p_modelLoc, Shape p_shape) {
	/// First couch leg
	///-------Transform and draw the cylinder mesh --------
	// Activate the VBOs contained within the mesh's VAO
	glBindTexture(GL_TEXTURE_2D, p_texId);
	switch (p_shape) {
	case Shape::CUBE:glBindVertexArray(meshes.gBoxMesh.vao); break;
	case Shape::CYLINDER: glBindVertexArray(meshes.gCylinderMesh.vao); break;
	case Shape::PLANE: glBindVertexArray(meshes.gPlaneMesh.vao); break;
	}
	
	// 1. Scales the object
	glm::mat4 scale = glm::scale(p_scale);
	// 2. Rotate the object
	glm::mat4 rotation = glm::rotate(p_rotAmt, p_rotation);
	// 3. Position the object
	glm::mat4 translation = glm::translate(p_translation);
	// Model matrix: transformations are applied right-to-left order
	glm::mat4 model = translation * rotation * scale;
	glUniformMatrix4fv(p_modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	switch (p_shape) {
		case Shape::CUBE: {
			glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);
			break;
		}
		case Shape::CYLINDER: {
			glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
			glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
			glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
			break;
		}
		case Shape::PLANE: {
			glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);
			break;
		}
	}

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
}

// Functioned called to render a frame
void URender()
{
	glm::mat4 scale;
	glm::mat4 rotation;
	glm::mat4 translation;
	glm::mat4 model;
	GLint modelLoc;
	GLint viewLoc;
	GLint projLoc;
		
	const glm::vec3 legScale = glm::vec3(0.1f, 0.4f, 0.1f);

	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the frame and z buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// camera/view transformation
	glm::mat4 view = gCamera.GetViewMatrix();

	// Creates a  projection that can be toggled between perspective and orthographic
	glm::mat4 projection;
	if (isPerspective)
		projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	else
		projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);

	// Set the shader to be used
	glUseProgram(gProgramId);

	// Retrieves and passes transform matrices to the Shader program
	modelLoc = glGetUniformLocation(gProgramId, "model");
	viewLoc = glGetUniformLocation(gProgramId, "view");
	projLoc = glGetUniformLocation(gProgramId, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); 
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
	GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");;
	GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
	GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
	GLint lightColorLoc2 = glGetUniformLocation(gProgramId, "lightColor2");
	GLint lightPositionLoc2 = glGetUniformLocation(gProgramId, "lightPos2");
	GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
	
	// Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
	glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
	glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
	glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
	glUniform3f(lightColorLoc2, gLightColor2.r, gLightColor2.g, gLightColor2.b);
	glUniform3f(lightPositionLoc2, gLightPosition2.x, gLightPosition2.y, gLightPosition2.z);
	const glm::vec3 cameraPosition = gCamera.Position;
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

	// Floor Plane
	MakeShape(gWoodFloorTexId, // Texture
				glm::vec3(9.0f, 1.0f, 8.0f), // Scale
				0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
				glm::vec3(3.5f, 0.0f, -2.0f), // Translation
				modelLoc, Shape::PLANE);

	// Close Left Couch Leg
	MakeShape(gMetalTexId, // Texture
		glm::vec3(0.1f, 0.4f, 0.1f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(-4.7f, 0.01f, -2.0f), // Translation
		modelLoc, Shape::CYLINDER);

	// Close Right Couch Leg
	MakeShape(gMetalTexId, // Texture
		glm::vec3(0.1f, 0.4f, 0.1f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(-2.5f, 0.01f, -2.0f), // Translation
		modelLoc, Shape::CYLINDER);

	// Back Middle Couch Leg
	MakeShape(gMetalTexId, // Texture
		glm::vec3(0.1f, 0.4f, 0.1f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(-1.9f, 0.01f, -7.0f), // Translation
		modelLoc, Shape::CYLINDER);

	// Back Right Couch Leg
	MakeShape(gMetalTexId, // Texture
		glm::vec3(0.1f, 0.4f, 0.1f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(3.0f, 0.01f, -7.0f), // Translation
		modelLoc, Shape::CYLINDER);

	// Closest Seat Cushion
	MakeShape(gCouchTexId, // Texture
		glm::vec3(3.0f, 1.5f, 8.0f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(-3.5f, 1.0f, -5.75f), // Translation
		modelLoc, Shape::CUBE);

	// Left Side Back Rest
	MakeShape(gCouchTexId, // Texture
		glm::vec3(1.0f, 1.5f, 6.0f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(-4.5f, 2.5f, -6.75f), // Translation
		modelLoc, Shape::CUBE);

	// Further Seat Cushion
	MakeShape(gCouchTexId, // Texture
		glm::vec3(5.5f, 1.5f, 3.0f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(0.75f, 1.0f, -8.25f), // Translation
		modelLoc, Shape::CUBE);

	// Further Back Rest
	MakeShape(gCouchTexId, // Texture
		glm::vec3(7.5f, 1.5f, 0.5f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(-0.25f, 2.5f, -9.5f), // Translation
		modelLoc, Shape::CUBE);

	// Right Side Arm Rest
	MakeShape(gCouchTexId, // Texture
		glm::vec3(0.5f, 2.5f, 3.125f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(3.75f, 1.5f, -8.25f), // Translation
		modelLoc, Shape::CUBE);

	// Table Left Leg
	MakeShape(gWoodFloorTexId, // Texture
		glm::vec3(0.15f, 1.5f, 3.125f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(0.0f, 0.751f, -1.25f), // Translation
		modelLoc, Shape::CUBE);

	// Table Right Leg
	MakeShape(gWoodFloorTexId, // Texture
		glm::vec3(0.15f, 1.5f, 3.125f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(4.0f, 0.751f, -1.25f), // Translation
		modelLoc, Shape::CUBE);

	// Table Center Leg
	MakeShape(gWoodFloorTexId, // Texture
		glm::vec3(0.15f, 1.5f, 3.125f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(1.8f, 0.751f, -1.25f), // Translation
		modelLoc, Shape::CUBE);

	// Table Surface
	MakeShape(gWoodFloorTexId, // Texture
		glm::vec3(4.15f, 0.15f, 3.125f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(2.0f, 1.575f, -1.25f), // Translation
		modelLoc, Shape::CUBE);

	// Plate
	MakeShape(gMetalTexId, // Texture
		glm::vec3(0.4f, 0.1f, 0.4f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(2.8f, 1.6f, -1.5f), // Translation
		modelLoc, Shape::CYLINDER);

	// Lamp Leg Back Right
	MakeShape(gWoodFloorTexId, // Texture
		glm::vec3(0.1f, 1.4f, 0.1f), // Scale
		0.5f, glm::vec3(0.5f, 0.0f, 0.5f), // Rotation
		glm::vec3(-3.0f, 0.05f, 1.0f), // Translation
		modelLoc, Shape::CYLINDER);

	// Lamp Leg Front
	MakeShape(gWoodFloorTexId, // Texture
		glm::vec3(0.1f, 1.4f, 0.1f), // Scale
		0.5f, glm::vec3(-0.5f, 0.0f, 0.0f), // Rotation
		glm::vec3(-3.7f, 0.05f, 2.5f), // Translation
		modelLoc, Shape::CYLINDER);

	// Lamp Leg Back Right
	MakeShape(gWoodFloorTexId, // Texture
		glm::vec3(0.1f, 1.4f, 0.1f), // Scale
		0.5f, glm::vec3(0.5f, 0.0f, -0.5f), // Rotation
		glm::vec3(-4.4f, 0.05f, 1.0f), // Translation
		modelLoc, Shape::CYLINDER);

	// Lamp Leg Connector Back Left Bottom
	MakeShape(gMetalTexId, // Texture
		glm::vec3(0.025f, 0.05f, 0.8f), // Scale
		0.9f, glm::vec3(0.0, 1.0f, 0.0f), // Rotation
		glm::vec3(-4.0f, 0.22f, 1.35f), // Translation
		modelLoc, Shape::CUBE);

	// Lamp Leg Connectors Back Left Upper
	MakeShape(gMetalTexId, // Texture
		glm::vec3(0.025f, 0.05f, 0.2f), // Scale
		0.9f, glm::vec3(0.0, 1.0f, 0.0f), // Rotation
		glm::vec3(-3.8f, 1.2f, 1.55f), // Translation
		modelLoc, Shape::CUBE);

	// Lamp Leg Connectors Back Right Lower
	MakeShape(gMetalTexId, // Texture
		glm::vec3(0.025f, 0.05f, 0.8f), // Scale
		0.9f, glm::vec3(0.0, -0.5f, 0.0f), // Rotation
		glm::vec3(-3.4f, 0.22f, 1.35f), // Translation
		modelLoc, Shape::CUBE);

	// Lamp Leg Connectors Back Right Upper
	MakeShape(gMetalTexId, // Texture
		glm::vec3(0.025f, 0.05f, 0.2f), // Scale
		1.0f, glm::vec3(0.0, -0.5f, 0.0f), // Rotation
		glm::vec3(-3.6f, 1.2f, 1.55f), // Translation
		modelLoc, Shape::CUBE);

	// Lamp Leg Connectors Front Bottom
	MakeShape(gMetalTexId, // Texture
		glm::vec3(0.025f, 0.05f, 0.8f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(-3.7f, 0.22f, 2.0f), // Translation
		modelLoc, Shape::CUBE);

	// Lamp Leg Connectors Front Upper
	MakeShape(gMetalTexId, // Texture
		glm::vec3(0.025f, 0.05f, 0.2f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(-3.7f, 1.2f, 1.71f), // Translation
		modelLoc, Shape::CUBE);

	// Lamp Pole
	MakeShape(gWoodFloorTexId, // Texture
		glm::vec3(0.05f, 5.5f, 0.05f), // Scale
		0.0f, glm::vec3(1.0f, 1.0f, 1.0f), // Rotation
		glm::vec3(-3.7f, 0.18f, 1.6f), // Translation
		modelLoc, Shape::CYLINDER);

	// Lamp Arm
	MakeShape(gWoodFloorTexId, // Texture
		glm::vec3(0.05f, 0.5f, 0.05f), // Scale
		1.0f, glm::vec3(-1.0f, 0.0f, -1.0f), // Rotation
		glm::vec3(-3.7f, 5.68f, 1.6f), // Translation
		modelLoc, Shape::CYLINDER);

	// Lamp Shade
	MakeShape(gWoodFloorTexId, // Texture
		glm::vec3(0.2f, 0.3f, 0.2f), // Scale
		1.0f, glm::vec3(1.0f, 0.0, 1.0f), // Rotation
		glm::vec3(-3.2f, 5.9f, 1.1f), // Translation
		modelLoc, Shape::CYLINDER);

	// Lamp Shade Bigger Piece
	MakeShape(gWoodFloorTexId, // Texture
		glm::vec3(0.4f, 0.2f, 0.4f), // Scale
		1.0f, glm::vec3(1.0f, 0.0, 1.0f), // Rotation
		glm::vec3(-3.1f, 5.8f, 1.0f), // Translation
		modelLoc, Shape::CYLINDER);

	glUseProgram(0);

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}

// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint &programId)
{
	// Compilation and linkage error reporting
	int success = 0;
	char infoLog[512];

	// Create a Shader program object.
	programId = glCreateProgram();

	// Create the vertex and fragment shader objects
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrive the shader source
	glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
	glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

	// Compile the vertex shader, and print compilation errors (if any)
	glCompileShader(vertexShaderId); // compile the vertex shader
	// check for shader compile errors
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glCompileShader(fragmentShaderId); // compile the fragment shader
	// check for shader compile errors
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	// Attached compiled shaders to the shader program
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);

	glLinkProgram(programId);   // links the shader program
	// check for linking errors
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glUseProgram(programId);    // Uses the shader program

	return true;
}

// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
	for (int j = 0; j < height / 2; ++j)
	{
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;

		for (int i = width * channels; i > 0; --i)
		{
			unsigned char tmp = image[index1];
			image[index1] = image[index2];
			image[index2] = tmp;
			++index1;
			++index2;
		}
	}
}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
	if (image)
	{
		flipImageVertically(image, width, height, channels);
		std::cout << textureId << std::endl;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << " channels" << endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		return true;
	}

	// Error loading the image
	return false;
}

void UDestroyTexture(GLuint textureId)
{
	glGenTextures(1, &textureId);
}

void UDestroyShaderProgram(GLuint programId)
{
	glDeleteProgram(programId);
}