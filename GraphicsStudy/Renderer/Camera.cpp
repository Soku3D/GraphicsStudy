#include "Camera.h"

Camera::Camera():
	m_position(DirectX::SimpleMath::Vector3(0.f,0.f,-1.f)),
	m_forwardDirection(DirectX::SimpleMath::Vector3(0.f, 0.f, 1.f)),
	m_upDirection(DirectX::SimpleMath::Vector3(0.f, 1.f, 0.f)),
	m_rightDirection(DirectX::SimpleMath::Vector3(1.f, 0.f, 0.f)),
	m_farZ(0.01f),
	m_nearZ(100.f)
{
	m_fov = DirectX::XMConvertToRadians(70.f);
}

void Camera::SetAspectRation(float aspectRatio)
{
	m_aspectRatio = aspectRatio;
}

void Camera::MoveUp(float deltaTime)
{
	m_position += (velocity * deltaTime) * m_upDirection;
}

void Camera::MoveDown(float deltaTime)
{
	m_position += (velocity * deltaTime) * -m_upDirection;
}

void Camera::MoveRight(float deltaTime)
{
	m_position += (velocity * deltaTime) * m_rightDirection;
}

void Camera::MoveLeft(float deltaTime)
{
	m_position += (velocity * deltaTime) * -m_rightDirection;
}

void Camera::MoveForward(float deltaTime)
{
	m_position += (velocity * deltaTime) * m_forwardDirection;
}

void Camera::MoveBackward(float deltaTime)
{
	m_position += (velocity * deltaTime) * -m_forwardDirection;
}

DirectX::SimpleMath::Matrix Camera::GetViewMatrix() const
{
	return DirectX::XMMatrixLookToLH(m_position, m_forwardDirection, m_upDirection);
}

DirectX::SimpleMath::Matrix Camera::GetProjMatrix() const
{
	return DirectX::XMMatrixPerspectiveFovLH(m_fov,m_aspectRatio,m_nearZ,m_farZ);
}