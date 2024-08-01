#pragma once

#include <vector>
#include "Vertex.h"

template<typename Vertex>
class MeshData {
public:
	MeshData() {}
	~MeshData() {}
	void Initialize(std::vector<Vertex>& vertices, std::vector<uint16_t> indices);

public:
	std::vector<Vertex> m_vertices;
	std::vector<uint16_t> m_indices;
};
//typedef MeshData<Renderer::SimpleVertex> SimpleMeshData;
using SimpleMeshData = MeshData<Renderer::SimpleVertex>;
using BasicMeshData = MeshData<Renderer::Vertex>;

class GeomertyGenerator {
public:
	static SimpleMeshData SimpleTriangle(const float& length);
	static SimpleMeshData SimpleRectangle(const float& length);
	static SimpleMeshData SimpleBox(const float& length);
	static BasicMeshData Box(const float& length);
	static BasicMeshData Grid(const float& xLength, const float& yLength, const int& x, const int& y);
	static BasicMeshData Cyilinder(const float& topRadius, const float& bottomRadius, const float& height, const int& x, const int& y);
	static BasicMeshData Sphere(const float& radius, const int& x, const int& y);
};

template<typename Vertex>
inline void MeshData<Vertex>::Initialize(std::vector<Vertex>& vertices, std::vector<uint16_t> indices)
{
	m_vertices = vertices;
	m_indices = indices;
}
