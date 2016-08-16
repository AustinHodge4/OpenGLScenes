// shadertype=glsl
#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TextureCoords;

out vec4 color;

uniform samplerCube cubemap;
uniform vec3 cameraPos;

void main()
{
	vec3 normal = normalize(Normal);
	float ratio = 1.0f/1.33f;
	vec3 direction = normalize(FragPos - cameraPos);
	vec3 refraction = refract(direction, normal, ratio);

	color = texture(cubemap, refraction);
}