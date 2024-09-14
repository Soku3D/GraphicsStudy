#include "CameraComponent.h"
#include <iostream>

namespace Core {
	using DirectX::SimpleMath::Vector3;

	CameraComponent::CameraComponent() :
		mLocalPosition(DirectX::SimpleMath::Vector3(0.f, 0.f, -1.f)),
		mUpDirection(DirectX::SimpleMath::Vector3(0, 1, 0)),
		mForwardDirection(DirectX::SimpleMath::Vector3(0, 0, 1)),
		mStandardDirection(DirectX::SimpleMath::Vector3(0, 0, 1)),
		mLookPosition(DirectX::SimpleMath::Vector3(0, 0, 0)),
		m_farZ(1000.f),
		m_nearZ(0.1f)
	{
		using DirectX::SimpleMath::Vector3;

		m_fov = DirectX::XMConvertToRadians(70.f);
		m_delTheta = DirectX::XMConvertToRadians(0.05f);
		m_delSine = sin(m_delTheta / 2.f);
		m_delCosine = cos(m_delTheta / 2.f);

		mPosition = mLocalPosition;

		d = 1.f / tan(m_fov / 2.f);
	}

	void CameraComponent::SetPositionAndDirection(const DirectX::SimpleMath::Vector3& position,
		const DirectX::SimpleMath::Vector3& direction)
	{
		using DirectX::SimpleMath::Vector3;

		mPosition = position;

	 	Vector3 v0 = Vector3(0, direction.y, direction.z);
		Vector3 v1 = Vector3(direction.x, 0, direction.z);

		v0.Normalize();
		v1.Normalize();
		float cosTheta1 = abs(v0.Dot(Vector3(0, 0, 1)));
		float cosTheta2 = v1.Dot(Vector3(0, 0, 1));
		mLocalXTheta = acos(cosTheta1);
		mLocalYTheta = acos(cosTheta2);
		if (v0.y > 0) {
			mLocalXTheta *= -1.0;
		}
		if (v1.x < 0) {
			mLocalYTheta *= -1.0;
		}
		m_fov = DirectX::XMConvertToRadians(70.0f);
		m_delTheta = DirectX::XMConvertToRadians(0.2f);
		m_delSine = sin(m_delTheta / 2.f);
		m_delCosine = cos(m_delTheta / 2.f);

	}

	void CameraComponent::SetAspectRatio(float aspectRatio)
	{
		m_aspectRatio = aspectRatio;
	}

	void CameraComponent::SetQuaternion(int deltaX, int deltaY)
	{
		DirectX::SimpleMath::Vector3 n((float)deltaY * m_aspectRatio, (float)deltaX, 0.f);
		m_quaternion = DirectX::SimpleMath::Quaternion(n * m_delSine, m_delCosine);
	}

	void CameraComponent::SetSpeed(float velocity)
	{
		mVelocity = velocity;
	}

	void CameraComponent::SetRotation(int deltaX, int deltaY) {
		mLocalXTheta += deltaY * m_aspectRatio * m_delTheta;
		mLocalYTheta += deltaX * m_delTheta;

		if (mLocalXTheta >= DirectX::XM_PIDIV2 - 0.001f) {
			mLocalXTheta = DirectX::XM_PIDIV2 - 0.001f;
		}
		if (mLocalXTheta <= -DirectX::XM_PIDIV2 + 0.001f) {
			mLocalXTheta = -DirectX::XM_PIDIV2 + 0.001f;
		}
	}

	DirectX::SimpleMath::Matrix CameraComponent::GetViewMatrix()
	{
		//return DirectX::XMMatrixLookAtLH(position, DirectX::SimpleMath::Vector3::Zero , mUpDirection);
		return DirectX::XMMatrixLookToLH(mPosition, mForwardDirection, mUpDirection);

	}

	DirectX::SimpleMath::Matrix CameraComponent::GetProjMatrix()
	{
		return DirectX::XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearZ, m_farZ);
	}
	DirectX::SimpleMath::Vector3 CameraComponent::GetPosition() const
	{
		return mPosition;
	}
	DirectX::SimpleMath::Vector3 CameraComponent::GetForwardDirection() const
	{
		return mForwardDirection;
	}
	DirectX::SimpleMath::Vector3 CameraComponent::GetUpDirection() const
	{
		return mUpDirection;
	}
	void CameraComponent::Update(
		DirectX::SimpleMath::Vector3& position,
		float & xTheta,
		float& yTheta)
	{
		mLookPosition = position;

		/*Vector3 pos = Vector3::Transform(mLocalPosition, DirectX::XMMatrixRotationY(yTheta));
		pos = Vector3::Transform(pos, DirectX::XMMatrixRotationX(xTheta));*/
		
		
		mXTheta = mLocalXTheta + xTheta;
		mYTheta = mLocalYTheta + yTheta;

		DirectX::SimpleMath::Matrix mat = DirectX::XMMatrixRotationX(mXTheta) * DirectX::XMMatrixRotationY(mYTheta);
		//DirectX::SimpleMath::Matrix mat = DirectX::XMMatrixRotationY(mYTheta);

		mForwardDirection = Vector3::Transform(mStandardDirection, mat);
		mPosition = Vector3::Transform(mLocalPosition, mat);
		mPosition += position;

		mForwardDirection.Normalize();
		//std::cout << "Camera Rotate : " << mYTheta << '\n';
	}
}
