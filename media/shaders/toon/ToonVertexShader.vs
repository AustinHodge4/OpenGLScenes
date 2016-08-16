// shadertype=glsl
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 texCoords;

out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal; // In view space

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f);
	Normal = normal * normals;
	FragPos = vec3(view * model* vec4(position, 1.0f)); // Convert to view space
}