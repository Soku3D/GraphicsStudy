#pragma once

#include "directxtk/SimpleMath.h"

namespace Renderer {
	struct SimpleVertex {
		DirectX::SimpleMath::Vector3 position;
	};
	struct Vertex {
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector3 normal;
		DirectX::SimpleMath::Vector2 texcoord;
	};
	struct PbrVertex {
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector3 normal;
		DirectX::SimpleMath::Vector2 texcoord;
		DirectX::SimpleMath::Vector3 tangent;
	};
	struct PbrSkinnedVertex {
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector3 normal;
		DirectX::SimpleMath::Vector2 texcoord;
		DirectX::SimpleMath::Vector3 tangent;

		float blendWeights[8] = { 0.0f, 0.0f, 0.0f, 0.0f,
						   0.0f, 0.0f, 0.0f, 0.0f };  // BLENDWEIGHT0 and 1
		uint8_t boneIndices[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // BLENDINDICES0 and 1
	};
/*	struct RaytracingVertex {
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector3 normal;
	}*/;
}