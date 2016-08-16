// shadertype=glsl
#version 330 core

in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform vec3 lightDir;
uniform vec3 viewPos;
uniform vec3 customColor;

void main()
{
	vec3 norm = normalize(Normal);
	vec3 lightDirection = normalize(-lightDir);
	vec3 viewDir = normalize(viewPos - FragPos);

	float ambient = 0.1f;
	float diffuse = max(dot(lightDirection, norm), 0.0f);

	vec3 reflectDir = reflect(-lightDirection, norm);
	float specular = pow(max(dot(viewDir, reflectDir),0.0f), 32.0f);
  
	float lightIntensity = clamp(ambient + diffuse + specular, 0, 1);
	color = vec4(1.0);

	if (lightIntensity < 0.85) 
	{
	   // hatch from left top corner to right bottom
	   if (mod(gl_FragCoord.x + gl_FragCoord.y, 10.0) == 0.0) 
	   {
		  color = vec4(0.0, 0.0, 0.0, 1.0);
	   }
	}

	if (lightIntensity < 0.75) 
	{
	   // hatch from right top corner to left boottom
	   if (mod(gl_FragCoord.x - gl_FragCoord.y, 10.0) == 0.0) 
	   {
		  color = vec4(0.0, 0.0, 0.0, 1.0);
	   }
	}

	if (lightIntensity < 0.5) 
	{
	   // hatch from left top to right bottom
	   if (mod(gl_FragCoord.x + gl_FragCoord.y - 5.0, 10.0) == 0.0) 
	   {
		  color = vec4(0.0, 0.0, 0.0, 1.0);
	   }
	}

	if (lightIntensity < 0.25) 
	{
	   // hatch from right top corner to left bottom
	   if (mod(gl_FragCoord.x - gl_FragCoord.y - 5.0, 10.0) == 0.0) 
	   {
		  color = vec4(0.0, 0.0, 0.0, 1.0);
	   }
	}
}