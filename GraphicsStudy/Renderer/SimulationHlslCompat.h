#ifndef SIMULATIONHLSLCOMPAT_H
#define SIMULATIONHLSLCOMPAT_H

#ifdef HLSL
#include "HlslCompat.h"
#endif

struct Particle {
	XMFLOAT3 position;
	XMFLOAT3 originPosition;
	XMFLOAT3 color;
	XMFLOAT3 velocity;
	XMFLOAT3 originVelocity;
	float life;
	float radius;
};

#endif