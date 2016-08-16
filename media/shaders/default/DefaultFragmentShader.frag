// shadertype=glsl
#version 330 core

in vec2 TextureCoords;
out vec4 color;

uniform sampler2D customTexture;

void main()
{
	color = texture(customTexture, TextureCoords);
}