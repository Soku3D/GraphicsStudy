#ifndef SIMULATIONHLSLCOMPAT_H
#define SIMULATIONHLSLCOMPAT_H

#ifdef HLSL
#include "HlslCompat.h"
#endif

struct Particle {
	XMFLOAT3 mPosition;
	XMFLOAT3 mOriginPosition;
	XMFLOAT3 mColor;
	XMFLOAT3 mVelocity;
	XMFLOAT3 mOriginVelocity;
	float mLife;
	float mRadius;
};

#endif