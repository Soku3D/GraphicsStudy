#include "GeometryGenerator.h"
#include <memory>

using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;
using namespace Renderer;

SimpleMeshData GeometryGenerator::SimpleTriangle(const float& length)
{
	SimpleMeshData data;

	float x = (float)sqrt(3);
	float l = length / 2.f;
	std::vector<Renderer::SimpleVertex> vertices = {
		{Vector3(-l, -l / x, 0.0f)},
		{Vector3(0.f, l * 2.f / x , 0.0f)},
		{Vector3(l, -l / x, 0.0f)},
	};
	std::vector<uint32_t> indices = {
		0, 1, 2
	};
	data.Initialize(vertices, indices);
	data.m_name = "SimpleTriangle";
	return data;
}

SimpleMeshData GeometryGenerator::SimpleRectangle(const float& length)
{
	SimpleMeshData data;

	float l = length;
	std::vector<Renderer::SimpleVertex> vertices = {
		{Vector3(-l, -l, 0.0f)},
		{Vector3(-l, l, 0.0f)},
		{Vector3(l, l, 0.0f)},
		{Vector3(l, -l, 0.0f)},
	};
	std::vector<uint32_t> indices = {
		0, 1, 2, 0, 2, 3
	};
	data.Initialize(vertices, indices);

	return data;
}

SimpleMeshData GeometryGenerator::SimpleBox(const float& length)
{
	SimpleMeshData data;

	float l = length;

	std::vector<Renderer::SimpleVertex> vertices = {
		{Vector3(-l, -l, -l)},{Vector3(-l, l, -l)},{Vector3(l, l, -l)},{Vector3(l, -l, -l)},
		{Vector3(-l, -l, l)},{Vector3(-l, l, l)},{Vector3(l, l, l)},{Vector3(l, -l, l)}
	};
	std::vector<uint32_t> indices = {
		0, 1, 2, 0, 2, 3,
		7, 6, 5, 7, 5, 4,
		4, 5, 1, 4, 1, 0,
		4, 0, 3, 4, 3, 7,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2
	};
	data.Initialize(vertices, indices);
	data.m_name = "SimpleBox";
	return data;
}


SimpleMeshData GeometryGenerator::SimpleCubeMapBox(const float& length)
{
	SimpleMeshData data;

	float l = length;

	std::vector<Renderer::SimpleVertex> vertices = {
		{Vector3(-l, -l, -l)},{Vector3(-l, l, -l)},{Vector3(l, l, -l)},{Vector3(l, -l, -l)},
		{Vector3(-l, -l, l)},{Vector3(-l, l, l)},{Vector3(l, l, l)},{Vector3(l, -l, l)}
	};
	std::vector<uint32_t> indices = {
		0, 2, 1, 0, 3, 2,
		7, 5, 6, 7, 4, 5,
		4, 1, 5, 4, 0, 1,
		4, 3, 0, 4, 7, 3,
		3, 6, 2, 3, 7, 6,
		1, 6, 5, 1, 2, 6
	};
	data.Initialize(vertices, indices);

	return data;
}

BasicMeshData GeometryGenerator::Rectangle(const float& length, const std::wstring& texturePath)
{
	BasicMeshData data;

	float hX = length / 2.f;
	float hY = length / 2.f;


	std::vector<Renderer::Vertex> vertices = {
		{Vector3(-hX, -hY, 0.f), Vector3(0.f,0.f,-1.f), Vector2(0.f, 1.f)},
		{Vector3(-hX, hY,  0.f), Vector3(0.f,0.f,-1.f), Vector2(0.f, 0.f)},
		{Vector3(hX, hY,  0.f), Vector3(0.f,0.f,-1.f), Vector2(1.f, 0.f)},
		{Vector3(hX, -hY,  0.f), Vector3(0.f,0.f,-1.f), Vector2(1.f, 1.f)},

	};
	std::vector<uint32_t> indices = {
		0,1,2,0,2,3
	};


	data.Initialize(vertices, indices, texturePath);

	return data;
}

BasicMeshData GeometryGenerator::Box(const float& length, const std::wstring& texturePath)
{
	return Box(length, length, length, texturePath);
}


BasicMeshData GeometryGenerator::Box(const float& x, const float& y, const float& z, const std::wstring& texturePath)
{
	BasicMeshData data;

	float hX = x / 2.f;
	float hY = y / 2.f;
	float hZ = z / 2.f;

	std::vector<Renderer::Vertex> vertices = {
		{Vector3(-hX, -hY, -hZ), Vector3(0.f,0.f,-1.f), Vector2(0.f, 1.f)},
		{Vector3(-hX, hY, -hZ), Vector3(0.f,0.f,-1.f), Vector2(0.f, 0.f)},
		{Vector3(hX, hY, -hZ), Vector3(0.f,0.f,-1.f), Vector2(1.f, 0.f)},
		{Vector3(hX, -hY, -hZ), Vector3(0.f,0.f,-1.f), Vector2(1.f, 1.f)},

		{Vector3(hX, -hY, hZ), Vector3(0.f,0.f, 1.f), Vector2(1.f, 1.f)},
		{Vector3(hX, hY, hZ), Vector3(0.f,0.f, 1.f), Vector2(1.f, 0.f)},
		{Vector3(-hX, hY, hZ), Vector3(0.f,0.f, 1.f), Vector2(0.f, 0.f)},
		{Vector3(-hX, -hY, hZ), Vector3(0.f,0.f, 1.f), Vector2(0.f, 1.f)},

		{Vector3(-hX, -hY, hZ), Vector3(-1.f,0.f,0.f), Vector2(0.f, 1.f)},
		{Vector3(-hX, hY, hZ), Vector3(-1.f,0.f, 0.f), Vector2(0.f, 0.f)},
		{Vector3(-hX, hY, -hZ), Vector3(-1.f,0.f, 0.f), Vector2(1.f, 0.f)},
		{Vector3(-hX, -hY, -hZ), Vector3(-1.f,0.f, 0.f), Vector2(1.f, 1.f)},

		{Vector3(hX, -hY, -hZ), Vector3(1.f,0.f,0.f), Vector2(1.f, 1.f)},
		{Vector3(hX, hY, -hZ), Vector3(1.f,0.f,0.f), Vector2(1.f, 0.f)},
		{Vector3(hX, hY, hZ), Vector3(1.f,0.f,0.f), Vector2(0.f, 0.f)},
		{Vector3(hX, -hY, hZ), Vector3(1.f,0.f,0.f), Vector2(0.f, 1.f)},

		// 윗 면 (xz 평면)
		{Vector3(-hX, hY, -hZ), Vector3(0.f, 1.f, 0.f), Vector2(0.f, 1.f)},
		{Vector3(-hX, hY, hZ), Vector3(0.f, 1.f, 0.f), Vector2(0.f, 0.f)},
		{Vector3(hX, hY, hZ), Vector3(0.f, 1.f, 0.f), Vector2(1.f, 0.f)},
		{Vector3(hX, hY, -hZ), Vector3(0.f, 1.f, 0.f), Vector2(1.f, 1.f)},

		// 아랫 면
		{Vector3(hX, -hY, -hZ), Vector3(0.f, -1.f, 0.f), Vector2(1.f, 1.f)},
		{Vector3(hX, -hY, hZ), Vector3(0.f, -1.f, 0.f), Vector2(1.f, 0.f)},
		{Vector3(-hX, -hY, hZ), Vector3(0.f, -1.f, 0.f), Vector2(0.f, 0.f)},
		{Vector3(-hX, -hY, -hZ), Vector3(0.f, -1.f, 0.f), Vector2(0.f, 1.f)}
	};
	std::vector<uint32_t> indices;

	for (int i = 0; i < 6; i++)
	{
		int idx = i * 4;
		indices.push_back(idx);
		indices.push_back(idx + 1);
		indices.push_back(idx + 2);

		indices.push_back(idx);
		indices.push_back(idx + 2);
		indices.push_back(idx + 3);
	}
	data.Initialize(vertices, indices, texturePath);

	return data;
}
BasicMeshData GeometryGenerator::Grid(const float& xLength, const float& yLength, const int& x, const int& y, const std::wstring& texturePath) {
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
	std::vector<uint32_t> indices;
	for (int j = 0; j < y + 1; ++j)
	{
		Vector3 position = basePosition - Vector3(0.f, j * dely, 0.f);
		for (int i = 0; i < x + 1; ++i)
		{
			vertices.push_back(Vertex(Vector3(position + Vector3(i * delX, 0.f, 0.f)), Vector3(0.f, 0.f, -1.f), Vector2(i * uvDelX, j * uvDelY)));
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
	data.Initialize(vertices, indices, texturePath);

	return data;
}

BasicMeshData GeometryGenerator::Cyilinder(const float& topRadius, const float& bottomRadius, const float& height, const int& x, const int& y, const std::wstring& texturePath) {

	BasicMeshData data;
	if (x == 0 || y == 0)
	{
		return data;
	}
	Vector3 basePosition(Vector3(-topRadius, height / 2.f, 0.f));

	float delY = height / y;
	float delX = (bottomRadius - topRadius) / y;

	float delTheta = DirectX::XM_2PI / x;
	float uvDelX = 1.f / x;
	float uvDelY = 1.f / y;

	int index = 0;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	for (int j = 0; j < y + 1; ++j)
	{
		Vector3 position = basePosition - Vector3(j * delX, j * delY, 0.f);
		for (int i = 0; i < x + 1; ++i)
		{
			Vertex v;
			v.position = Vector3::Transform(position, DirectX::XMMatrixRotationY(-delTheta * i));
			v.texcoord = Vector2(i * uvDelX, j * uvDelY);
			v.normal = v.position;
			v.normal.y = 0.f;
			v.normal.Normalize();

			vertices.push_back(v);
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

	data.Initialize(vertices, indices, texturePath);
	return data;
}

BasicMeshData GeometryGenerator::Sphere(const float& radius, const int& x, const int& y, const std::wstring& texturePath)
{
	BasicMeshData data;
	if (x == 0 || y == 0)
	{
		return data;
	}
	Vector3 basePosition(Vector3(0.f, radius / 2.f, 0.f));


	float delYTheta = DirectX::XM_2PI / x;
	float delZTheta = DirectX::XM_PI / y;

	float uvDelX = 1.f / x;
	float uvDelY = 1.f / y;

	int index = 0;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	for (int j = 0; j < y + 1; ++j)
	{
		Vector3 position = Vector3::Transform(basePosition, DirectX::XMMatrixRotationZ(-delZTheta * j));
		for (int i = 0; i < x + 1; ++i)
		{
			Vertex v;
			v.position = Vector3::Transform(position, DirectX::XMMatrixRotationY(-delYTheta * i));
			v.texcoord = Vector2(i * uvDelX, j * uvDelY);
			v.normal = v.position;
			v.normal.Normalize();

			vertices.push_back(v);
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
	data.Initialize(vertices, indices, texturePath);

	return data;
}

PbrMeshData GeometryGenerator::PbrTriangle(const float& length, const std::wstring& texturePath)
{
	PbrMeshData data;

	
	float x = (float)sqrt(3);
	float l = length;
	std::vector<Renderer::PbrVertex> vertices = {
		{Vector3(-l, -l / x, 0.0f), Vector3(0.f,0.f,-1.f), Vector2(0.f, 1.f),Vector3::Zero},
		{Vector3(0.f, l * 2.f / x , 0.0f), Vector3(0.f,0.f,-1.f), Vector2(0.f, 0.f),Vector3::Zero},
		{Vector3(l, -l / x, 0.0f), Vector3(0.f,0.f,-1.f), Vector2(1.f, 0.f),Vector3::Zero},
	};
	std::vector<uint32_t> indices = {
		0, 1, 2
	};
	ComputeTangent(vertices[0], vertices[1], vertices[2]);

	data.Initialize(vertices, indices, texturePath);
	data.m_name = "SimpleTriangle";
	return data;
}

PbrMeshData GeometryGenerator::PbrRectangle(const float& length, const std::wstring& texturePath)
{
	return PbrMeshData();
}

PbrMeshData GeometryGenerator::PbrBox(const float& length, const std::wstring& texturePath)
{
	return PbrBox(length, length, length, texturePath);
}

PbrMeshData GeometryGenerator::PbrBox(const float& x, const float& y, const float& z, const std::wstring& texturePath,
	const float& UVx, const float& UVy, const float& UVz)
{
	PbrMeshData data;

	float hX = x;
	float hY = y;
	float hZ = z;

	std::vector<PbrVertex> vertices = {
		{Vector3(-hX, -hY, -hZ), Vector3(0.f,0.f,-1.f), Vector2(0.f, 1.f),Vector3::Zero},
		{Vector3(-hX, hY, -hZ), Vector3(0.f,0.f,-1.f), Vector2(0.f, 0.f),Vector3::Zero},
		{Vector3(hX, hY, -hZ), Vector3(0.f,0.f,-1.f), Vector2(1.f, 0.f),Vector3::Zero},
		{Vector3(hX, -hY, -hZ), Vector3(0.f,0.f,-1.f), Vector2(1.f, 1.f),Vector3::Zero},

		{Vector3(hX, -hY, hZ), Vector3(0.f,0.f, 1.f), Vector2(1.f, 1.f),Vector3::Zero},
		{Vector3(hX, hY, hZ), Vector3(0.f,0.f, 1.f), Vector2(1.f, 0.f),Vector3::Zero},
		{Vector3(-hX, hY, hZ), Vector3(0.f,0.f, 1.f), Vector2(0.f, 0.f),Vector3::Zero},
		{Vector3(-hX, -hY, hZ), Vector3(0.f,0.f, 1.f), Vector2(0.f, 1.f),Vector3::Zero},

		{Vector3(-hX, -hY, hZ), Vector3(-1.f,0.f,0.f), Vector2(0.f, 1.f),Vector3::Zero},
		{Vector3(-hX, hY, hZ), Vector3(-1.f,0.f, 0.f), Vector2(0.f, 0.f),Vector3::Zero},
		{Vector3(-hX, hY, -hZ), Vector3(-1.f,0.f, 0.f), Vector2(1.f, 0.f),Vector3::Zero},
		{Vector3(-hX, -hY, -hZ), Vector3(-1.f,0.f, 0.f), Vector2(1.f, 1.f),Vector3::Zero},

		{Vector3(hX, -hY, -hZ), Vector3(1.f,0.f,0.f), Vector2(1.f, 1.f),Vector3::Zero},
		{Vector3(hX, hY, -hZ), Vector3(1.f,0.f,0.f), Vector2(1.f, 0.f),Vector3::Zero},
		{Vector3(hX, hY, hZ), Vector3(1.f,0.f,0.f), Vector2(0.f, 0.f),Vector3::Zero},
		{Vector3(hX, -hY, hZ), Vector3(1.f,0.f,0.f), Vector2(0.f, 1.f),Vector3::Zero},

		// 윗 면 (xz 평면)
		{Vector3(-hX, hY, -hZ), Vector3(0.f, 1.f, 0.f), Vector2(0.f, UVz),Vector3::Zero},
		{Vector3(-hX, hY, hZ), Vector3(0.f, 1.f, 0.f), Vector2(0.f, 0.f),Vector3::Zero},
		{Vector3(hX, hY, hZ), Vector3(0.f, 1.f, 0.f), Vector2(UVx, 0.f),Vector3::Zero},
		{Vector3(hX, hY, -hZ), Vector3(0.f, 1.f, 0.f), Vector2(UVx, UVz),Vector3::Zero},

		// 아랫 면
		{Vector3(hX, -hY, -hZ), Vector3(0.f, -1.f, 0.f), Vector2(UVx, UVz),Vector3::Zero},
		{Vector3(hX, -hY, hZ), Vector3(0.f, -1.f, 0.f), Vector2(UVx, 0.f),Vector3::Zero},
		{Vector3(-hX, -hY, hZ), Vector3(0.f, -1.f, 0.f), Vector2(0.f, 0.f),Vector3::Zero},
		{Vector3(-hX, -hY, -hZ), Vector3(0.f, -1.f, 0.f), Vector2(0.f, UVz),Vector3::Zero}
	};
	std::vector<uint32_t> indices;

	for (int i = 0; i < 6; i++)
	{
		int idx = i * 4;
		indices.push_back(idx);
		indices.push_back(idx + 1);
		indices.push_back(idx + 2);
		ComputeTangent(vertices[idx], vertices[idx + 1], vertices[idx + 2]);

		indices.push_back(idx);
		indices.push_back(idx + 2);
		indices.push_back(idx + 3);
		ComputeTangent(vertices[idx], vertices[idx + 2], vertices[idx + 3]);

	}
	data.Initialize(vertices, indices, texturePath);

	return data;
}
PbrMeshData GeometryGenerator::PbrUseTesslationBox(const float& length, const std::wstring& texturePath)
{
	return PbrUseTesslationBox(length, length, length, texturePath);
}
PbrMeshData GeometryGenerator::PbrUseTesslationBox(const float& x, const float& y, const float& z, const std::wstring& texturePath, const float maxUVX, const float maxUVY, const float maxUVZ)
{
	PbrMeshData data;

	float hX = x;
	float hY = y;
	float hZ = z;

	std::vector<PbrVertex> vertices = {
		// 앞 면
		{Vector3(-hX, -hY, -hZ), Vector3(0.f,0.f,-1.f), Vector2(0.f, maxUVY),Vector3::Zero},
		{Vector3(-hX, hY, -hZ), Vector3(0.f,0.f,-1.f), Vector2(0.f, 0.f),Vector3::Zero},
		{Vector3(hX, hY, -hZ), Vector3(0.f,0.f,-1.f), Vector2(maxUVX, maxUVY),Vector3::Zero},
		{Vector3(hX, -hY, -hZ), Vector3(0.f,0.f,-1.f), Vector2(maxUVX, 0.f),Vector3::Zero},

		{Vector3(hX, -hY, hZ), Vector3(0.f,0.f, 1.f), Vector2(maxUVX, maxUVY),Vector3::Zero},
		{Vector3(hX, hY, hZ), Vector3(0.f,0.f, 1.f), Vector2(maxUVX, 0.f),Vector3::Zero},
		{Vector3(-hX, hY, hZ), Vector3(0.f,0.f, 1.f), Vector2(0.f, 0.f),Vector3::Zero},
		{Vector3(-hX, -hY, hZ), Vector3(0.f,0.f, 1.f), Vector2(0.f, maxUVY),Vector3::Zero},

		{Vector3(-hX, -hY, hZ), Vector3(-1.f,0.f,0.f), Vector2(0.f, maxUVZ),Vector3::Zero},
		{Vector3(-hX, hY, hZ), Vector3(-1.f,0.f, 0.f), Vector2(0.f, 0.f),Vector3::Zero},
		{Vector3(-hX, hY, -hZ), Vector3(-1.f,0.f, 0.f), Vector2(maxUVY, 0.f),Vector3::Zero},
		{Vector3(-hX, -hY, -hZ), Vector3(-1.f,0.f, 0.f), Vector2(maxUVY, maxUVZ),Vector3::Zero},

		{Vector3(hX, -hY, -hZ), Vector3(1.f,0.f,0.f), Vector2(maxUVY, maxUVZ),Vector3::Zero},
		{Vector3(hX, hY, -hZ), Vector3(1.f,0.f,0.f), Vector2(maxUVY, 0.f),Vector3::Zero},
		{Vector3(hX, hY, hZ), Vector3(1.f,0.f,0.f), Vector2(0.f, 0.f),Vector3::Zero},
		{Vector3(hX, -hY, hZ), Vector3(1.f,0.f,0.f), Vector2(0.f, maxUVZ),Vector3::Zero},

		// 윗 면 (xz 평면)
		{Vector3(-hX, hY, -hZ), Vector3(0.f, 1.f, 0.f), Vector2(0.f, maxUVZ),Vector3::Zero},
		{Vector3(-hX, hY, hZ), Vector3(0.f, 1.f, 0.f), Vector2(0.f, 0.f), Vector3::Zero},
		{Vector3(hX, hY, hZ), Vector3(0.f, 1.f, 0.f), Vector2(maxUVX, 0.f),Vector3::Zero},
		{Vector3(hX, hY, -hZ), Vector3(0.f, 1.f, 0.f), Vector2(maxUVX, maxUVZ),Vector3::Zero},

		// 아랫 면
		{Vector3(hX, -hY, -hZ), Vector3(0.f, -1.f, 0.f), Vector2(maxUVX, maxUVZ),Vector3::Zero},
		{Vector3(hX, -hY, hZ), Vector3(0.f, -1.f, 0.f), Vector2(maxUVX, 0.f),Vector3::Zero},
		{Vector3(-hX, -hY, hZ), Vector3(0.f, -1.f, 0.f), Vector2(0.f, 0.f),Vector3::Zero},
		{Vector3(-hX, -hY, -hZ), Vector3(0.f, -1.f, 0.f), Vector2(0.f, maxUVZ),Vector3::Zero}
	};
	std::vector<uint32_t> indices;

	for (int i = 0; i < 6; i++)
	{
		int idx = i * 4;
		indices.push_back(idx + 1);
		indices.push_back(idx + 2);
		indices.push_back(idx);
		indices.push_back(idx + 3);

		ComputeTangent(vertices[idx], vertices[idx + 1], vertices[idx + 2]);
		ComputeTangent(vertices[idx + 1], vertices[idx + 3], vertices[idx + 2]);
	}
	data.Initialize(vertices, indices, texturePath);

	return data;
}

PbrMeshData GeometryGenerator::PbrGrid(const float& xLength, const float& yLength, const int& x, const int& y, const std::wstring& texturePath)
{
	return PbrMeshData();
}

PbrMeshData GeometryGenerator::PbrCyilinder(const float& topRadius, const float& bottomRadius, const float& height, const int& x, const int& y, const std::wstring& texturePath)
{
	return PbrMeshData();
}

PbrMeshData GeometryGenerator::PbrSphere(const float& radius, const int& x, const int& y, const std::wstring& texturePath, float uvDeltaX, float uvDeltaY)
{

	//PbrMeshData data;
	//if (x == 0 || y == 0)
	//{
	//	return data;
	//}
	//Vector3 basePosition(Vector3(0.f, radius, 0.f));


	//float delYTheta = DirectX::XM_2PI / x;
	//float delZTheta = DirectX::XM_PI / y;

	//float uvDelX = 1.f / x * uvDeltaX;
	//float uvDelY = 1.f / y * uvDeltaY;

	//int index = 0;

	//std::vector<PbrVertex> vertices;
	//std::vector<uint32_t> indices;
	//for (int j = 0; j < y + 1; ++j)
	//{
	//	Vector3 position = Vector3::Transform(basePosition, DirectX::XMMatrixRotationZ(-delZTheta * j));
	//	for (int i = 0; i < x + 1; ++i)
	//	{
	//		PbrVertex v;
	//		v.position = Vector3::Transform(position, DirectX::XMMatrixRotationY(-delYTheta * i));
	//		v.texcoord = Vector2(i * uvDelX, j * uvDelY);
	//		v.normal = v.position;
	//		v.normal.Normalize();

	//		vertices.push_back(v);
	//	}
	//}

	//for (int i = 0; i < y; i++)
	//{
	//	int index = i * (x + 1);
	//	for (int j = 0; j < x; j++)
	//	{
	//		int idx = index + j;
	//		indices.push_back(idx + x + 1);
	//		indices.push_back(idx);
	//		indices.push_back(idx + 1);
	//		if (i != 0)
	//			ComputeTangent(vertices[idx + x + 1], vertices[idx], vertices[idx + 1]);
	//		
	//		indices.push_back(idx + x + 1);
	//		indices.push_back(idx + 1);
	//		indices.push_back(idx + x + 2);
	//		if (i != y - 1)
	//			ComputeTangent(vertices[idx + x + 1], vertices[idx + 1], vertices[idx + x + 2]);
	//	}
	//}
	////vertices[0].tangent = Vector3(0.f, 0.f, 1.f);
	////vertices[vertices.size() - 1].tangent = vertices[vertices.size() - 2].tangent;
	///*for (auto& v : vertices) {
	//	std::cout << v.tangent.x << ' '<< v.tangent.y << ' ' << v.tangent.z << '\n';
	//}*/
	//data.Initialize(vertices, indices, texturePath);

	//return data;

	PbrMeshData data;
	if (x == 0 || y == 0)
	{
		return data;
	}
	Vector3 basePosition(Vector3(0.f, radius, 0.f));


	float delYTheta = DirectX::XM_2PI / x;
	float delZTheta = DirectX::XM_PI / y;

	float uvDelX = 1.f / x * uvDeltaX;
	float uvDelY = 1.f / y * uvDeltaY;

	int index = 0;

	std::vector<PbrVertex> vertices;
	std::vector<uint32_t> indices;
	for (int j = 0; j < y + 1; ++j)
	{
		Vector3 position = Vector3::Transform(basePosition, DirectX::XMMatrixRotationZ(-delZTheta * j));
		for (int i = 0; i < x + 1; ++i)
		{
			if (j == 0 || j == y) {
				if (i == x) {
					break;
				}
			}

			PbrVertex v;
			v.position = Vector3::Transform(position, DirectX::XMMatrixRotationY(-delYTheta * i));
			v.texcoord = Vector2(i * uvDelX, j * uvDelY);
			v.normal = v.position;
			v.normal.Normalize();

			vertices.push_back(v);
		}
	}

	for (int i = 0; i < y; i++)
	{
		int index = i * (x + 1) - 1;
		for (int j = 0; j < x; j++)
		{
			int idx = index + j;
			if (i == 0) {
				indices.push_back(idx + x + 1);
				indices.push_back(idx + 1);
				indices.push_back(idx + x + 2);
			}
			else if (i == y - 1) {
				indices.push_back(index + x + 1);
				indices.push_back(idx);
				indices.push_back(idx + 1);
			}
			else {
				indices.push_back(idx + x + 1);
				indices.push_back(idx);
				indices.push_back(idx + 1);

				indices.push_back(idx + x + 1);
				indices.push_back(idx + 1);
				indices.push_back(idx + x + 2);
			}

		}
	}
	for (size_t i = 0; i < indices.size(); i += 3) {
		int idx0 = indices[i];
		int idx1 = indices[i + 1];
		int idx2 = indices[i + 2];

		Renderer::PbrVertex& v0 = vertices[idx0];
		Renderer::PbrVertex& v1 = vertices[idx1];
		Renderer::PbrVertex& v2 = vertices[idx2];

		ComputeTangent(v0, v1, v2);
	}
	//vertices[0].tangent = Vector3(0, 0, 0);

	data.Initialize(vertices, indices, texturePath);

	return data;
}


PbrMeshData GeometryGenerator::PbrUseTesslationSphere(const float& radius, const int& x, const int& y, const std::wstring& texturePath, float uvDeltaX, float uvDeltaY)
{
	PbrMeshData data;
	if (x == 0 || y == 0)
	{
		return data;
	}
	Vector3 basePosition(Vector3(0.f, radius / 2.f, 0.f));


	float delYTheta = DirectX::XM_2PI / x;
	float delZTheta = DirectX::XM_PI / y;

	float uvDelX = 1.f / x * uvDeltaX;
	float uvDelY = 1.f / y * uvDeltaY;

	int index = 0;

	std::vector<PbrVertex> vertices;
	std::vector<uint32_t> indices;
	for (int j = 0; j < y + 1; ++j)
	{
		Vector3 position = Vector3::Transform(basePosition, DirectX::XMMatrixRotationZ(-delZTheta * j));
		for (int i = 0; i < x + 1; ++i)
		{
			PbrVertex v;
			v.position = Vector3::Transform(position, DirectX::XMMatrixRotationY(-delYTheta * i));
			v.texcoord = Vector2(i * uvDelX, j * uvDelY);
			v.normal = v.position;
			v.normal.Normalize();

			vertices.push_back(v);
		}
	}

	for (int i = 0; i < y; i++)
	{
		int index = i * (x + 1);
		for (int j = 0; j < x; j++)
		{
			int idx = index + j;
			indices.push_back(idx);
			indices.push_back(idx + 1);
			indices.push_back(idx + x + 1);
			indices.push_back(idx + x + 2);

			if (i != 0)
				ComputeTangent(vertices[idx + x + 1], vertices[idx], vertices[idx + 1]);

			if (i != y - 1)
				ComputeTangent(vertices[idx + x + 1], vertices[idx + 1], vertices[idx + x + 2]);

		}
	}
	vertices[0].tangent = Vector3(0.f, 0.f, 1.f);
	vertices[vertices.size() - 1].tangent = vertices[vertices.size() - 2].tangent;
	/*for (auto& v : vertices) {
		std::cout << v.tangent.x << ' '<< v.tangent.y << ' ' << v.tangent.z << '\n';
	}*/
	data.Initialize(vertices, indices, texturePath);

	return data;
}


RaytracingMeshData GeometryGenerator::RTTriangle(const float& length, const std::wstring& texturePath)
{
	return RaytracingMeshData();
}

RaytracingMeshData GeometryGenerator::RTRectangle(const float& length, const std::wstring& texturePath)
{
	return RaytracingMeshData();
}

RaytracingMeshData GeometryGenerator::RTBox(const float& length, const std::wstring& texturePath)
{
	return RTBox(length, length, length, texturePath);
}

RaytracingMeshData GeometryGenerator::RTBox(const float& x, const float& y, const float& z, const std::wstring& texturePath)
{
	RaytracingMeshData data;

	float hX = x;
	float hY = y;
	float hZ = z;
	Vector3 uv0 = Vector3(0, 1, 0);
	Vector3 uv1 = Vector3(0, 0, 0);
	Vector3 uv2 = Vector3(1, 0, 0);
	Vector3 uv3 = Vector3(1, 1, 0);
	std::vector<RaytracingVertex> vertices = {
		{Vector3(-hX, -hY, -hZ), Vector3(0.f,0.f,-1.f), uv0, uv0},
		{Vector3(-hX, hY, -hZ), Vector3(0.f,0.f,-1.f), uv1, uv1},
		{Vector3(hX, hY, -hZ), Vector3(0.f,0.f,-1.f), uv2, uv2},
		{Vector3(hX, -hY, -hZ), Vector3(0.f,0.f,-1.f), uv3, uv3},

		{Vector3(hX, -hY, hZ), Vector3(0.f,0.f, 1.f), uv0, uv0},
		{Vector3(hX, hY, hZ), Vector3(0.f,0.f, 1.f), uv1, uv1},
		{Vector3(-hX, hY, hZ), Vector3(0.f,0.f, 1.f), uv2, uv2},
		{Vector3(-hX, -hY, hZ), Vector3(0.f,0.f, 1.f), uv3, uv3},

		{Vector3(-hX, -hY, hZ), Vector3(-1.f,0.f,0.f), uv0, uv0},
		{Vector3(-hX, hY, hZ), Vector3(-1.f,0.f, 0.f), uv1, uv1},
		{Vector3(-hX, hY, -hZ), Vector3(-1.f,0.f, 0.f), uv2, uv2},
		{Vector3(-hX, -hY, -hZ), Vector3(-1.f,0.f, 0.f), uv3, uv3},

		// 우측 면
		{Vector3(hX, -hY, -hZ), Vector3(1.f,0.f,0.f), uv0, uv0},
		{Vector3(hX, hY, -hZ), Vector3(1.f,0.f,0.f), uv1, uv1},
		{Vector3(hX, hY, hZ), Vector3(1.f,0.f,0.f), uv2, uv2},
		{Vector3(hX, -hY, hZ), Vector3(1.f,0.f,0.f), uv3, uv3},

		// 윗 면 (xz 평면)
		{Vector3(-hX, hY, -hZ), Vector3(0.f, 1.f, 0.f), uv0, uv0},
		{Vector3(-hX, hY, hZ), Vector3(0.f, 1.f, 0.f), uv1, uv1},
		{Vector3(hX, hY, hZ), Vector3(0.f, 1.f, 0.f), uv2, uv2},
		{Vector3(hX, hY, -hZ), Vector3(0.f, 1.f, 0.f), uv3, uv3},

		// 아랫 면
		{Vector3(hX, -hY, -hZ), Vector3(0.f, -1.f, 0.f), uv0, uv0},
		{Vector3(hX, -hY, hZ), Vector3(0.f, -1.f, 0.f), uv1, uv1},
		{Vector3(-hX, -hY, hZ), Vector3(0.f, -1.f, 0.f), uv2, uv2},
		{Vector3(-hX, -hY, -hZ), Vector3(0.f, -1.f, 0.f), uv3, uv3}
	};
	std::vector<uint32_t> indices;

	for (int i = 0; i < 6; i++)
	{
		int idx = i * 4;
		indices.push_back(idx);
		indices.push_back(idx + 1);
		indices.push_back(idx + 2);

		indices.push_back(idx);
		indices.push_back(idx + 2);
		indices.push_back(idx + 3);
	}
	data.Initialize(vertices, indices, texturePath);

	return data;
}

RaytracingMeshData GeometryGenerator::RTCubeMapBox(const float& length)
{
	RaytracingMeshData data;

	float l = length;

	std::vector<RaytracingVertex> vertices =
	{
		{Vector3(-l, -l, -l)},{Vector3(-l, l, -l)},{Vector3(l, l, -l)},{Vector3(l, -l, -l)},
		{Vector3(-l, -l, l)},{Vector3(-l, l, l)},{Vector3(l, l, l)},{Vector3(l, -l, l)}
	};
	std::vector<uint32_t> indices = {
		0, 2, 1, 0, 3, 2,
		7, 5, 6, 7, 4, 5,
		4, 1, 5, 4, 0, 1,
		4, 3, 0, 4, 7, 3,
		3, 6, 2, 3, 7, 6,
		1, 6, 5, 1, 2, 6
	};
	data.Initialize(vertices, indices);

	return data;
}

RaytracingMeshData GeometryGenerator::RTGrid(const float& xLength, const float& yLength, const int& x, const int& y, const std::wstring& texturePath)
{
	return RaytracingMeshData();
}

RaytracingMeshData GeometryGenerator::RTCyilinder(const float& topRadius, const float& bottomRadius, const float& height, const int& x, const int& y, const std::wstring& texturePath)
{
	return RaytracingMeshData();
}

RaytracingMeshData GeometryGenerator::RTSphere(const float& radius, const int& x, const int& y, const std::wstring& texturePath)
{
	RaytracingMeshData data;
	if (x == 0 || y == 0)
	{
		return data;
	}
	Vector3 basePosition(Vector3(0.f, radius, 0.f));


	float delYTheta = DirectX::XM_2PI / x;
	float delZTheta = DirectX::XM_PI / y;

	float uvDelX = 1.f / x;
	float uvDelY = 1.f / y;

	int index = 0;

	std::vector<RaytracingVertex> vertices;
	std::vector<uint32_t> indices;
	for (int j = 0; j < y + 1; ++j)
	{
		Vector3 position = Vector3::Transform(basePosition, DirectX::XMMatrixRotationZ(-delZTheta * j));
		for (int i = 0; i < x + 1; ++i)
		{
			RaytracingVertex v;
			v.position = Vector3::Transform(position, DirectX::XMMatrixRotationY(-delYTheta * i));
			Vector3 normal = v.position;
			normal.Normalize();
			v.normal = normal;
			v.texcoord = Vector3(i * uvDelX, j * uvDelY, 0);
			vertices.push_back(v);
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
			if (i != 0)
				ComputeTangent(vertices[idx + x + 1], vertices[idx], vertices[idx + 1]);

			indices.push_back(idx + x + 1);
			indices.push_back(idx + 1);
			indices.push_back(idx + x + 2);
			if (i != y - 1)
				ComputeTangent(vertices[idx + x + 1], vertices[idx + 1], vertices[idx + x + 2]);
		}
	}
	vertices[0].tangent = Vector3(0.f, 0.f, 1.f);
	vertices[vertices.size() - 1].tangent = vertices[vertices.size() - 2].tangent;

	data.Initialize(vertices, indices, texturePath);

	return data;
}

void GeometryGenerator::ComputeTangent(Renderer::PbrVertex& v0, Renderer::PbrVertex& v1, Renderer::PbrVertex& v2) {
	DirectX::SimpleMath::Vector2 t0 = v1.texcoord - v0.texcoord;
	DirectX::SimpleMath::Vector2 t1 = v2.texcoord - v0.texcoord;

	Vector3 e0 = v1.position - v0.position;
	Vector3 e1 = (v2.position - v0.position);

	float a = t0.x;
	float b = t0.y;
	float c = t1.x;
	float d = t1.y;

	float det = a * d - b * c;
	float invDet = 1.f / det;

	Vector3 tangent = invDet * (e0 * d - b * e1);
	tangent.Normalize();

	v0.tangent = tangent;
	v1.tangent = tangent;
	v2.tangent = tangent;

}

void GeometryGenerator::ComputeTangent(RaytracingVertex& v0, RaytracingVertex& v1, RaytracingVertex& v2) {
	using DirectX::SimpleMath::Vector3;

	Vector3 t0 = Vector3(v1.texcoord) - Vector3(v0.texcoord);
	Vector3 t1 = Vector3(v2.texcoord) - Vector3(v0.texcoord);

	Vector3 e0 = Vector3(v1.position) - Vector3(v0.position);
	Vector3 e1 = Vector3(v2.position) - Vector3(v0.position);

	float a = t0.x;
	float b = t0.y;
	float c = t1.x;
	float d = t1.y;

	float det = a * d - b * c;
	float invDet = 1.f / det;

	Vector3 tangent = invDet * (e0 * d - b * e1);
	tangent.Normalize();

	v0.tangent = tangent;
	v1.tangent = tangent;
	v2.tangent = tangent;

}