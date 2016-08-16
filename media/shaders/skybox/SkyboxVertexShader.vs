// shadertype=glsl
#version 330 core
layout (location = 0) in vec3 position;

out vec3 TextureCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 pos = projection * view * vec4(position, 1.0f);
	// set the pos z (depth) to 1 so depth buffer can discard parts of the skybox if objects are infront
	gl_Position = pos.xyww;
	TextureCoords = position;
	
}