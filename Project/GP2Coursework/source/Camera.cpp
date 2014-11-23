#include "Camera.h"
#include "GameObject.h"
#include "Transform.h"

Camera::Camera()
{
	m_Type = "Camera";
	m_Up=vec3(0.0f,1.0f,0.0f);
	m_NearClip=0.1f;
	m_LookAt = vec3(0.0f, 0.0f, 0.0f);
	m_FarClip = 1000.0f;
	m_FOV=45.0f;
	m_View = mat4();
	m_Projection = mat4();
}

Camera::~Camera()
{

}

void Camera::update()
{
	//get the position from the transform
	vec3 position = m_Parent->getTransform()->getPosition();
    	
	//Set matrices
	m_Projection = glm::perspective(m_FOV, m_AspectRatio, m_NearClip, m_FarClip);

	if (m_Parent->getName() == "DebugCamera")
		m_View = glm::lookAt(position, position + vec3(0.0f, 0.0f, 1.0f), m_Up);
	else
		m_View = glm::lookAt(position, m_LookAt, m_Up);
}

void Camera::setLookAt(float x, float y, float z)
{
    m_LookAt = vec3(x, y, z);
}

void Camera::setDirection(vec3 direction)
{
	m_Direction = direction;
}

void Camera::setUp(float x, float y, float z)
{
    m_Up = vec3(x, y, z);
}

 void Camera::setNearClip(float near)
 {
     m_NearClip = near;
 }
 
 void Camera::setFarClip(float far)
 {
     m_FarClip = far;
 }

void Camera::setFOV(float FOV)
{
    m_FOV = FOV;
}

void Camera::setAspectRatio(float ratio)
{
    m_AspectRatio = ratio;
}

mat4& Camera::getView()
{
    return m_View;
}

mat4& Camera::getProjection()
{
    return m_Projection;
}