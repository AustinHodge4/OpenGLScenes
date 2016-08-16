// shadertype=glsl
#version 330 core

in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform vec3 lightDir;
uniform vec3 customColor;

void main()
{
	vec3 norm = normalize(Normal);
	vec3 lightDirection = normalize(-lightDir);
	vec3 viewDir = normalize(-FragPos); // The viewer is at (0,0,0) so viewDir is (0,0,0) - Position => -Position

	float ambient = 0.1f;
	float diffuse = max(dot(lightDirection, norm), 0.0f);
	vec3 reflectDir = reflect(-lightDirection, norm);
	float specular = pow(max(dot(viewDir, reflectDir),0.0f), 32.0f);
  
	float intensity = ambient + diffuse + specular;

	color = vec4(customColor * intensity, 1.0f);
	//float influence = dot(vec3(0.0f, 1.0f, 0.0f), norm) * 0.5f + 0.5f;
	//color = vec4(mix(vec3(0.0f, 0.4f, 0.0f), vec3(0.6f, 0.6f, 1.0f), influence), 1.0f);

}