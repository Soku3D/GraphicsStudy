#pragma once

#include "directxtk/SimpleMath.h"

namespace Renderer {
	struct SimpleVertex {
		DirectX::SimpleMath::Vector3 Position;
	};
	struct Vertex {
		DirectX::SimpleMath::Vector3 Position;
		DirectX::SimpleMath::Vector3 Normal;
		DirectX::SimpleMath::Vector2 Texcoord;
	};
}