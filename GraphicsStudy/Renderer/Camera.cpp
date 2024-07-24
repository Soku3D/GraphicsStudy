#include "Camera.h"

Camera::Camera():
	m_position(DirectX::SimpleMath::Vector3(0.f,0.f,-1.f)),
	m_direction(DirectX::SimpleMath::Vector3(0.f, 0.f, 1.f)),
	m_farZ(0.01f),
	m_nearZ(100.f)
{

}

void Camera::SetAspectRation(float aspectRatio)
{
	m_aspectRatio = aspectRatio;
}

