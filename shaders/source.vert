#version 330 core
		in vec3 position; 
		in vec2 tex_coord; 
		in vec3 normal; 

		out vec3 v_diffuse; 
		out vec2 tex_c;

		uniform mat4 M; 
		uniform mat4 itM; 
		uniform mat4 V; 
		uniform mat4 P; 

		//Think about which uniform you may need
		uniform vec3 light; 
		 void main(){ 
		vec4 frag_coord = M*vec4(position, 1.0); 
		gl_Position = P*V*frag_coord;
		//3. transform correctly the normals
		vec3 norm = vec3(itM * vec4(normal, 1.0)); 
		//3. use Gouraud : compute the diffuse light with the normals at the vertices
		vec3 L = normalize(light - frag_coord.xyz);
		float diffuse = max(dot(norm, L), 0.0); 
		v_diffuse = vec3(diffuse);  //put the result of your calculation here
		text_c = tex_coord;
		};