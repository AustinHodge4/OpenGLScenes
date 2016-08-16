// shadertype=glsl
#version 330 core
out vec4 color;
uniform vec3 customColor;

void main()
{
    color = vec4(customColor, 1.0f);
}