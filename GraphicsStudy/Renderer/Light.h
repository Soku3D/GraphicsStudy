#pragma once
#include "directxtk/SimpleMath.h"
#include "wrl.h"
#include "d3d12.h"

namespace Core {

	class Light {
	public:
		Light();
		virtual ~Light() {}

		void Initialize(const UINT & width, const UINT & height);

	public:
		float m_aspectRatio;
		float m_fov;
		float m_nearZ;
		float m_farZ;
		float d;
		DirectX::SimpleMath::Vector3 mPosition;
		DirectX::SimpleMath::Vector3 mForwardDirection;
		DirectX::SimpleMath::Vector3 mStandardDirection;
		DirectX::SimpleMath::Vector3 mRightDirection;
		DirectX::SimpleMath::Vector3 mUpDirection;
		float mVelocity = 1.f;
		float m_delTheta;
		float m_delSine;
		float m_delCosine;

		DirectX::SimpleMath::Quaternion m_quaternion;
		float m_xTheta = 0.f;
		float m_yTheta = 0.f;

	public:
		void SetPositionAndDirection(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& direction);
		void SetAspectRatio(float aspectRatio);
		void SetQuaternion(int deltaX, int deltaY);
		void SetSpeed(float velocity);
		void SetRotation(int deltaX, int deltaY);
		void SetFov(const float& degree);

	public:
		DirectX::SimpleMath::Matrix GetViewMatrix() const;
		DirectX::SimpleMath::Matrix GetProjMatrix() const;
		DirectX::SimpleMath::Vector3 GetPosition() const;

		DirectX::SimpleMath::Vector3 GetForwardDirection() const;
		DirectX::SimpleMath::Vector3 GetUpDirection() const;
		DirectX::SimpleMath::Vector3 GetRightDirection() const;
		float GetFov()  const { return m_fov; };
		float GetFarPlane()  const { return m_farZ; };
		float GetNeaPlane()  const { return m_nearZ; };
		float GetAspectRatio() const { return m_aspectRatio; };

	private:
		UINT mDepthWidth;
		UINT mDepthHeight;
		Microsoft::WRL::ComPtr<ID3D12Resource> mDepthBuffer;
	};
}