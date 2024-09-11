#pragma once
#include "directxtk/SimpleMath.h"

namespace Core {

	class CameraComponent{
	public:
		CameraComponent();
		virtual ~CameraComponent() {}

		float m_aspectRatio;
		float m_fov;
		float m_nearZ;
		float m_farZ;
		float d;

		DirectX::SimpleMath::Vector3 mPosition;
		DirectX::SimpleMath::Vector3 mLookPosition;
		DirectX::SimpleMath::Vector3 mForwardDirection;
		DirectX::SimpleMath::Vector3 mStandardDirection;
		DirectX::SimpleMath::Vector3 mRightDirection;
		DirectX::SimpleMath::Vector3 mUpDirection;

		DirectX::SimpleMath::Vector3 mLocalPosition;

		float mVelocity = 1.f;
		float m_delTheta;
		float m_delSine;
		float m_delCosine;

		DirectX::SimpleMath::Quaternion m_quaternion;

		float mLocalXTheta = 0.f;
		float mLocalYTheta = 0.f;		
		
		float mXTheta = 0.f;
		float mYTheta = 0.f;

	public:
		void SetPositionAndDirection(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& direction);
		void SetAspectRatio(float aspectRatio);
		void SetQuaternion(int deltaX, int deltaY);
		void SetSpeed(float velocity);
		void SetRotation(int deltaX, int deltaY);

	public:
		DirectX::SimpleMath::Matrix GetViewMatrix();
		DirectX::SimpleMath::Matrix GetProjMatrix();
		DirectX::SimpleMath::Vector3 GetPosition() const ;

		DirectX::SimpleMath::Vector3 GetForwardDirection() const;
		DirectX::SimpleMath::Vector3 GetUpDirection() const;

		void Update(DirectX::SimpleMath::Vector3& position, float& xTheta, float& yTheta);

	public:
	};
}