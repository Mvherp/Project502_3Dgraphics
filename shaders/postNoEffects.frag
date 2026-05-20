#version 330 core
		
out vec4 FragColors; 
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main(){
	FragColors = texture(screenTexture, TexCoords);
};
