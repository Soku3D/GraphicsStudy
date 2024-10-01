#include "Camera.h"
#include <iostream>

namespace Core {

	Camera::Camera() :
		mPosition(DirectX::SimpleMath::Vector3(0,0,-1)),
		mForwardDirection(DirectX::SimpleMath::Vector3(0,0,1)),
		mUpDirection(DirectX::SimpleMath::Vector3(0.f, 1.f, 0.f)),
		mRightDirection(DirectX::SimpleMath::Vector3(1.f, 0.f, 0.f)),
		m_farZ(1000.f),
		m_nearZ(0.1f),
		Actor()
	{
		using DirectX::SimpleMath::Vector3;

		mForwardDirection.Normalize();
	
		m_fov = DirectX::XMConvertToRadians(70.f);
		m_delTheta = DirectX::XMConvertToRadians(0.2f);
		m_delSine = sin(m_delTheta / 2.f);
		m_delCosine = cos(m_delTheta / 2.f);
	
		mStandardDirection = Vector3(0,0,1);
		
		d = 1.f/tan(m_fov/2.f);
	}

	void Camera::SetPositionAndDirection(const DirectX::SimpleMath::Vector3 & position,
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
	void Camera::SetAspectRatio(float aspectRatio)
	{
		m_aspectRatio = aspectRatio;
	}

	void Camera::SetQuaternion(int deltaX, int deltaY)
	{
		DirectX::SimpleMath::Vector3 n((float)deltaY * m_aspectRatio, (float)deltaX, 0.f);
		m_quaternion = DirectX::SimpleMath::Quaternion(n * m_delSine, m_delCosine);
	}

	void Camera::SetSpeed(float velocity)
	{
		mVelocity = velocity;
	}

	void Camera::SetRotation(int deltaX, int deltaY) {
		m_xTheta += deltaY * m_aspectRatio * m_delTheta;
		m_yTheta += (float)deltaX * m_delTheta;
		
		if (m_xTheta >= DirectX::XM_PIDIV2 - 0.001f) {
			m_xTheta = DirectX::XM_PIDIV2 - 0.001f;
		}
		if (m_xTheta <= -DirectX::XM_PIDIV2 + 0.001f) {
			m_xTheta = -DirectX::XM_PIDIV2 + 0.001f;
		}
	}

	void Camera::SetFov(const float& degree)
	{
		m_fov = DirectX::XMConvertToRadians(degree);
	}

	void Camera::RotateDirection() {
		using DirectX::SimpleMath::Vector3;
	/*	if (mViewDirection.Dot(Vector3(0.f, 1.f, 0.f)) < 0.99f && mViewDirection.Dot(Vector3(0.f, -1.f, 0.f)) > -0.99f) {
		mViewDirection = Vector3::Transform(mViewDirection, DirectX::XMMatrixRotationQuaternion(m_quaternion));
		mViewDirection.Normalize();
		m_quaternion = DirectX::SimpleMath::Quaternion();
		
		}*/
		mForwardDirection = Vector3::Transform(mStandardDirection, DirectX::XMMatrixRotationX(m_xTheta));
		mForwardDirection = Vector3::Transform(mForwardDirection, DirectX::XMMatrixRotationY(m_yTheta));
		mForwardDirection.Normalize();

		m_quaternion = DirectX::SimpleMath::Quaternion();
		mRightDirection = mUpDirection.Cross(mForwardDirection);
		mRightDirection.Normalize();
	}

	void Camera::MoveUp(float deltaTime)
	{
		mPosition += (mVelocity * deltaTime) * mUpDirection;
	}

	void Camera::MoveDown(float deltaTime)
	{
		mPosition += (mVelocity * deltaTime) * -mUpDirection;
	}

	void Camera::MoveRight(float deltaTime)
	{
		mPosition += (mVelocity * deltaTime) * mRightDirection;
	}

	void Camera::MoveLeft(float deltaTime)
	{
		mPosition += (mVelocity * deltaTime) * -mRightDirection;
	}

	void Camera::MoveForward(float deltaTime)
	{
		mPosition += (mVelocity * deltaTime) * mForwardDirection;
	}

	void Camera::MoveBackward(float deltaTime)
	{
		mPosition += (mVelocity * deltaTime) * -mForwardDirection;
	}

	DirectX::SimpleMath::Matrix Camera::GetViewMatrix() const
	{
		return DirectX::XMMatrixLookToLH(mPosition, mForwardDirection, mUpDirection);
	}

	DirectX::SimpleMath::Matrix Camera::GetProjMatrix() const
	{
		return DirectX::XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearZ, m_farZ);
	}
	DirectX::SimpleMath::Vector3 Camera::GetPosition() const
	{
		return mPosition;
	}
	DirectX::SimpleMath::Vector3 Camera::GetForwardDirection() const
	{
		return mForwardDirection;
	}
	DirectX::SimpleMath::Vector3 Camera::GetUpDirection() const
	{
		return mUpDirection;
	}
	DirectX::SimpleMath::Vector3 Camera::GetRightDirection() const
	{
		return mRightDirection;
	}
}
