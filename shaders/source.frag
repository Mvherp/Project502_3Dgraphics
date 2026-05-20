#version 330 core
		out vec4 FragColor;
		precision mediump float; 
		in vec3 v_diffuse; 
		in vec2 tex_c;



		void main() { 
		FragColor = vec4(v_diffuse, 1.0); 
		} ;