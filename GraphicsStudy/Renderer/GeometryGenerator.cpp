#include "GeometryGenerator.h"
#include "directxtk/SimpleMath.h"

using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;
using namespace Renderer;

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

BasicMeshData GeomertyGenerator::Grid(const float& xLength, const float& yLength, const int& x, const int& y) {
	BasicMeshData data;
	if (x == 0 || y == 0)
	{
		return data;
	}
	Vector3 basePosition(Vector3(-xLength / 2.f, yLength / 2.f, 0.f));
	
	float delX = xLength / x;
	float dely = yLength / y;

	float uvDelX = 1.f / x;
	float uvDelY = 1.f / y;

	int index = 0;

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	for (int j = 0; j < y+1; ++j)
	{
		Vector3 position = basePosition - Vector3(0.f, j * dely, 0.f);
		for (int i = 0; i < x+1; ++i)
		{
			vertices.push_back(Vertex(Vector3(position + Vector3(i * delX, 0.f, 0.f)), Vector3(0.f,0.f,-1.f), Vector2(i*uvDelX, j*uvDelY)));
		}
	}

	for (int i = 0; i < y; i++)
	{
		int index = i * (x + 1);
		for (int j = 0; j < x; j++)
		{
			int idx = index + j;
			indices.push_back(idx + x + 1);
			indices.push_back(idx);
			indices.push_back(idx + 1);
			
			indices.push_back(idx + x + 1);
			indices.push_back(idx + 1);
			indices.push_back(idx + x + 2);
		}
	}

	data.m_vertices = vertices;
	data.m_indices = indices;
	return data;
}

