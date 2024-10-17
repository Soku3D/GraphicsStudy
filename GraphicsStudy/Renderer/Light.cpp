#include "Light.h"
#include <iostream>
#include "Utility.h"

namespace Core {

	Light::Light() :
		mPosition(DirectX::SimpleMath::Vector3(0, 0, -1)),
		mForwardDirection(DirectX::SimpleMath::Vector3(0, 0, 1)),
		mUpDirection(DirectX::SimpleMath::Vector3(0.f, 1.f, 0.f)),
		mRightDirection(DirectX::SimpleMath::Vector3(1.f, 0.f, 0.f)),
		m_farZ(1000.f),
		m_nearZ(0.1f)
	{
		using DirectX::SimpleMath::Vector3;

		mForwardDirection.Normalize();

		m_fov = DirectX::XMConvertToRadians(70.f);
		m_delTheta = DirectX::XMConvertToRadians(0.2f);
		m_delSine = sin(m_delTheta / 2.f);
		m_delCosine = cos(m_delTheta / 2.f);

		mStandardDirection = Vector3(0, 0, 1);

		d = 1.f / tan(m_fov / 2.f);
	}

	void Light::Initialize(const UINT& width, const UINT& height)
	{
		mDepthWidth = width;
		mDepthHeight = height;

		
	}
	

	void Light::SetPositionAndDirection(const DirectX::SimpleMath::Vector3& position,
		const DirectX::SimpleMath::Vector3& direction)
	{
		using DirectX::SimpleMath::Vector3;

		mPosition = position;
		mForwardDirection = direction;

		Vector3 v0 = Vector3(0, mForwardDirection.y, mForwardDirection.z);
		Vector3 v1 = Vector3(mForwardDirection.x, 0, mForwardDirection.z);
		v0.Normalize();
		v1.Normalize();
		float cosTheta1 = abs(v0.Dot(Vector3(0, 0, 1)));
		float cosTheta2 = v1.Dot(Vector3(0, 0, 1));
		m_xTheta = acos(cosTheta1);
		m_yTheta = acos(cosTheta2);
		if (v0.y > 0) {
			m_xTheta *= -1.f;
		}
		if (v1.x < 0) {
			m_yTheta *= -1.f;
		}
		m_fov = DirectX::XMConvertToRadians(70.f);
		m_delTheta = DirectX::XMConvertToRadians(0.2f);
		m_delSine = sin(m_delTheta / 2.f);
		m_delCosine = cos(m_delTheta / 2.f);

		mForwardDirection = Vector3::Transform(mStandardDirection, DirectX::XMMatrixRotationX(m_xTheta));
		mForwardDirection = Vector3::Transform(mForwardDirection, DirectX::XMMatrixRotationY(m_yTheta));
		mForwardDirection.Normalize();
	}
	void Light::SetAspectRatio(float aspectRatio)
	{
		m_aspectRatio = aspectRatio;
	}

	void Light::SetQuaternion(int deltaX, int deltaY)
	{
		DirectX::SimpleMath::Vector3 n((float)deltaY * m_aspectRatio, (float)deltaX, 0.f);
		m_quaternion = DirectX::SimpleMath::Quaternion(n * m_delSine, m_delCosine);
	}

	void Light::SetSpeed(float velocity)
	{
		mVelocity = velocity;
	}

	void Light::SetRotation(int deltaX, int deltaY) {
		m_xTheta += deltaY * m_aspectRatio * m_delTheta;
		m_yTheta += (float)deltaX * m_delTheta;

		if (m_xTheta >= DirectX::XM_PIDIV2 - 0.001f) {
			m_xTheta = DirectX::XM_PIDIV2 - 0.001f;
		}
		if (m_xTheta <= -DirectX::XM_PIDIV2 + 0.001f) {
			m_xTheta = -DirectX::XM_PIDIV2 + 0.001f;
		}
	}

	void Light::SetFov(const float& degree)
	{
		m_fov = DirectX::XMConvertToRadians(degree);
	}
	DirectX::SimpleMath::Matrix Light::GetViewMatrix() const
	{
		return DirectX::XMMatrixLookToLH(mPosition, mForwardDirection, mUpDirection);
	}

	DirectX::SimpleMath::Matrix Light::GetProjMatrix() const
	{
		return DirectX::XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearZ, m_farZ);
	}
	DirectX::SimpleMath::Vector3 Light::GetPosition() const
	{
		return mPosition;
	}
	DirectX::SimpleMath::Vector3 Light::GetForwardDirection() const
	{
		return mForwardDirection;
	}
	DirectX::SimpleMath::Vector3 Light::GetUpDirection() const
	{
		return mUpDirection;
	}
	DirectX::SimpleMath::Vector3 Light::GetRightDirection() const
	{
		return mRightDirection;
	}
}
