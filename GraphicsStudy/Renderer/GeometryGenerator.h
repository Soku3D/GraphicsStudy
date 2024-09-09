#pragma once

#include <vector>
#include <string>
#include <tuple>

#include "Vertex.h"
#include "MeshData.h"
#include "AnimationClip.h"
#include "RaytracingHlslCompat.h"

using RaytracingMeshData = MeshData<RaytracingVertex, uint16_t>;

class GeometryGenerator {
public:
	static SimpleMeshData SimpleTriangle(const float& length);
	static SimpleMeshData SimpleRectangle(const float& length);
	static SimpleMeshData SimpleBox(const float& length);
	static SimpleMeshData SimpleCubeMapBox(const float& length);
	static BasicMeshData Rectangle(const float& length, const std::wstring& texturePath = L"");
	static BasicMeshData Box(const float& length, const std::wstring& texturePath = L"");
	static BasicMeshData Box(const float& x, const float& y, const float& z, const std::wstring& texturePath);
	static BasicMeshData Grid(const float& xLength, const float& yLength, const int& x, const int& y, const std::wstring& texturePath = L"");
	static BasicMeshData Cyilinder(const float& topRadius, const float& bottomRadius, const float& height, const int& x, const int& y, const std::wstring& texturePath = L"");
	static BasicMeshData Sphere(const float& radius, const int& x, const int& y, const std::wstring& texturePath = L"");
	static std::tuple<std::vector<BasicMeshData>, Animation::AnimationData> ReadFromFile(std::string filename, bool loadAnimation = false);

	static PbrMeshData PbrRectangle(const float& length, const std::wstring& texturePath = L"");
	static PbrMeshData PbrBox(const float& length, const std::wstring& texturePath = L"");
	static PbrMeshData PbrBox(const float& x, const float& y, const float& z, const std::wstring& texturePath
				, const float& UVx = 1.f, const float& UVy = 1.f, const float& UVz = 1.f );
	static PbrMeshData PbrUseTesslationBox(const float& length, const std::wstring& texturePath = L"");
	static PbrMeshData PbrUseTesslationBox(const float& x, const float& y, const float& z, const std::wstring& texturePath, const float maxUVX = 1.f, const float maxUVY = 1.f, const float maxUVZ = 1.f);
	static PbrMeshData PbrGrid(const float& xLength, const float& yLength, const int& x, const int& y, const std::wstring& texturePath = L"");
	static PbrMeshData PbrCyilinder(const float& topRadius, const float& bottomRadius, const float& height, const int& x, const int& y, const std::wstring& texturePath = L"");
	static PbrMeshData PbrSphere(const float& radius, const int& x, const int& y, const std::wstring& texturePath, 
		float uvDeltaX = 1.f, float uvDeltaY = 1.f);
	static PbrMeshData PbrUseTesslationSphere(const float& radius, const int& x, const int& y, const std::wstring& texturePath, float uvDeltaX, float uvDeltaY);

	static std::tuple<std::vector<PbrMeshData>, Animation::AnimationData> ReadFromFile_Pbr(std::string filename, bool loadAnimation, bool updateTangent = false);
	static std::tuple<std::vector<PbrMeshData32>, Animation::AnimationData> ReadFromFile_Pbr32(std::string filename, bool loadAnimation, bool updateTangent = false);

	static RaytracingMeshData RTTriangle(const float& length, const std::wstring& texturePath = L"");
	static RaytracingMeshData RTRectangle(const float& length, const std::wstring& texturePath = L"");
	static RaytracingMeshData RTBox(const float& length, const std::wstring& texturePath = L"");
	static RaytracingMeshData RTBox(const float& x, const float& y, const float& z, const std::wstring& texturePath = L"");
	static RaytracingMeshData RTCubeMapBox(const float& length);
	static RaytracingMeshData RTGrid(const float& xLength, const float& yLength, const int& x, const int& y, const std::wstring& texturePath = L"");
	static RaytracingMeshData RTCyilinder(const float& topRadius, const float& bottomRadius, const float& height, const int& x, const int& y, const std::wstring& texturePath = L"");
	static RaytracingMeshData RTSphere(const float& radius, const int& x, const int& y, const std::wstring& texturePath = L"");

	static void ComputeTangent(Renderer::PbrVertex& v0, Renderer::PbrVertex& v1, Renderer::PbrVertex& v2);

	static void ComputeTangent(RaytracingVertex& v0, RaytracingVertex& v1, RaytracingVertex& v2);

};

