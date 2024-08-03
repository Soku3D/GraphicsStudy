#pragma once

#include <vector>
#include <string>

#include "Vertex.h"
#include "MeshData.h"

class GeometryGenerator {
public:
	static SimpleMeshData SimpleTriangle(const float& length);
	static SimpleMeshData SimpleRectangle(const float& length);
	static SimpleMeshData SimpleBox(const float& length);
	static BasicMeshData Box(const float& length, const std::wstring& texturePath = L"");
	static BasicMeshData Box(const float& x, const float& y, const float& z, const std::wstring& texturePath);
	static BasicMeshData Grid(const float& xLength, const float& yLength, const int& x, const int& y, const std::wstring& texturePath = L"");
	static BasicMeshData Cyilinder(const float& topRadius, const float& bottomRadius, const float& height, const int& x, const int& y, const std::wstring& texturePath = L"");
	static BasicMeshData Sphere(const float& radius, const int& x, const int& y, const std::wstring& texturePath = L"");
	static std::vector<BasicMeshData> ReadFromFile(std::string filename);
};

