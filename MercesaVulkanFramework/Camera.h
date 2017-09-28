#pragma once


// This class is the base camera class
#include "glm\common.hpp"
#include "glm\gtx/color_space.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp" 

class Camera
{

public:
	Camera();
	~Camera();

	glm::mat4	GetViewMatrix();

	void		SetPerspective(float a_FovY, float a_AspectRatio, float a_Near, float a_Far);
	glm::mat4	GetProjectionMatrix();

	void		SetPosition(glm::vec3 a_Position);
	glm::vec3	GetPosition();
	void		SetRotation(glm::vec3 a_Rotation);
	glm::vec3	GetRotation();

	void		SetViewportSize(glm::vec2 a_Size);
	glm::vec2	GetViewPortSize();

	void		SetViewPortPosition(glm::vec2 a_Size);
	glm::vec2	GetViewPortPosition();
	
	glm::vec3	GetForwardVector();
	glm::vec3	GetRightVector();
	glm::vec3	GetUpVector();

	// position
	glm::vec3 m_Position;
	// rotation (in degrees)
	glm::vec3 m_Rotation;

private:
	// calculate view matrix
	void CalculateViewMatrix();

	// view matrix
	glm::mat4 m_ViewMatrix;
	// Projection matrix
	glm::mat4 m_ProjectionMatrix;



	// right/x vector
	glm::vec3 m_Right;
	// up/y vector
	glm::vec3 m_Up;
	// forward/z vector
	glm::vec3 m_Forward;

	glm::vec2 m_ViewPortSize;
	glm::vec2 m_ViewPortPosition;

	
};

