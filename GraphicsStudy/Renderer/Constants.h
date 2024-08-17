#pragma once

#include "directxtk/SimpleMath.h"

struct Light {
	DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3(5.f, 5.f, 0.f);
	float fallOfStart = 1.f;
	DirectX::SimpleMath::Vector3 Direction = DirectX::SimpleMath::Vector3::Zero;
	float fallOfEnd = 5.f;
};

struct Material {
	float albedo = 0.8f;
	float metalic = 0.8f;
	float roughness = 1.f;
	float dummy;
};


__declspec(align(256)) struct GlobalVertexConstantData
{
	DirectX::SimpleMath::Matrix ViewMat = DirectX::SimpleMath::Matrix();
	DirectX::SimpleMath::Matrix ProjMat = DirectX::SimpleMath::Matrix();
};

__declspec(align(256)) struct ObjectConstantData {
	DirectX::SimpleMath::Matrix Model = DirectX::SimpleMath::Matrix();
	DirectX::SimpleMath::Matrix invTranspose = DirectX::SimpleMath::Matrix();
	Material Material;
	bool bUseNormalMap;
	DirectX::SimpleMath::Vector3 dummy;
};
__declspec(align(256)) struct ObjectConstantData2 {
	DirectX::SimpleMath::Matrix Model = DirectX::SimpleMath::Matrix();
};

__declspec(align(256)) struct LightPassConstantData {
	Light light[1];
	DirectX::SimpleMath::Vector3 eyePos = DirectX::SimpleMath::Vector3(0.f,0.f,-1.f);
	float lod = 0.f;
};

__declspec(align(256)) struct CSConstantData {
	float time;
	float dummy[3];
};

__declspec(align(256)) struct CubeMapConstantData {
	float expose;
	float lodLevel;
	DirectX::SimpleMath::Vector2 dummy;
};