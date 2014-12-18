#include "PPFilterType.h"

//Returns shader depending on index.
mat4 getShader()
{
	switch (shaderIndex)
	{
		case 0: return NONE;
		case 1: return BLACK_AND_WHITE;
		case 2: return SEPIA;
		case 3: return INVERTED;
		case 4: return POLAROID;
		default: return NONE;
	}
}

//Matches the getShader method but with string representation.
std::string getShaderName()
{
	switch (shaderIndex)
	{
		case 0: return "NONE";
		case 1: return "BLACK_AND_WHITE";
		case 2: return "SEPIA";
		case 3: return "INVERTED";
		case 4: return "POLAROID";
		default: return "NONE";
	}
}

//Increments index if index is between 0 and 3 inclusive.
void nextShader()
{	
	if (shaderIndex < 4)
		shaderIndex++;

	else shaderIndex = 0;
}

//Returns shader index
int getShaderIndex()
{
	return shaderIndex;
}