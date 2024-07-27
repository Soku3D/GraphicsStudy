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

class GeomertyGenerator {
public:
	static SimpleMeshData SimpleTriangle(const float& length);
};

template<typename Vertex>
inline void MeshData<Vertex>::Initialize(std::vector<Vertex>& vertices, std::vector<uint16_t> indices)
{
	m_vertices = vertices;
	m_indices = indices;
}
