#ifndef OBJECT_H
#define OBJECT_H

#include<iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include "stb_image.h"


class Texture {
public:
	GLuint ID;

	Texture(const char* path, bool flip = true) {

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
		unsigned char* data = stbi_load(PATH_TO_TEXTURE + path, &width, &height, &nrchannels, STBI_rgb);

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
	}

};

#endif