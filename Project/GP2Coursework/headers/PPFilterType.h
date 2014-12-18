#pragma region Euan

#ifndef PPFilterType_H
#define PPFilterType_H

//Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <gl/GLU.h>
#include <string>
//Usings
using glm::mat4;

//Variables.  Everything's global because scope.

//No colour filter.
static const mat4 NONE = mat4(1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1);

//Black and white colour filter.
static const mat4 BLACK_AND_WHITE = mat4(-0.7, 2.0, -0.3, 0,
	-0.7, 2.0, -0.3, 0,
	-0.7, 2.0, -0.3, 0,
	0, 0, 0, 1);

//Sepia colour filter
static const mat4 SEPIA = mat4(0.393, 0.769, 0.189, 0,
	0.349, 0.686, 0.168, 0,
	0.272, 0.534, 0.131, 0,
	0, 0, 0, 1);

//Inverted colour filter
static const mat4 INVERTED = mat4(-1, 0, 0, 1,
	0, -1, 0, 1,
	0, 0, -1, 1,
	1, 1, 1, 1);  //Perhaps swap out the last 1 for a 0.  Background still draws black but rest of the scene "looks" inverted.

//Polaroid colour filter
static const mat4 POLAROID = mat4(1.438, -0.062, -0.062, 0,
	0.122, 1.378, -0.122, 0,
	-0.016, -0.016, 1.483, 0,
	-0.03, 0.05, -0.02, 1);

//Shader index.
static int shaderIndex = 0;

//Shader Methods
mat4 getShader();
std::string getShaderName();
void nextShader();
int getShaderIndex();

#endif

#pragma endregion