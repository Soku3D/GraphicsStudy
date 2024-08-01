#pragma once
#include "directxtk/SimpleMath.h"
#include "Actor.h"

namespace Core {

	class Camera : public Actor {
	public:
		Camera();
		virtual ~Camera() {}

		float m_aspectRatio;
		float m_fov;
		float m_nearZ;
		float m_farZ;
		DirectX::SimpleMath::Vector3 m_position;
		DirectX::SimpleMath::Vector3 m_forwardDirection;
		DirectX::SimpleMath::Vector3 m_standardDirection;
		DirectX::SimpleMath::Vector3 m_rightDirection;
		DirectX::SimpleMath::Vector3 m_upDirection;
		float velocity = 1.f;
		float m_delTheta;
		float m_delSine;
		float m_delCosine;

		DirectX::SimpleMath::Quaternion m_quaternion;
		float m_xTheta = 0.f;
		float m_yTheta = 0.f;

	public:
		void SetAspectRation(float aspectRatio);
		void SetQuaternion(int deltaX, int deltaY);

		void SetRotation(int deltaX, int deltaY);

	public:
		DirectX::SimpleMath::Matrix GetViewMatrix() const;
		DirectX::SimpleMath::Matrix GetProjMatrix() const;

	public:
		virtual void RotateDirection() override;

		virtual void MoveUp(float deltaTime) override;
		virtual void MoveDown(float deltaTime) override;
		void MoveRight(float deltaTime) override;
		void MoveLeft(float deltaTime) override;
		void MoveForward(float deltaTime) override;
		void MoveBackward(float deltaTime) override;
	};
}