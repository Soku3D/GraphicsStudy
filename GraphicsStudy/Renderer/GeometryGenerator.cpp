#include "GeometryGenerator.h"
#include "directxtk/SimpleMath.h"
#include "ModelLoader.h"


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

	return data;
}

SimpleMeshData GeometryGenerator::SimpleRectangle(const float& length)
{
	SimpleMeshData data;

	float l = length / 2.f;
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

	float l = length / 2.f;
 
	std::vector<Renderer::SimpleVertex> vertices = {
		{Vector3(-l, -l, -l)},{Vector3(-l, l, -l)},{Vector3(l, l, -l)},{Vector3(l, -l, -l)},
		{Vector3(-l, -l, l)},{Vector3(-l, l, l)},{Vector3(l, l, l)},{Vector3(l, -l, l)}
	};
	std::vector<uint32_t> indices = {
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


SimpleMeshData GeometryGenerator::SimpleCubeMapBox(const float& length)
{
	SimpleMeshData data;

	float l = length / 2.f;

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
		{Vector3(-hX, hY, -hZ), Vector3(0.f, 1.f, 0.f), Vector2(0.f, z)},
		{Vector3(-hX, hY, hZ), Vector3(0.f, 1.f, 0.f), Vector2(0.f, 0.f)},
		{Vector3(hX, hY, hZ), Vector3(0.f, 1.f, 0.f), Vector2(x, 0.f)},
		{Vector3(hX, hY, -hZ), Vector3(0.f, 1.f, 0.f), Vector2(x, z)},

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
	data.Initialize(vertices, indices, texturePath);

	return data;
}

BasicMeshData GeometryGenerator::Cyilinder(const float& topRadius, const float& bottomRadius, const float& height,  const int& x, const int& y, const std::wstring& texturePath) {
	
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
		Vector3 position = basePosition - Vector3(j*delX, j * delY, 0.f);
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
						   
std::tuple<std::vector<BasicMeshData>, Animation::AnimationData> GeometryGenerator::ReadFromFile(std::string filename, bool loadAnimation) {

    using namespace DirectX;

    ModelLoader modelLoader;
    modelLoader.Load(filename, loadAnimation);
    std::vector<BasicMeshData> &meshes = modelLoader.meshes;
	   
    /*Vector3 vmin(1000, 1000, 1000);
    Vector3 vmax(-1000, -1000, -1000);
    for (auto &mesh : meshes) {
        for (auto &v : mesh.m_vertices) {
            vmin.x = XMMin(vmin.x, v.position.x);
            vmin.y = XMMin(vmin.y, v.position.y);
            vmin.z = XMMin(vmin.z, v.position.z);
            vmax.x = XMMax(vmax.x, v.position.x);
            vmax.y = XMMax(vmax.y, v.position.y);
            vmax.z = XMMax(vmax.z, v.position.z);
        }
    }

    float dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
	float scale = 1.f / XMMax(XMMax(dx, dy), dz);
    
	Vector3 translation = -(vmin + vmax) * 0.5f;

	for (auto& mesh : meshes) {
		for (auto& v : mesh.m_vertices) {
			v.position = (v.position + translation) * scale;
		}
	}
	modelLoader.m_animeData.defaultTransform =
		DirectX::SimpleMath::Matrix::CreateTranslation(translation) * DirectX::SimpleMath::Matrix::CreateScale(scale);*/
	return { meshes, modelLoader.m_animeData };
}

