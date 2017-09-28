#include "Camera.h"


Camera::Camera() :
	m_Position(0.0f, 0.0f, 0.0f),
	m_ViewPortSize(1280.0f, 720.0f),
	m_ViewPortPosition(0.0f, 0.0f),
	m_Rotation(0.0f)
{
	CalculateViewMatrix();
}

Camera::~Camera()
{
}

glm::mat4 Camera::GetViewMatrix()
{
	return m_ViewMatrix;
}

void Camera::SetPerspective(float a_FovY, float a_AspectRatio, float a_Near, float a_Far)
{
	m_ProjectionMatrix = glm::perspective(a_FovY, a_AspectRatio, a_Near, a_Far);
}

glm::mat4 Camera::GetProjectionMatrix()
{
	return m_ProjectionMatrix;
}

void Camera::SetPosition(glm::vec3 a_Position)
{
	m_Position = a_Position;
	CalculateViewMatrix();
}

glm::vec3 Camera::GetPosition()
{
	return m_Position;
}

void Camera::SetRotation(glm::vec3 a_Rotation)
{
	m_Rotation = a_Rotation;
	CalculateViewMatrix();
}

glm::vec3 Camera::GetRotation()
{
	return m_Rotation;
}

void Camera::SetViewportSize(glm::vec2 a_Size)
{
	m_ViewPortSize = a_Size;
}

glm::vec2 Camera::GetViewPortSize()
{
	return m_ViewPortSize;
}

void Camera::SetViewPortPosition(glm::vec2 a_Position)
{
	m_ViewPortPosition = a_Position;
}

glm::vec2 Camera::GetViewPortPosition()
{
	return m_ViewPortPosition;
}


glm::vec3 Camera::GetForwardVector()
{
	return m_Forward;
}

glm::vec3 Camera::GetRightVector()
{
	return m_Right;
}

glm::vec3 Camera::GetUpVector()
{
	return m_Up;
}

#define PI 3.14159265f

void Camera::CalculateViewMatrix()
{
	// convert rotation to radians
	float pitch = m_Rotation.x * (PI / 180);
	float yaw = m_Rotation.y * (PI / 180);

	// get cos and sin values
	float cosPitch = cos(pitch);
	float sinPitch = sin(pitch);
	float cosYaw = cos(yaw);
	float sinYaw = sin(yaw);

	// get normalized direction vectors
	m_Right = { cosYaw, 0, -sinYaw };								// right
	m_Up = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };		// up
	m_Forward = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };// forward

	// set up viewmatrix
	m_ViewMatrix = glm::mat4{
		glm::vec4(m_Right.x, m_Up.x, m_Forward.x, 0),
		glm::vec4(m_Right.y, m_Up.y, m_Forward.y, 0),
		glm::vec4(m_Right.z, m_Up.z, m_Forward.z, 0),
		glm::vec4(-glm::dot(m_Right, m_Position), -glm::dot(m_Up, m_Position), -glm::dot(m_Forward, m_Position), 1)
	};

	m_Forward = -m_Forward;
}
