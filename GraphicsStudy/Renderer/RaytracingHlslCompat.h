#ifndef RAYTRACINGHLSLCOMPAT_H
#define RAYTRACINGHLSLCOMPAT_H

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX;
#endif

#define MAX_RAY_RECURSION_DEPTH 10

struct SceneConstantBuffer {
	XMMATRIX projectionToWorld;
	XMVECTOR cameraPosition;
};
struct RayPayload
{
	XMFLOAT4 color;
	UINT   recursionDepth;
};

#endif // RAYTRACINGHLSLCOMPAT_H