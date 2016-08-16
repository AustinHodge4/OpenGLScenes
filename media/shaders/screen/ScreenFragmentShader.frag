// shadertype=glsl
#version 330 core

in vec2 TextureCoords;
out vec4 color;

uniform sampler2D customTexture;

const float offset = 1.0f/300.0f;

void main()
{
	vec2 offsets[8] = {
		vec2(-offset, offset), // top-left
		vec2(0.0f, offset), // top-middle
		vec2(offset, offset), // top-right
		vec2(-offset, 0.0f), // left-middle
		vec2(offset, 0.0f), // right-middle
		vec2(-offset, -offset), // bottom-left
		vec2(0.0f, -offset), // bottom-middle
		vec2(offset, -offset) // bottom-right
	};
	float kernel[9] = {
		1.0f,  1.0f,  1.0f,
		1.0f,  -8.0f, 1.0f,
		1.0f,  1.0f,  1.0f
	};
	float blurKernel[9] = {
		1.0/16,  2.0/16,  1.0/16,
		2.0/16,  4.0/16, 2.0/16,
		1.0/16,  2.0/16,  1.0/16
	};
	vec3 samples[9];
	for(int i = 0; i < 9; i++)
		samples[i] = vec3(texture(customTexture, TextureCoords.st + offsets[i]));

	vec3 col = vec3(0.0f);
	for(int i = 0; i < 9; i++)
		col += samples[i] * blurKernel[i];

	color = vec4(col, 1.0f);
}