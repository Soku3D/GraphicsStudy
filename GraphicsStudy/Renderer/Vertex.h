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
	struct RaytracingVertex {
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector3 normal;
	};
}