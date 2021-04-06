#ifndef _MAIN_HXX_
#define _MAIN_HXX_

#include <stdio.h>
#include <stdlib.h>

#include <ios>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
using namespace glm;

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glfw3.lib")

//http://www.opengl-tutorial.org/beginners-tutorials/tutorial-1-opening-a-window/

static const GLfloat g_triangle[] =
{
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f
};

static inline GLuint LoadShaders(const char* VertexProgram, const char* FragmentProgram)
{
	GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	string VertexShaderCode;
	ifstream VertexShaderStream(VertexProgram, ios::in);
	if (VertexShaderStream.is_open())
	{
		std::stringstream ss;
		ss << VertexShaderStream.rdbuf();
		VertexShaderCode = ss.str();
		VertexShaderStream.close();
	}
	else
	{
		printf("[INFO]: %s\n", "Failed to open Vertex Program");
		return 0;
	}

	string FragmentShaderCode;
	ifstream FragmentShaderStream(FragmentProgram, ios::in);
	if (FragmentShaderStream.is_open())
	{
		std::stringstream ss;
		ss << FragmentShaderStream.rdbuf();
		FragmentShaderCode = ss.str();
		FragmentShaderStream.close();
	}
	else
	{
		printf("[WARN]: %s\n", "Failed to open Fragment Program");
		return 0;
	}

	GLint result = GL_FALSE;
	GLint infoLogLength = 0;

	printf("[INFO]: %s\n", "Compiling Vertex Shader...");
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShader, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShader);
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(VertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0)
	{
		vector<char> ErrorMessage((size_t)infoLogLength + 1);
		glGetShaderInfoLog(VertexShader, infoLogLength, NULL, &ErrorMessage[0]);
		printf("%s\n", &ErrorMessage[0]);
	}

	printf("[INFO]: %s\n", "Compiling Fragment Shader...");
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShader, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShader);
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(FragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0)
	{
		vector<char> ErrorMessage((size_t)infoLogLength + 1);
		glGetShaderInfoLog(FragmentShader, infoLogLength, NULL, &ErrorMessage[0]);
		printf("%s\n", &ErrorMessage[0]);
	}

	printf("[INFO]: %s\n", "Linking Shader Program...");
	GLuint program = glCreateProgram();
	glAttachShader(program, VertexShader);
	glAttachShader(program, FragmentShader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0)
	{
		vector<char> ErrorMessage((size_t)infoLogLength + 1);
		glGetProgramInfoLog(program, infoLogLength, NULL, &ErrorMessage[0]);
		printf("%s\n", &ErrorMessage[0]);
	}

	printf("[INFO]: %s\n", "Done!");
	glDetachShader(program, VertexShader);
	glDetachShader(program, FragmentShader);
	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);
	return program;
}

#endif//_MAIN_HXX_