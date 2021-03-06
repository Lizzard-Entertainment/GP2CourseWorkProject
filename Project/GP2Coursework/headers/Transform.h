#ifndef Transform_h
#define Transform_h

//Headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Our headers
#include "Component.h"

//Using statements
using glm::mat4;
using glm::vec3;
using glm::vec4;


class Transform:public Component
{
public:
    Transform();
    ~Transform();
    
    void update();
    
    void setPosition(float x,float y,float z);
	void setPosition(vec3 xyz);

    void setRotation(float x,float y,float z);
	void setRotation(vec3 xyz);

    void setScale(float x,float y,float z);
	void setScale(vec3 xyz);
    
	void rotateAroundPoint(float rotation, vec3 axis);
	void zoom(float zoomSpeed, vec3 focus);
	void Transform::reset(vec3 focus);
	//void forwardT(float zoomSpeed, vec3 focus);

    vec3& getPosition();
    vec3& getScale();
    vec3& getRotation();
    mat4& getModel();

private:
    vec3 m_Position;
    vec3 m_Scale;
    vec3 m_Rotation;
    mat4 m_Model;

	void DEBUGShowCoords();

protected:
};


#endif
