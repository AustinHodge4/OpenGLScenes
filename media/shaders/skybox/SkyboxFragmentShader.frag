// shadertype=glsl
#version 330 core

in vec3 TextureCoords;
out vec4 color;

uniform samplerCube cubemap;

void main()
{
	color = texture(cubemap, TextureCoords);
}