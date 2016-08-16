// shadertype=glsl
#version 330 core

in vec2 textureCoords;
in vec3 customColor;
out vec4 color;

uniform sampler2D ourTexture;
uniform sampler2D ourTexture1;

uniform float opacity;

void main()
{
	color = mix(texture(ourTexture, textureCoords),texture(ourTexture1, textureCoords), 0.5f) * vec4(customColor, 1.0f);
}