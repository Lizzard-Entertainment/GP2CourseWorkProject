#include "Light.h"

Light::Light()
{
	m_DiffuseColour = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	m_SpecularColour = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	m_LightDirection = vec3(0.0f, 0.5f, 0.5f);
	m_Type = "Light";
}

Light::~Light()
{

}

vec4& Light::getSpecularColour()
{
	return m_SpecularColour;
}

void Light::setSpecularColour(float r, float g, float b, float a)
{
	m_SpecularColour = vec4(r, g, b, a);
}

vec4& Light::getDiffuseColour()
{
	return m_DiffuseColour;
}

void Light::setDiffuseColour(float r, float g, float b, float a)
{
	m_DiffuseColour = vec4(r, g, b, a);
}

vec3& Light::getDirection()
{
	return m_LightDirection;
}

void Light::setDirection(float x, float y, float z)
{
	m_LightDirection = vec3(x, y, z);
}
