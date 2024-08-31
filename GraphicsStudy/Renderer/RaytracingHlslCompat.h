#ifndef RAYTRACINGHLSLCOMPAT_H
#define RAYTRACINGHLSLCOMPAT_H

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX;
#endif

#define MAX_RAY_RECURSION_DEPTH 1

struct SceneConstantBuffer {
	XMMATRIX projectionToWorld;
	XMVECTOR cameraPosition;
};
struct PrimitiveConstantBuffer {
	XMFLOAT4 color;
};
struct RayPayload
{
	XMFLOAT4 color;
	UINT   recursionDepth;
};
struct RaytracingVertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
};
#endif // RAYTRACINGHLSLCOMPAT_H