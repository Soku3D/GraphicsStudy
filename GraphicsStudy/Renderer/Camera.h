#pragma once
#include "directxtk/SimpleMath.h"

class Camera {
public:
	Camera();
	~Camera();

	float m_aspectRatio;
	float m_fov;
	float m_nearZ;
	float m_farZ;
	DirectX::SimpleMath::Vector3 m_position;
	DirectX::SimpleMath::Vector3 m_direction;

public:
	void SetAspectRation(float aspectRatio);
};