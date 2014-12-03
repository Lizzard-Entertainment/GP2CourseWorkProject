#include "Material.h"
#include "Shader.h"
#include "Texture.h"
#include "Vertex.h"

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


	GLint vertexPosLocation = glGetAttribLocation(m_ShaderProgram, "vertexPosition");
	GLint vertexNormalLocation = glGetAttribLocation(m_ShaderProgram, "vertexNormals");
	GLint vertexTexLocation = glGetAttribLocation(m_ShaderProgram, "vertexTexCoords");
	GLint vertexColourLocation = glGetAttribLocation(m_ShaderProgram, "vertexColour");
	GLint vertexTangentLocation = glGetAttribLocation(m_ShaderProgram, "vertexTangents");
	GLint vertexBinormalLocation = glGetAttribLocation(m_ShaderProgram, "vertexBinormals");

	glBindAttribLocation(m_ShaderProgram, vertexPosLocation, "vertexPosition");
	glBindAttribLocation(m_ShaderProgram, vertexNormalLocation, "vertexNormals");
	glBindAttribLocation(m_ShaderProgram, vertexTexLocation, "vertexTexCoords");
	glBindAttribLocation(m_ShaderProgram, vertexColourLocation, "vertexColour");
	glBindAttribLocation(m_ShaderProgram, vertexTangentLocation, "vertexTangents");
	glBindAttribLocation(m_ShaderProgram, 5, "vertexBinormals");

	//Tell the shader that 0 is the position element
	glEnableVertexAttribArray(vertexPosLocation);
	glVertexAttribPointer(vertexPosLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
	glEnableVertexAttribArray(vertexNormalLocation);
	glVertexAttribPointer(vertexNormalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void**)sizeof(vec3));
	glEnableVertexAttribArray(vertexTexLocation);
	glVertexAttribPointer(vertexTexLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void**)(sizeof(vec3) + sizeof(vec3)));
	glEnableVertexAttribArray(vertexColourLocation);
	glVertexAttribPointer(vertexColourLocation, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void**)(sizeof(vec3) + sizeof(vec3) + sizeof(vec2)));
	glEnableVertexAttribArray(vertexTangentLocation);
	glVertexAttribPointer(vertexTangentLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void**)(sizeof(vec3) + sizeof(vec3) + sizeof(vec2) + sizeof(vec4)));
	glEnableVertexAttribArray(vertexBinormalLocation);
	glVertexAttribPointer(vertexBinormalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void**)(sizeof(vec3) + sizeof(vec3) + sizeof(vec2) + sizeof(vec4) + sizeof(vec3)));
}

bool Material::loadShader(const std::string& vsFilename, const std::string& fsFilename)
{
	GLuint vertexShaderProgram = 0;
	vertexShaderProgram = loadShaderFromFile(const_cast<std::string&>(vsFilename), VERTEX_SHADER);

	GLuint fragmentShaderProgram = 0;
	fragmentShaderProgram = loadShaderFromFile(const_cast<std::string&>(fsFilename), FRAGMENT_SHADER);

	m_ShaderProgram = glCreateProgram();
	glAttachShader(m_ShaderProgram, vertexShaderProgram);
	glAttachShader(m_ShaderProgram, fragmentShaderProgram);
	glLinkProgram(m_ShaderProgram);
	checkForLinkErrors(m_ShaderProgram);

	//now we can delete the VS & FS Programs
	glDeleteShader(vertexShaderProgram);
	glDeleteShader(fragmentShaderProgram);

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
	m_DiffuseMap = loadTextureFromFile(filename);
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