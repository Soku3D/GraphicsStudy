#pragma once

#include "directxtk/SimpleMath.h"

struct Light {
	DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3(5.f, 5.f, 0.f);
	float fallOfStart = 1.f;
	DirectX::SimpleMath::Vector3 Direction = DirectX::SimpleMath::Vector3::Zero;
	float fallOfEnd = 5.f;
};

struct Material {
	DirectX::SimpleMath::Vector3 ambient = DirectX::SimpleMath::Vector3(0.2f);
	float shineiness = 0.f;
	DirectX::SimpleMath::Vector3 diffuse = DirectX::SimpleMath::Vector3(0.8f, 0.8f, 0.8f);
	float dummy1;
	DirectX::SimpleMath::Vector3 specular = DirectX::SimpleMath::Vector3(1.f);
	float dummy2;
};

__declspec(align(256)) struct GlobalVertexConstantData
{
	DirectX::SimpleMath::Matrix ViewMat = DirectX::SimpleMath::Matrix();
	DirectX::SimpleMath::Matrix ProjMat = DirectX::SimpleMath::Matrix();
};

__declspec(align(256)) struct ObjectConstantData {
	DirectX::SimpleMath::Matrix Model = DirectX::SimpleMath::Matrix();
};


__declspec(align(256)) struct PSConstantData {
	Light light[1];
	Material material;
	DirectX::SimpleMath::Vector3 eyePos = DirectX::SimpleMath::Vector3(0.f,0.f,-1.f);
};

__declspec(align(256)) struct CSConstantData {
	float time;
	float dummy[3];
};