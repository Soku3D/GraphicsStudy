#pragma once

#include <vector>
#include <string>

#include "Vertex.h"

template<typename Vertex>
class MeshData {
public:
	MeshData() {}
	~MeshData() {}
	void Initialize(std::vector<Vertex>& vertices, std::vector<uint16_t> indice, const std::wstring& texturePath = L"");
	std::wstring GetTexturePath() const { return m_texturePath; }
public:
	std::vector<Vertex> m_vertices;
	std::vector<uint16_t> m_indices;
	std::wstring m_texturePath;
};
//typedef MeshData<Renderer::SimpleVertex> SimpleMeshData;
using SimpleMeshData = MeshData<Renderer::SimpleVertex>;
using BasicMeshData = MeshData<Renderer::Vertex>;

class GeomertyGenerator {
public:
	static SimpleMeshData SimpleTriangle(const float& length);
	static SimpleMeshData SimpleRectangle(const float& length);
	static SimpleMeshData SimpleBox(const float& length);
	static BasicMeshData Box(const float& length, const std::wstring& texturePath = L"");
	static BasicMeshData Box(const float& x, const float& y, const float& z, const std::wstring& texturePath);
	static BasicMeshData Grid(const float& xLength, const float& yLength, const int& x, const int& y, const std::wstring& texturePath = L"");
	static BasicMeshData Cyilinder(const float& topRadius, const float& bottomRadius, const float& height, const int& x, const int& y, const std::wstring& texturePath = L"");
	static BasicMeshData Sphere(const float& radius, const int& x, const int& y, const std::wstring& texturePath = L"");
};

template<typename Vertex>
inline void MeshData<Vertex>::Initialize(std::vector<Vertex>& vertices, std::vector<uint16_t> indices, const std::wstring& texturePath)
{
	m_vertices = vertices;
	m_indices = indices;
	m_texturePath = texturePath;
}
