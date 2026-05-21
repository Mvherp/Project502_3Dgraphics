#version 330 core
		out vec4 FragColors; 
		in vec2 TexCoords;

		uniform sampler2D screenTexture;
		uniform vec2 screenSize;

		const int SECTOR_COUNT = 8;
		const int kernelSize = 6;
		const int sampleSize = kernelSize * 5;



		vec3 computeKernels(){
			vec3 bestAvg = vec3(0.0);
			float bestVar = 1.0;
			for(int k = 0; k < SECTOR_COUNT; k++){ // number of slice of the circle (8 has been found optimal)
				vec3 colorSum = vec3(0.0);
				vec3 colorSqSum = vec3(0.0);
				float angle = k * 6.28318 / SECTOR_COUNT;
				for(int r = 1; r <= kernelSize; r++){ // radius
					for(float a = -0.392699; a <= 0.392699; a += 0.196349){ // size of a radians
					    vec2 sampleOffset = r * vec2(cos(angle + a), sin(angle + a));
						vec3 pixel = vec3(texture(screenTexture, TexCoords + sampleOffset / screenSize));
						colorSum += pixel;
						colorSqSum += pixel * pixel;
					};
				};
				vec3 avg = colorSum / (sampleSize);
				float var = dot((colorSqSum / (sampleSize) - avg * avg), vec3(0.299, 0.587, 0.114));
				if(	var < bestVar ) {
					bestAvg = avg;
					bestVar = var;
				};
			};
			return bestAvg;
		}
		

		void main(){
			FragColors = vec4(computeKernels(), 1.0);
		};
