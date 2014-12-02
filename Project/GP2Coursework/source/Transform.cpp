#include <glm/glm.hpp>
#include <GL\glew.h>
#include <iostream>
#include "Transform.h"
#include "GameObject.h"

Transform::Transform()
{
    m_Position=vec3(0.0f,0.0f,0.0f);
    m_Rotation=vec3(0.0f,0.0f,0.0f);
    m_Scale=vec3(1.0f,1.0f,1.0f);
    m_Model=mat4();
}

Transform::~Transform()
{
    
}

// make public the transform and rotation matrix and use it in update.  none of this ap nonsense.
void Transform::update()
{
	//Initialise matrices.
	mat4 translate = mat4(1.0f);
	mat4 scale = mat4(1.0f);
	mat4 rotation = mat4(1.0f);

	//Set up translation matrix.
	translate = glm::translate(translate, m_Position);
	
	//Scale matrix? pfft.
	scale = glm::scale(scale, m_Scale);
    
	/*
	Improve efficiancy by feeding in the previous matrix to the current.  Since they're all going to be multiplied in the end, this saves time.
	https://www.youtube.com/watch?v=U_RtSchYYec	

	I also merged all matrices into one massive statement, rather than 3 seperate rotation matrices then a multiplication.
	*/
	rotation = glm::rotate(glm::rotate(glm::rotate(mat4(1.0f),
		m_Rotation.x, vec3(1.0f, 0.0f, 0.0f)),
		m_Rotation.y, vec3(0.0f, 1.0f, 0.0f)),
		m_Rotation.z,vec3(0.0f, 0.0f, 1.0f));

	//Calculate model view matrix.
	m_Model = translate * rotation * scale;

	//Move this object in respect to the parent if there is a parent object.
	if (m_Parent->getParent())
	{
		m_Model = m_Parent->getParent()->getTransform()->getModel()*m_Model;
	}
}

void Transform::setPosition(float x,float y,float z)
{
    m_Position=vec3(x,y,z);
}

void Transform::setRotation(float x,float y,float z)
{
    m_Rotation=vec3(x,y,z);
}

void Transform::setScale(float x,float y,float z)
{
    m_Scale=vec3(x,y,z);
}

vec3& Transform::getPosition()
{
    return m_Position;
}

vec3& Transform::getScale()
{
    return m_Scale;
}

vec3& Transform::getRotation()
{
    return m_Rotation;
}

mat4& Transform::getModel()
{
    return m_Model;
}

void Transform::rotateAroundPoint(float rotation, vec3 axis, vec3 focus)
{
	//Initialise matrices
	mat4 TranslationToOrigin(1.0f);   //Matrix for the translation to the origin.  T1
	mat4 RotationAroundOrigin(1.0f);  //Rotation matrix for rotation around the origin. R o T1
	mat4 CompositeMatrix(1.0f);       //Composite matrix for reverse translation after rotation. T2 o R o T1

	//Move to the origin, rotate, move back.
	TranslationToOrigin = glm::translate(mat4(1.0f), -focus);
	RotationAroundOrigin = glm::rotate(TranslationToOrigin, rotation, axis);
	CompositeMatrix = glm::translate(RotationAroundOrigin, focus);

	/*
	m_Position is fed into a vector4 so that it may conform with the matrix4 composite matrix and output a matrix4.
	A matrix 4 and a vector 3 cannot be multiplied to give a vector 3, which the position requires.
	*/
	m_Position = vec3(CompositeMatrix * vec4(m_Position, 1.0f));

	DEBUGShowCoords();
}

void Transform::zoom(float zoomSpeed)
{
	mat4 TranslationToOrigin(1.0f);
	mat4 TranslationBack(1.0f);

	TranslationToOrigin = glm::translate(mat4(1.0f), -vec3(0.0f, 0.0f, 0.0f));
	TranslationBack = glm::translate(TranslationToOrigin, (vec3(0.0f, 0.0f, 0.0f) + zoomSpeed));
	m_Position = vec3(TranslationBack * vec4(m_Position, 1.0f));

	DEBUGShowCoords();
}

void Transform::DEBUGShowCoords()
{
	//Prints the XYZ of the camera out to the console.
	std::cout << "Coords after movement: " << std::to_string(m_Position.x) << " , " << std::to_string((m_Position.y)) << " , " << std::to_string((m_Position.z)) << std::endl << std::endl;
}