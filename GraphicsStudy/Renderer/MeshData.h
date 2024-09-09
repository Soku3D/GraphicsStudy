#pragma once

#include "Vertex.h"


template<typename Vertex, typename Index>
class MeshData {
public:
	MeshData() {}
	~MeshData() {}
	void Initialize(std::vector<Vertex>& vertices, std::vector<Index> indice, const std::wstring& texturePath = L"");
	std::wstring GetTexturePath() const { return m_texturePath; }
public:
	std::vector<Vertex> m_vertices;
	std::vector<Index> m_indices;
	std::wstring m_texturePath;
	std::string m_name;
};
//typedef MeshData<Renderer::SimpleVertex> SimpleMeshData;
using SimpleMeshData = MeshData<Renderer::SimpleVertex, uint16_t>;
using BasicMeshData = MeshData<Renderer::Vertex, uint16_t>;
using PbrMeshData = MeshData<Renderer::PbrVertex, uint16_t>;
using PbrMeshData32 = MeshData<Renderer::PbrVertex, uint32_t>;


template<typename Vertex, typename Index>
inline void MeshData<Vertex, Index>::Initialize(std::vector<Vertex>& vertices, std::vector<Index> indices, const std::wstring& texturePath)
{
	m_vertices = vertices;
	m_indices = indices;
	m_texturePath = texturePath;
}
