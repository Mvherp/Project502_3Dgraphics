#version 330 core
		out vec4 FragColors; 
		in vec2 TexCoords;

		uniform sampler2D screenTexture;

		const float offset_x = 1.0f / 500.0f;
		const float offset_y = 1.0f / 500.0f;

		const int SECTOR_COUNT = 4;
		vec2 offsets[4] = vec2[]( vec2(-offset_x, -offset_y), vec2(offset_x, -offset_y), vec2(offset_x, offset_y), vec2(-offset_x, offset_y) );
		const int kernelSize = 6;

		vec3 computeKernels(){
			vec3 bestAvg = vec3(0.0);
			float bestVar = 1.0;
			for(int k = 0; k < SECTOR_COUNT; k++){
				vec3 colorSum = vec3(0.0);
				vec3 colorSqSum = vec3(0.0);
				for(int i = 1; i <= kernelSize; i++){
					for(int j = 1; j <= kernelSize; j++){
						vec3 pixel = vec3(texture(screenTexture, TexCoords + vec2(i, j) * offsets[k]));
						colorSum += pixel;
						colorSqSum += pixel * pixel;
					};
				};
				vec3 avg = colorSum / (kernelSize * kernelSize);
				float var = dot((colorSqSum / (kernelSize * kernelSize) - avg * avg), vec3(0.299, 0.587, 0.114));
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
