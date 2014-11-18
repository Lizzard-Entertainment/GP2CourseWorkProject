#ifndef Shader_h
#define Shader_h

#include <GL/glew.h>
#include <SDL_opengl.h>

#include <iostream>
#include <fstream>
#include <string>

enum SHADER_TYPE
{
	VERTEX_SHADER=GL_VERTEX_SHADER,
	FRAGMENT_SHADER=GL_FRAGMENT_SHADER
};

GLuint loadShaderFromFile(std::string& filename, SHADER_TYPE shaderType);
GLuint loadShaderFromMemory(const char * pMem, SHADER_TYPE shaderType);

bool checkForLinkErrors(GLuint program);
bool checkForCompilerErrors(GLuint shaderProgram);

#endif
