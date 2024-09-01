#ifndef RAYTRACINGHLSLCOMPAT_H
#define RAYTRACINGHLSLCOMPAT_H

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX;
#endif

#define MAX_RAY_RECURSION_DEPTH 1

struct SceneConstantBuffer {
	XMMATRIX cubeMapView;
	XMMATRIX view;
	XMMATRIX projection;
	XMVECTOR cameraPosition;
	float d;
};
struct PrimitiveConstantBuffer {
	XMMATRIX invTranspose;
};
struct RayPayload
{
	XMFLOAT4 color;
	UINT   recursionDepth;
	bool isHit;
};
struct RaytracingVertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 texcoord;
	XMFLOAT3 tangent;
};
#endif // RAYTRACINGHLSLCOMPAT_H