#ifndef RAYTRACINGHLSLCOMPAT_H
#define RAYTRACINGHLSLCOMPAT_H

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX;
#endif

struct SceneConstantBuffer {
	XMMATRIX projectionToWorld;
	XMVECTOR cameraPosition;
};


#endif // RAYTRACINGHLSLCOMPAT_H