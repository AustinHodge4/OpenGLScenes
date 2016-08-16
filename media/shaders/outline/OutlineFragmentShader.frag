// shadertype=glsl
#version 330 core

out vec4 color;
in vec2 TextureCoords;

uniform sampler2D customTexture;

void main()
{
	color = vec4(1.0f, 0.0, 0.0, 1.0f);
}