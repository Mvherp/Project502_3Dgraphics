#version 400 core
		out vec4 FragColor;
		

		in vec3 v_frag_coord; 
		in vec2 t_coords; 
		in vec3 v_normal; 

		uniform vec3 u_view_pos; 

		uniform sampler2D T;

		uniform samplerCube cubemapSampler; 


		void main() { 
		vec3 N = normalize(v_normal);
		vec3 V = normalize(u_view_pos - v_frag_coord); 
		vec3 R = reflect(-V,N); 
		vec3 tex1 = texture(cubemapSampler,R).rgb;
		vec3 tex2 = texture(T, t_coords).rgb;
		vec3 tex3 = max(tex1 * 0.75, tex2) ; // hard coding reflectiveness
		FragColor =  vec4(tex3, 1.0); 
		} ;