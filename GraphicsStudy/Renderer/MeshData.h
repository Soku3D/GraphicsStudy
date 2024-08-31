#pragma once

#include "Vertex.h"
#include "RaytracingHlslCompat.h"

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
	std::string m_name;
};
//typedef MeshData<Renderer::SimpleVertex> SimpleMeshData;
using SimpleMeshData = MeshData<Renderer::SimpleVertex>;
using BasicMeshData = MeshData<Renderer::Vertex>;
using PbrMeshData = MeshData<Renderer::PbrVertex>;
using RaytracingMeshData = MeshData<RaytracingVertex>;

template<typename Vertex>
inline void MeshData<Vertex>::Initialize(std::vector<Vertex>& vertices, std::vector<uint16_t> indices, const std::wstring& texturePath)
{
	m_vertices = vertices;
	m_indices = indices;
	m_texturePath = texturePath;
}
