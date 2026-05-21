// Project502MVHerpHCaeyman.cpp : Defines the entry point for the application.
//

#include<iostream>

//include glad before GLFW to avoid header conflict or define "#define GLFW_INCLUDE_NONE"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/matrix_inverse.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include<map>

#include "camera.h"
#include "shader.h"
#include "object.h"


const int WIDTH = 500;
const int HEIGHT = 500;

float screenVertices[] = {
	// Coords		// texCoords
	1.0f, -1.0f,	1.0f, 0.0f,
	-1.0f, -1.0f,	0.0f, 0.0f,
	-1.0f, 1.0f,	0.0f, 1.0f,

	1.0f, 1.0f,		1.0f, 1.0f,
	1.0f, -1.0f,	1.0f, 0.0f,
	-1.0f, 1.0f,	0.0f, 1.0f
};


GLuint compileShader(std::string shaderCode, GLenum shaderType);
GLuint compileProgram(GLuint vertexShader, GLuint fragmentShader);
void processInput(GLFWwindow* window);

void loadCubemapFace(const char* file, const GLenum& targetCube);



#ifndef NDEBUG
void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}
#endif

Camera camera(glm::vec3(1.0, 0.0, -6.0), glm::vec3(0.0, 1.0, 0.0), 90.0);


int main(int argc, char* argv[])
{
	std::cout << "Welcome to exercice 3: " << std::endl;
	
	//Boilerplate
	//Create the OpenGL context 
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialise GLFW \n");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifndef NDEBUG
	//create a debug context to help with Debugging
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif


	//Create the window
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Project 502", nullptr, nullptr);
	if (window == NULL)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window\n");
	}

	glfwMakeContextCurrent(window);


	//load openGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("Failed to initialize GLAD");
	}

	glEnable(GL_DEPTH_TEST);

#ifndef NDEBUG
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif


	Shader shader(PATH_TO_SHADERS"/advanceLight.vert", PATH_TO_SHADERS"/advanceLight.frag");
	Shader skyBoxShader(PATH_TO_SHADERS"/skyBox.vert", PATH_TO_SHADERS"/skyBox.frag");
	Shader framebufferProgram(PATH_TO_SHADERS"/post.vert", PATH_TO_SHADERS"/postKuwaharaCircle.frag");

	//1. Load the model for 3 types of spheres

	char path1[] = PATH_TO_OBJECTS"/sphere_extremely_coarse.obj";
	char path2[] = PATH_TO_OBJECTS"/sphere_coarse.obj";
	char path3[] = PATH_TO_OBJECTS"/sphere_smooth.obj";
	char pathCube[] = PATH_TO_OBJECTS "/cube.obj";
	Object skyBox(pathCube);
	skyBox.makeObject(skyBoxShader);

	Object sphere1(path1);
	sphere1.makeObject(shader);
	sphere1.model = glm::translate(sphere1.model, glm::vec3(2.3, 0.0, 0.0));
	sphere1.model = glm::scale(sphere1.model, glm::vec3(0.5, 0.5, 0.5));

	Object sphere2(path2);
	sphere2.makeObject(shader);
	sphere2.model = glm::translate(sphere2.model, glm::vec3(0.0, 0.0, 0.0));
	sphere2.model = glm::scale(sphere2.model, glm::vec3(0.5, 0.5, 0.5));

	Object sphere3(path3);
	sphere3.makeObject(shader);
	sphere3.model = glm::translate(sphere3.model, glm::vec3(-2.3, 0.0, -0.0));
	sphere3.model = glm::scale(sphere3.model, glm::vec3(0.5, 0.5, 0.5));

	// Load necessary textures
	GLuint texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);


	//3. Define the parameters for the texture

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//4. Load the image
	//Carefull depending on where your executable is, the relative path might be different from what you think it is
	//Try to use an absolute path
	//image usually have thei 0.0 at the top of the vertical axis and not the bottom like opengl expects

	stbi_set_flip_vertically_on_load(true);
	int width, height, nrchannels;
	unsigned char* data = stbi_load(PATH_TO_TEXTURE"/horse.jpg", &width, &height, &nrchannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "failed to load texture" << std::endl;
		const char* reason = stbi_failure_reason();
		std::cout << reason << std::endl;
	}
	stbi_image_free(data);


	//2. Choose a position for the light
	const glm::vec3 light_pos = glm::vec3(1.0, 2.0, 1.5);

	double prev = 0;
	int deltaFrame = 0;
	//fps function
	auto fps = [&](double now) {
		double deltaTime = now - prev;
		deltaFrame++;
		if (deltaTime > 0.5) {
			prev = now;
			const double fpsCount = (double)deltaFrame / deltaTime;
			deltaFrame = 0;
			std::cout << "\r FPS: " << fpsCount;
		}
		};

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();

	// create shader shenanigans for the postproccessing

	unsigned int rectVAO, rectVBO;
	glGenVertexArrays(1, &rectVAO);
	glGenBuffers(1, &rectVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), &screenVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// create and bind framebuffer for postprocessing
	unsigned int FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	//create (and bind) color texture and scale to the size of screen
	unsigned int framebufferTexture;
	glGenTextures(1, &framebufferTexture);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	auto fbostatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fbostatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "framebuffer error" << fbostatus << std::endl;

	// create postprocessing uniforms
	framebufferProgram.use();
	framebufferProgram.setInteger("screenTexture", 0);


	// SPECIAL advance lighting
	float ambient = 0.1f;
	float diffuse = 0.5f;
	float specular = 0.8f;

	glm::vec3 materialColour = glm::vec3(0.5f, 0.6f, 0.8f);

	shader.use();
	shader.setFloat("shininess", 32.0f);
	shader.setVector3f("materialColour", materialColour);
	shader.setFloat("light.ambient_strength", ambient);
	shader.setFloat("light.diffuse_strength", diffuse);
	shader.setFloat("light.specular_strength", specular);
	shader.setFloat("light.constant", 1.0);
	shader.setFloat("light.linear", 0.14);
	shader.setFloat("light.quadratic", 0.07);
	// SPECIAL advance lighting

	//cubeMap (skyBox)
	GLuint cubeMapTexture;
	glGenTextures(1, &cubeMapTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);

	// texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(false);

	std::string pathToCubeMap = PATH_TO_TEXTURE "/cubemaps/yokohama3/";

	std::map<std::string, GLenum> facesToLoad = {
		{pathToCubeMap + "posx.jpg",GL_TEXTURE_CUBE_MAP_POSITIVE_X},
		{pathToCubeMap + "posy.jpg",GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
		{pathToCubeMap + "posz.jpg",GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
		{pathToCubeMap + "negx.jpg",GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
		{pathToCubeMap + "negy.jpg",GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
		{pathToCubeMap + "negz.jpg",GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
	};
	//load the six faces
	for (std::pair<std::string, GLenum> pair : facesToLoad) {
		loadCubemapFace(pair.first.c_str(), pair.second);
	}

	//6. get the uniform location for the texture
	auto u_texture = glGetUniformLocation(shader.ID, "T");

	/*
	Refraction indices:
	Air:      1.0
	Water:    1.33
	Ice:      1.309
	Glass:    1.52
	Diamond:  2.42
*/
	shader.setFloat("refractionIndice", 1.0);

	//Rendering
	glfwSwapInterval(1);

	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		view = camera.GetViewMatrix();
		glfwPollEvents();
		double now = glfwGetTime();

		// binding frame  buffer for postProcessing (stores the screen image in the postbuffer instead of the normal one)

		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		// Clear back buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);


		//2. Use the shader Class to send the relevant uniform
		shader.use();

		shader.setMatrix4("M", sphere1.model);
		shader.setMatrix4("itM", glm::transpose(glm::inverse(sphere1.model)));
		shader.setMatrix4("V", view);
		shader.setMatrix4("P", perspective);
		shader.setVector3f("u_view_pos", camera.Position);
		//what other uniforms do you need to send
		shader.setVector3f("light.light_pos", light_pos);

		//6. Send the texture
		glUniform1i(u_texture, 0); //use ID of correct texture

		//7. activate the texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glDepthFunc(GL_LEQUAL);

		//don't forget to draw your objects
		sphere1.draw();

		shader.setMatrix4("M", sphere2.model);
		shader.setMatrix4("itM", glm::transpose(glm::inverse(sphere2.model)));
		sphere2.draw();

		shader.setMatrix4("M", sphere3.model);
		shader.setMatrix4("itM", glm::transpose(glm::inverse(sphere3.model)));
		sphere3.draw();

		//sphere1.model = glm::translate(sphere1.model, glm::vec3(std::sin(now) * 0.05, 0.05, std::cos(now) * 0.05));
		//sphere2.model = glm::translate(sphere1.model, glm::vec3(std::sin(now) * 0.05, 0.05, std::cos(now) * 0.05));
		//sphere3.model = glm::translate(sphere1.model, glm::vec3(std::sin(now) * 0.05, 0.05, std::cos(now) * 0.05));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
		//skyBoxShader.setInteger("cubemapTexture", 0); // no use ???

		skyBoxShader.use();
		skyBoxShader.setMatrix4("V", view);
		skyBoxShader.setMatrix4("P", perspective);
		//skyBoxShader.setInteger("cubemapTexture", 0); // no use ???

		skyBox.draw();
		glDepthFunc(GL_LESS);

		// PostProcessing
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // rebind normal screen buffer to show result after operations
		framebufferProgram.use();
		glBindVertexArray(rectVAO);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, framebufferTexture);

		framebufferProgram.setVector2f("screenSize", WIDTH, HEIGHT);
		glDrawArrays(GL_TRIANGLES, 0, 6);


		fps(now);
		glfwSwapBuffers(window);
	}

	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void loadCubemapFace(const char* path, const GLenum& targetFace)
{
	int imWidth, imHeight, imNrChannels;
	unsigned char* data = stbi_load(path, &imWidth, &imHeight, &imNrChannels, 0);
	if (data)
	{

		glTexImage2D(targetFace, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(targetFace);
	}
	else {
		std::cout << "Failed to Load texture" << std::endl;
		const char* reason = stbi_failure_reason();
		std::cout << reason << std::endl;
	}
	stbi_image_free(data);
}


void processInput(GLFWwindow* window) {
	// Use the cameras class to change the parameters of the camera
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(LEFT, 0.1);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(RIGHT, 0.1);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(FORWARD, 0.1);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(BACKWARD, 0.1);

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(1, 0.0, 1);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(-1, 0.0, 1);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(0.0, 1.0, 1);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(0.0, -1.0, 1);


}


