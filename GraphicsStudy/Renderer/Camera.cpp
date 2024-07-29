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
	m_delTheta = DirectX::XMConvertToRadians(0.2f);
	m_delSine = sin(m_delTheta / 2.f);
	m_delCosine = cos(m_delTheta / 2.f);
}

void Camera::SetAspectRation(float aspectRatio)
{
	m_aspectRatio = aspectRatio;
}


void Camera::SetQuaternion(int deltaX, int deltaY)
{
	DirectX::SimpleMath::Vector3 n((float)deltaY*m_aspectRatio, (float)deltaX, 0.f);
	m_quaternion = DirectX::SimpleMath::Quaternion(n * m_delSine, m_delCosine);
}

void Camera::RotateDirection() {
	using DirectX::SimpleMath::Vector3;
	//if (m_forwardDirection.Dot(Vector3(0.f, 1.f, 0.f)) < 0.99f && m_forwardDirection.Dot(Vector3(0.f, -1.f, 0.f)) > -0.99f) {
		m_forwardDirection = Vector3::Transform(m_forwardDirection, DirectX::XMMatrixRotationQuaternion(m_quaternion));
		m_forwardDirection.Normalize();
		m_quaternion = DirectX::SimpleMath::Quaternion();
		m_rightDirection = m_upDirection.Cross(m_forwardDirection);
	//}
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