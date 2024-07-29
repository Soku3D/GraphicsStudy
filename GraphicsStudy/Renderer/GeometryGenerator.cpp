#include "GeometryGenerator.h"
#include "directxtk/SimpleMath.h"

using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;

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

SimpleMeshData GeomertyGenerator::SimpleRectangle(const float& length)
{
	SimpleMeshData data;

	float l = length / 2.f;
	std::vector<Renderer::SimpleVertex> vertices = {
		{Vector3(-l, -l, 0.0f)},
		{Vector3(-l, l, 0.0f)},
		{Vector3(l, l, 0.0f)},
		{Vector3(l, -l, 0.0f)},
	};
	std::vector<uint16_t> indices = {
		0, 1, 2, 0, 2, 3
	};
	data.Initialize(vertices, indices);

	return data;
}

SimpleMeshData GeomertyGenerator::SimpleBox(const float& length)
{
	SimpleMeshData data;

	float l = length / 2.f;
 
	std::vector<Renderer::SimpleVertex> vertices = {
		{Vector3(-l, -l, -l)},{Vector3(-l, l, -l)},{Vector3(l, l, -l)},{Vector3(l, -l, -l)},
		{Vector3(-l, -l, l)},{Vector3(-l, l, l)},{Vector3(l, l, l)},{Vector3(l, -l, l)}
	};
	std::vector<uint16_t> indices = {
		0, 1, 2, 0, 2, 3,
		7, 6, 5, 7, 5, 4,
		4, 5, 1, 5, 1, 0,
		4, 0, 3, 4, 3, 7,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2
	};
	data.Initialize(vertices, indices);

	return data;
}

BasicMeshData GeomertyGenerator::Box(const float& length)
{
	BasicMeshData data;

	float l = length / 2.f;

	std::vector<Renderer::Vertex> vertices = {
		{Vector3(-l, -l, -l), Vector3(0.f,0.f,-1.f), Vector2(0.f, 1.f)},
		{Vector3(-l, l, -l), Vector3(0.f,0.f,-1.f), Vector2(0.f, 0.f)},
		{Vector3(l, l, -l), Vector3(0.f,0.f,-1.f), Vector2(1.f, 0.f)},
		{Vector3(l, -l, -l), Vector3(0.f,0.f,-1.f), Vector2(1.f, 1.f)},

		{Vector3(l, -l, l), Vector3(0.f,0.f, 1.f), Vector2(1.f, 1.f)},
		{Vector3(l, l, l), Vector3(0.f,0.f, 1.f), Vector2(1.f, 0.f)},
		{Vector3(-l, l, l), Vector3(0.f,0.f, 1.f), Vector2(0.f, 0.f)},
		{Vector3(-l, -l, l), Vector3(0.f,0.f, 1.f), Vector2(0.f, 1.f)},

		{Vector3(-l, -l, l), Vector3(-1.f,0.f,0.f), Vector2(0.f, 1.f)},
		{Vector3(-l, l, l), Vector3(-1.f,0.f, 0.f), Vector2(0.f, 0.f)},
		{Vector3(-l, l, -l), Vector3(-1.f,0.f, 0.f), Vector2(1.f, 0.f)},
		{Vector3(-l, -l, -l), Vector3(-1.f,0.f, 0.f), Vector2(1.f, 1.f)},
		
		{Vector3(l, -l, -l), Vector3(1.f,0.f,0.f), Vector2(1.f, 1.f)},
		{Vector3(l, l, -l), Vector3(1.f,0.f,0.f), Vector2(1.f, 0.f)},
		{Vector3(l, l, l), Vector3(1.f,0.f,0.f), Vector2(0.f, 0.f)},
		{Vector3(l, -l, l), Vector3(1.f,0.f,0.f), Vector2(0.f, 1.f)},		

		{Vector3(-l, l, -l), Vector3(0.f, 1.f, 0.f), Vector2(0.f, 1.f)},
		{Vector3(-l, l, l), Vector3(0.f, 1.f, 0.f), Vector2(0.f, 0.f)},
		{Vector3(l, l, l), Vector3(0.f, 1.f, 0.f), Vector2(1.f, 0.f)},
		{Vector3(l, l, -l), Vector3(0.f, 1.f, 0.f), Vector2(1.f, 1.f)},

		{Vector3(l, -l, -l), Vector3(0.f, -1.f, 0.f), Vector2(1.f, 1.f)},
		{Vector3(l, -l, l), Vector3(0.f, -1.f, 0.f), Vector2(1.f, 0.f)},
		{Vector3(-l, -l, l), Vector3(0.f, -1.f, 0.f), Vector2(0.f, 0.f)},
		{Vector3(-l, -l, -l), Vector3(0.f, -1.f, 0.f), Vector2(0.f, 1.f)}

		
		
	};
	std::vector<uint16_t> indices;
	
	for (int i = 0; i < 6; i++)
	{
		int idx = i * 4;
		indices.push_back(idx);
		indices.push_back(idx+1);
		indices.push_back(idx+2);

		indices.push_back(idx);
		indices.push_back(idx+2);
		indices.push_back(idx+3);
	}
	data.Initialize(vertices, indices);

	return data;
}