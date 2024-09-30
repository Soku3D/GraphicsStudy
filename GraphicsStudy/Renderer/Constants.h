#pragma once

#include "directxtk/SimpleMath.h"

struct Light {
	DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3(5.f, 5.f, 0.f);
	float fallOfStart = 1.f;
	DirectX::SimpleMath::Vector3 Direction = DirectX::SimpleMath::Vector3::Zero;
	float fallOfEnd = 5.f;
};

//albedo
//ao
//metallic
//roughness
struct Material {
	float albedo = 0.8f;
	float ao = 1.f;
	float metallic = 0.8f;
	float roughness = 1.f;
};


__declspec(align(256)) struct GlobalVertexConstantData
{
	DirectX::SimpleMath::Matrix ViewMat = DirectX::SimpleMath::Matrix();
	DirectX::SimpleMath::Matrix ProjMat = DirectX::SimpleMath::Matrix();
	DirectX::SimpleMath::Vector3 eyePosition;
	float dummy;
};

__declspec(align(256)) struct GlobalConstantDataDLSS
{
	DirectX::SimpleMath::Matrix ViewMat = DirectX::SimpleMath::Matrix();
	DirectX::SimpleMath::Matrix ProjMat = DirectX::SimpleMath::Matrix();
	DirectX::SimpleMath::Matrix prevViewMat = DirectX::SimpleMath::Matrix();
	DirectX::SimpleMath::Matrix prevProjMat = DirectX::SimpleMath::Matrix();
	DirectX::SimpleMath::Vector3 eyePosition;
	float dummy;
};

__declspec(align(256)) struct ObjectConstantData {
	DirectX::SimpleMath::Matrix Model = DirectX::SimpleMath::Matrix();
	DirectX::SimpleMath::Matrix invTranspose = DirectX::SimpleMath::Matrix();
	Material Material;

	float bUseAoMap = false;
	float bUseHeightMap = false;
	float bUseMetalnessMap = false;
	float bUseNormalMap = false;
	
	float bUseRoughnessMap = false;
	float bUseTesslation = false;
	float boundingBoxHalfLengthX = 0.f;
	float boundingBoxHalfLengthY = 0.f;

	float boundingBoxHalfLengthZ = 0.f;

};

__declspec(align(256)) struct LightPassConstantData {
	Light light[1];
	DirectX::SimpleMath::Vector3 eyePos = DirectX::SimpleMath::Vector3(0.f,0.f,-1.f);
	float lod = 0.f;

	float ao = 0.f;
	float metallic = 0.f;
	float roughness = 0.f;
	float expose = 1.f;
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


__declspec(align(256)) struct SimulationCSConstantData {
	float deltaTime;
	float time;
};

__declspec(align(256)) struct CFDConstantData {
	DirectX::SimpleMath::Vector3 color;
	float deltaTime;
	DirectX::SimpleMath::Vector3 velocity;
	float radius;

	float viscosity;
	float vorticity;
	uint32_t i;
	uint32_t j;

	float time;
	float sourceStrength;
};
__declspec(align(256)) struct PostprocessingConstantData {
	float bUseGamma;
};

__declspec(align(256)) struct VolumeConstantData {
	float deltaTime;
};