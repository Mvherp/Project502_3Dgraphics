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

#include<map>

#include "camera.h"
#include "shader.h"
#include "object.h"
#include "texture.h"


const int WIDTH = 1680;
const int HEIGHT = 1240;
bool CAMLOCK = false;
bool POSTPAINT = false;

float screenVertices[] = {
	// Coords		// texCoords
	1.0f, -1.0f,	1.0f, 0.0f,
	-1.0f, -1.0f,	0.0f, 0.0f,
	-1.0f, 1.0f,	0.0f, 1.0f,

	1.0f, 1.0f,		1.0f, 1.0f,
	1.0f, -1.0f,	1.0f, 0.0f,
	-1.0f, 1.0f,	0.0f, 1.0f
};


/*
	Refraction indices:
	Air:      1.0
	Water:    1.33
	Ice:      1.309
	Glass:    1.52
	Diamond:  2.42
*/
float REFRACTION_INDEX = 1.33;


GLuint compileShader(std::string shaderCode, GLenum shaderType);
GLuint compileProgram(GLuint vertexShader, GLuint fragmentShader);
void processInput(GLFWwindow* window, double delta);

void loadCubemapFace(const char* file, const GLenum& targetCube);
float PLAYERROTATION = 0;



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

Camera camera(glm::vec3(0.0, 8.0, -10), glm::vec3(0.0, 1.0, 0.0), 90.0, -30.0);


char pathPlayer[] = PATH_TO_OBJECTS "/compact_classic.obj"; // funny car i guess
Object player(pathPlayer);

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
	Shader illuminated(PATH_TO_SHADERS"/advanceLight.vert", PATH_TO_SHADERS"/advanceLight.frag");
	Shader skyBoxShader(PATH_TO_SHADERS"/skyBox.vert", PATH_TO_SHADERS"/skyBox.frag");
	Shader reflective(PATH_TO_SHADERS"/reflective.vert", PATH_TO_SHADERS"/reflective.frag");
	Shader refractive(PATH_TO_SHADERS"/refractive.vert", PATH_TO_SHADERS"/refractive.frag");

	Shader framebufferProgram(PATH_TO_SHADERS"/post.vert", PATH_TO_SHADERS"/postNoEffects.frag");
	Shader postPaint(PATH_TO_SHADERS"/post.vert", PATH_TO_SHADERS"/postKuwaharaCircle.frag");


	//1. Load the model for 3 types of spheres

	// SCENE paths
	char pathCube[] = PATH_TO_OBJECTS "/cube.obj";
	char groundP[] = PATH_TO_OBJECTS"/plane.obj";

	char bowlP[] = PATH_TO_OBJECTS"/plate.obj";
	char appleP[] = PATH_TO_OBJECTS"/apple.obj";
	char orangeP[] = PATH_TO_OBJECTS"/orange.obj";
	char bananaP[] = PATH_TO_OBJECTS"/banana.obj";
	char grapeP[] = PATH_TO_OBJECTS"/grape.obj";


	//place holders
	char path2[] = PATH_TO_OBJECTS"/sphere_coarse.obj";
	char path3[] = PATH_TO_OBJECTS"/sphere_smooth.obj";

	//place holders
	/*Object sphere2(path2);
	sphere2.makeObject(shader);
	sphere2.model = glm::translate(sphere2.model, glm::vec3(0.0, 1.0, 5.0));
	sphere2.model = glm::scale(sphere2.model, glm::vec3(0.5, 0.5, 0.5));

	Object sphere3(path3);
	sphere3.makeObject(shader);
	sphere3.model = glm::translate(sphere3.model, glm::vec3(-2.3, 1.0, 5.0));
	sphere3.model = glm::scale(sphere3.model, glm::vec3(0.5, 0.5, 0.5));

	Object sphere4(path3);
	sphere4.makeObject(reflective);
	sphere4.model = glm::translate(sphere4.model, glm::vec3(-4.6, 1.0, 5.0));
	sphere4.model = glm::scale(sphere4.model, glm::vec3(0.5, 0.5, 0.5));

	Object sphere5(path3);
	sphere5.makeObject(refractive);
	sphere5.model = glm::translate(sphere5.model, glm::vec3(-6.9, 1.0, 5.0));
	sphere5.model = glm::scale(sphere5.model, glm::vec3(0.5, 0.5, 0.5));*/


	// SCENE objects
	Object skyBox(pathCube);
	skyBox.makeObject(skyBoxShader);

	Object ground(groundP);
	ground.makeObject(shader);
	ground.model = glm::translate(ground.model, glm::vec3(0.0, 0.0, 0.0));
	ground.model = glm::scale(ground.model, glm::vec3(10, 0.0, 10));

	Object bowl(bowlP);
	bowl.makeObject(reflective);
	bowl.model = glm::translate(bowl.model, glm::vec3(0.0, 1.0, 0.0));
	bowl.model = glm::scale(bowl.model, glm::vec3(1, 1, 1));

	Object apple(appleP);
	apple.makeObject(shader);
	apple.model = glm::translate(apple.model, glm::vec3(0.0, 1.0, 0.0));
	apple.model = glm::scale(apple.model, glm::vec3(1, 1, 1));
	apple.model = glm::rotate(apple.model, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));


	Object orange(orangeP);
	orange.makeObject(shader);
	orange.model = glm::translate(orange.model, glm::vec3(0.0, 1.0, 0.0));
	orange.model = glm::scale(orange.model, glm::vec3(1, 1, 1));
	orange.model = glm::rotate(orange.model, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));


	Object banana(bananaP);
	banana.makeObject(shader);
	banana.model = glm::translate(banana.model, glm::vec3(0.0, 1.0, 0.0));
	banana.model = glm::scale(banana.model, glm::vec3(1, 1, 1));
	banana.model = glm::rotate(banana.model, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));


	Object grape(grapeP);
	grape.makeObject(shader);
	grape.model = glm::translate(grape.model, glm::vec3(0.0, 1.0, 0.0));
	grape.model = glm::scale(grape.model, glm::vec3(1, 1, 1));
	grape.model = glm::rotate(grape.model, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));



	player.makeObject(shader);
	player.model = glm::translate(player.model, glm::vec3(10.0, 0.0, 0.0));
	player.model = glm::rotate(player.model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
	//player.model = glm::scale(apple.model, glm::vec3(1.0,1.0,1.0));

	
	Texture horseT(PATH_TO_TEXTURE"/horse.jpg");
	Texture carT(PATH_TO_TEXTURE"/carTextures/java_green.png");

	// SCENE textures
	Texture groundT(PATH_TO_TEXTURE"/grassTexture.png");
	Texture bowlT(PATH_TO_TEXTURE"/metal.jpg");

	Texture appleT(PATH_TO_TEXTURE"/apple.png");
	Texture orangeT(PATH_TO_TEXTURE"/orangeCombined.png");
	Texture bananaT(PATH_TO_TEXTURE"/banana.png");
	Texture grapeT(PATH_TO_TEXTURE"/grape.png");




	//2. Choose a position for the light
	const glm::vec3 light_pos = glm::vec3(10.0, 20.0, 5.0);

	double prev = 0;
	int deltaFrame = 0;
	double delta;
	//fps function
	auto fps = [&](double now) {
		double deltaTime = now - prev;
		deltaFrame++;
		if (deltaTime > 0.5) {
			prev = now;
			const double fpsCount = (double)deltaFrame / deltaTime;
			delta = 30.0 / fpsCount;
			deltaFrame = 0;
			std::cout << "\r FPS: " << fpsCount;
		}
		};

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix(45.0, WIDTH/HEIGHT);

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

	postPaint.use();
	postPaint.setInteger("screenTexture", 0);


	// SPECIAL advance lighting
	float ambient = 0.4f;
	float diffuse = 0.6f;
	float specular = 0.9f;

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

	illuminated.use();
	illuminated.setFloat("shininess", 32.0f);
	illuminated.setVector3f("materialColour", materialColour);
	illuminated.setFloat("light.ambient_strength", 0.5);
	illuminated.setFloat("light.diffuse_strength", 0.7);
	illuminated.setFloat("light.specular_strength", 0.7);
	illuminated.setFloat("light.constant", 1.0);
	illuminated.setFloat("light.linear", 0.14);
	illuminated.setFloat("light.quadratic", 0.07);
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

	std::string pathToCubeMap = PATH_TO_TEXTURE "/cubemaps/SanFrancisco2/";

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
	//auto u_texture = glGetUniformLocation(shader.ID, "T");
	//auto reflective_texture = glGetUniformLocation(reflective.ID, "T");

	refractive.setFloat("refractionIndice", REFRACTION_INDEX);

	//Rendering
	glfwSwapInterval(1);
	double now = glfwGetTime();
	
	while (!glfwWindowShouldClose(window)) {
		processInput(window, delta);
		view = camera.GetViewMatrix();
		glfwPollEvents();
		now = glfwGetTime();

		// binding frame  buffer for postProcessing (stores the screen image in the postbuffer instead of the normal one)

		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		// Clear back buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);


		//2. Use the shader Class to send the relevant uniform
		shader.use();
		shader.setMatrix4("V", view);
		shader.setMatrix4("P", perspective);
		shader.setVector3f("u_view_pos", camera.Position);
		//what other uniforms do you need to send
		shader.setVector3f("light.light_pos", light_pos);

		//2. Use the shader Class to send the relevant uniform
		illuminated.use();
		illuminated.setMatrix4("V", view);
		illuminated.setMatrix4("P", perspective);
		illuminated.setVector3f("u_view_pos", camera.Position);
		//what other uniforms do you need to send
		illuminated.setVector3f("light.light_pos", light_pos);

		//2. Use the shader Class to send the relevant uniform
		reflective.use();
		reflective.setMatrix4("V", view);
		reflective.setMatrix4("P", perspective);
		reflective.setVector3f("u_view_pos", camera.Position);
		//what other uniforms do you need to send
		reflective.setVector3f("light.light_pos", light_pos);

		//2. Use the shader Class to send the relevant uniform
		refractive.use();
		refractive.setMatrix4("V", view);
		refractive.setMatrix4("P", perspective);
		refractive.setVector3f("u_view_pos", camera.Position);
		//what other uniforms do you need to send
		refractive.setVector3f("light.light_pos", light_pos);
		refractive.setFloat("refractionIndice", REFRACTION_INDEX);
		

		//6. Send the texture
		//shader.setInteger("T", 0); //

		//7. activate the texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, carT.ID);

		glDepthFunc(GL_LEQUAL);

		//don't forget to draw your objects
		player.draw(shader);

		// SCENE draw
		reflective.use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bowlT.ID);
		glUniform1i(glGetUniformLocation(reflective.ID, "T"), 0);
		// cubemap
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
		glUniform1i(glGetUniformLocation(reflective.ID, "cubemapSampler"), 1);

		bowl.draw(reflective);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, appleT.ID);

		apple.draw(shader);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, orangeT.ID);

		orange.draw(shader);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bananaT.ID);

		banana.draw(shader);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grapeT.ID);

		grape.draw(shader);

		// SCENE

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, horseT.ID);

		//sphere2.draw(shader);

		//sphere3.draw(shader);

		reflective.use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, appleT.ID);
		glUniform1i(glGetUniformLocation(reflective.ID, "T"), 0);

		// cubemap
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
		glUniform1i(glGetUniformLocation(reflective.ID, "cubemapSampler"), 1);

		//sphere4.draw(reflective);
		//reflective.setMatrix4();

		//sphere5.draw(refractive);
		//sphere5.model = glm::translate(sphere5.model, glm::vec3(0.0,0.01,0.0));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, groundT.ID);


		ground.draw(illuminated);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
		//skyBoxShader.setInteger("cubemapTexture", 0); // no use ???

		skyBoxShader.use();
		skyBoxShader.setMatrix4("V", view);
		skyBoxShader.setMatrix4("P", perspective);
		//skyBoxShader.setInteger("cubemapTexture", 0); // no use ???

		skyBox.draw(skyBoxShader);
		glDepthFunc(GL_LESS);

		// PostProcessing
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // rebind normal screen buffer to show result after operations
		if (POSTPAINT) {
			postPaint.use();
		}
		else {
			framebufferProgram.use();
		}
		glBindVertexArray(rectVAO);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, framebufferTexture);

		if (POSTPAINT) {
			postPaint.setVector2f("screenSize", WIDTH, HEIGHT);
		}
		else {
			framebufferProgram.setVector2f("screenSize", WIDTH, HEIGHT);
		}
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

bool T_LOCK = false;
bool Y_LOCK = false;



void processInput(GLFWwindow* window, double delta) {

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !T_LOCK) {
		T_LOCK = true;
		CAMLOCK = !CAMLOCK;
	}
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE && T_LOCK) {
		T_LOCK = false;
	}

	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS && !Y_LOCK) {
		Y_LOCK = true;
		POSTPAINT = !POSTPAINT;
	}
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_RELEASE && Y_LOCK) {
		Y_LOCK = false;
	}

	if (CAMLOCK) {
		// use the controls to interact with the game
		float angle = 0.0;
		float forward = 0;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			angle += 2 * delta;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			angle -= 2 * delta;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			forward += 3 * delta;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			forward -= 3 * delta;
		
		player.model = glm::rotate(player.model, glm::radians(angle), glm::vec3(0.0, 1.0, 0.0));
		player.model = glm::translate(player.model, glm::vec3(0.0, 0.0, forward * delta));

		
		PLAYERROTATION -= angle;
		camera.Yaw = PLAYERROTATION;
		camera.Pitch = -15.0f;
		player.model = glm::translate(player.model, glm::vec3(0.0, 5.0, -7.0)); // TODO rework shitty code

		camera.Position = player.getLocation();
		player.model = glm::translate(player.model, glm::vec3(0.0, -5.0, 7.0));

		camera.updateCameraVectors();

	
	}
	else {
		// Use the cameras class to change the parameters of the camera

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboardMovement(LEFT, 0.1 * delta);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboardMovement(RIGHT, 0.1 * delta);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboardMovement(FORWARD, 0.1 * delta);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboardMovement(BACKWARD, 0.1 * delta);

		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			camera.ProcessKeyboardRotation(1, 0.0, 1 * delta);
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			camera.ProcessKeyboardRotation(-1, 0.0, 1 * delta);

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			camera.ProcessKeyboardRotation(0.0, 1.0, 1 * delta);
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			camera.ProcessKeyboardRotation(0.0, -1.0, 1 * delta);
	}
}


