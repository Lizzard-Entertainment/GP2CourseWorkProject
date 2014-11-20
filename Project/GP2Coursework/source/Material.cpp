#include "Material.h"
#include "Shader.h"
#include "Texture.h"

Material::Material()
{
	m_ShaderProgram = -1;
	m_Type = "Material";
	m_AmbientColour = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	m_DiffuseColour = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	m_SpecularColour = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	m_SpecularPower = 2.0f;
	m_DiffuseMap = 0;
	m_SpecularMap = 0;
	m_HeightMap = 0;
}

Material::~Material()
{
    
}

void Material::destroy()
{
    glDeleteProgram(m_ShaderProgram);
	glDeleteTextures(1, &m_DiffuseMap);
	glDeleteTextures(1, &m_SpecularMap);
	glDeleteTextures(1, &m_BumpMap);
	glDeleteTextures(2, &m_HeightMap);
}

void Material::bind()
{
    glUseProgram(m_ShaderProgram);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_DiffuseMap);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_SpecularMap);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_BumpMap);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_HeightMap);
}

bool Material::loadShader(const std::string& vsFilename,const std::string& fsFilename)
{
    GLuint vertexShaderProgram=0;
	vertexShaderProgram = loadShaderFromFile(const_cast<std::string&>(vsFilename), VERTEX_SHADER);
    
    GLuint fragmentShaderProgram=0;
	fragmentShaderProgram = loadShaderFromFile(const_cast<std::string&>(fsFilename), FRAGMENT_SHADER);
    
    m_ShaderProgram = glCreateProgram();
	glAttachShader(m_ShaderProgram, vertexShaderProgram);
	glAttachShader(m_ShaderProgram, fragmentShaderProgram);
	glLinkProgram(m_ShaderProgram);
	checkForLinkErrors(m_ShaderProgram);
    
	//now we can delete the VS & FS Programs
	glDeleteShader(vertexShaderProgram);
	glDeleteShader(fragmentShaderProgram);
    
    glBindAttribLocation(m_ShaderProgram, 0, "vertexPosition");
	glBindAttribLocation(m_ShaderProgram, 1, "vertexNormals");
	glBindAttribLocation(m_ShaderProgram, 2, "vertexTexCoords");
	glBindAttribLocation(m_ShaderProgram, 3, "vertexColour");
	glBindAttribLocation(m_ShaderProgram, 4, "vertexTangents");
	glBindAttribLocation(m_ShaderProgram, 5, "vertexBinormals");

    return true;
}

GLint Material::getUniformLocation(const std::string& name)
{
    return glGetUniformLocation(m_ShaderProgram, name.c_str());
}

vec4& Material::getAmbientColour()
{
	return m_AmbientColour;
}

void Material::setAmbientColour(float r, float g, float b, float a)
{
	m_AmbientColour = vec4(r, g, b, a);
}

vec4& Material::getDiffuseColour()
{
	return m_DiffuseColour;
}

void Material::setDiffuseColour(float r, float g, float b, float a)
{
	m_DiffuseColour = vec4(r, g, b, a);
}

vec4& Material::getSpecularColour()
{
	return m_SpecularColour;
}

void Material::setSpecularColour(float r, float g, float b, float a)
{
	m_SpecularColour = vec4(r, g, b, a);
}

float Material::getSpecularPower()
{
	return m_SpecularPower;
}

void Material::setSpecularPower(float power)
{
	m_SpecularPower = power;
}

void Material::loadDiffuseMap(const std::string& filename)
{
	m_DiffuseMap=loadTextureFromFile(filename);
}

GLuint Material::getDiffuseMap()
{
	return m_DiffuseMap;
}

void Material::loadSpecularMap(const std::string& filename)
{
	m_SpecularMap = loadTextureFromFile(filename);
}

GLuint Material::getSpecularMap()
{
	return m_SpecularMap;
}


void Material::loadBumpMap(const std::string& filename)
{
	m_BumpMap = loadTextureFromFile(filename);
}

GLuint Material::getBumpMap()
{
	return m_BumpMap;
}

void Material::loadHeightMap(const std::string& filename)
{
	m_HeightMap = loadTextureFromFile(filename);
}


GLuint Material::getHeightMap()
{
	return m_HeightMap;
}