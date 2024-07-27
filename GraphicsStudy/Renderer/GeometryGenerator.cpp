#include "GeometryGenerator.h"
#include "directxtk/SimpleMath.h"

using DirectX::SimpleMath::Vector3;
SimpleMeshData GeomertyGenerator::SimpleTriangle(const float& length)
{
	SimpleMeshData data;

	float x = (float)sqrt(3);
	float l = length / 2.f;
	std::vector<Renderer::SimpleVertex> vertices = {
		{Vector3(-l, -l / x, 0.0f)},
		{Vector3(0.f, l * 2.f / x , 0.0f)},
		{Vector3(l, -l / x, 0.0f)},
	};
	std::vector<uint16_t> indices = {
		0, 1, 2
	};
	data.Initialize(vertices, indices);

	return data;
}
