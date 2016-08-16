#include "GL\glew.h";
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
	Shader();
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath);
	void Use();
public:
	GLuint Program;
};