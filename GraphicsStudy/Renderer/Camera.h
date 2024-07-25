#pragma once
#include "directxtk/SimpleMath.h"

class Camera {
public:
	Camera();
	virtual ~Camera() {}

	float m_aspectRatio;
	float m_fov;
	float m_nearZ;
	float m_farZ;
	DirectX::SimpleMath::Vector3 m_position;
	DirectX::SimpleMath::Vector3 m_forwardDirection;
	DirectX::SimpleMath::Vector3 m_rightDirection;
	DirectX::SimpleMath::Vector3 m_upDirection;
	float velocity = 1.f;

public:
	void SetAspectRation(float aspectRatio);

public:
	DirectX::SimpleMath::Matrix GetViewMatrix() const;
	DirectX::SimpleMath::Matrix GetProjMatrix() const;

public:
	void MoveUp(float deltaTime);
	void MoveDown(float deltaTime);
	void MoveRight(float deltaTime);
	void MoveLeft(float deltaTime);
	void MoveForward(float deltaTime);
	void MoveBackward(float deltaTime);
};