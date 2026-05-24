#version 400 core
		out vec4 FragColor;
		precision mediump float; 

		in vec3 v_frag_coord; 
		in vec3 v_normal; 

		uniform vec3 u_view_pos; 

		vec3 bubble_color = vec3(0.4,0.9,1.0);


		uniform samplerCube cubemapSampler; 
		uniform float refractionIndice;

		void main() { 
		float ratio = 1.00 / refractionIndice;
		vec3 N = normalize(v_normal);
		vec3 V = normalize(u_view_pos - v_frag_coord); 
		vec3 R = refract(-V,N,ratio); 
		FragColor = vec4(texture(cubemapSampler,R).rgb * 0.4 + bubble_color  * 0.6, 1.0); 
		} ;