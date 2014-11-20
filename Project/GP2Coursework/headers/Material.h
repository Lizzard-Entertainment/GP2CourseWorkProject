#ifndef Material_h
#define Material_h

//Headers
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL.h>
#include <SDL_opengl.h>
#include <gl/GLU.h>
#include <string>

//Our headers
#include "Component.h"

//Using statements
using glm::vec4;

class Material:public Component{
public:
    Material();
    ~Material();
    
    void destroy();
    
    void bind();
    
    bool loadShader(const std::string& vsFilename,const std::string& fsFilename);
    GLint getUniformLocation(const std::string& name);
	
	vec4& getAmbientColour();
	void setAmbientColour(float r, float g, float b, float a);

	vec4& getDiffuseColour();
	void setDiffuseColour(float r, float g, float b, float a);

	vec4& getSpecularColour();
	void setSpecularColour(float r, float g, float b, float a);

	float getSpecularPower();
	void setSpecularPower(float power);

	void loadDiffuseMap(const std::string& filename);
	GLuint getDiffuseMap();

	void loadSpecularMap(const std::string& filename);
	GLuint getSpecularMap();

	void loadBumpMap(const std::string& filename);
	GLuint getBumpMap();

	void loadHeightMap(const std::string& filename);
	GLuint getHeightMap();
protected:
private:
    GLuint m_ShaderProgram;
	vec4 m_AmbientColour;
	vec4 m_DiffuseColour;
	vec4 m_SpecularColour;
	float m_SpecularPower;
	GLuint m_DiffuseMap;
	GLuint m_SpecularMap;
	GLuint m_BumpMap;
	GLuint m_HeightMap;
};

#endif
