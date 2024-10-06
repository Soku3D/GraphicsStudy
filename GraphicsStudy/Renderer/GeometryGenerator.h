#pragma once

#include <vector>
#include <string>
#include <tuple>

#include "Vertex.h"
#include "MeshData.h"
#include "AnimationClip.h"
#include "RaytracingHlslCompat.h"
#include "ModelLoader.h"

using RaytracingMeshData = MeshData<RaytracingVertex, uint32_t>;

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

	template<typename Vertex, typename Index>
	static std::tuple<std::vector<MeshData<Vertex, Index>>, Animation::AnimationData> ReadFromFile(std::string filename, bool loadAnimation = false, bool updateTangent = false,
		DirectX::SimpleMath::Matrix tr = DirectX::SimpleMath::Matrix()) {

		using namespace DirectX;
		using DirectX::SimpleMath::Vector3;
		using DirectX::SimpleMath::Vector2;
		using namespace Renderer;

		std::shared_ptr<ModelLoader<Vertex, Index>> modelLoader = std::make_shared<Renderer::ModelLoader<Vertex, Index>>();
		modelLoader->Load<Vertex, Index>(filename, loadAnimation, tr);
		std::vector<MeshData<Vertex,Index>>& meshData = modelLoader->meshes;

		Vector3 vmin(1000, 1000, 1000);
		Vector3 vmax(-1000, -1000, -1000);
		for (auto& mesh : meshData) {
			for (auto& v : mesh.m_vertices) {
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

		for (auto& mesh : meshData) {
			for (auto& v : mesh.m_vertices) {
				v.position = (v.position + translation) * scale;
			}
		}
		modelLoader->m_animeData.defaultTransform =
			DirectX::SimpleMath::Matrix::CreateTranslation(translation) * DirectX::SimpleMath::Matrix::CreateScale(scale);

		if (updateTangent) {
			for (auto& mesh : meshData)
			{
				for (size_t i = 0; i < mesh.m_indices.size(); i += 3)
				{
					ComputeTangent<Vertex>(mesh.m_vertices[mesh.m_indices[i]], mesh.m_vertices[mesh.m_indices[i + 1]], mesh.m_vertices[mesh.m_indices[i + 2]]);
				}
			}
		}
		return { meshData, modelLoader->m_animeData };
	}

	static PbrMeshData PbrTriangle(const float& length, const std::wstring& texturePath = L"");
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

	static RaytracingMeshData RTTriangle(const float& length, const std::wstring& texturePath = L"");
	static RaytracingMeshData RTRectangle(const float& length, const std::wstring& texturePath = L"");
	static RaytracingMeshData RTRectangle(const float& x, const float& y, const std::wstring& texturePath);
	static RaytracingMeshData RTBox(const float& length, const std::wstring& texturePath = L"");
	static RaytracingMeshData RTBox(const float& x, const float& y, const float& z, const std::wstring& texturePath = L"");
	static RaytracingMeshData RTCubeMapBox(const float& length);
	static RaytracingMeshData RTGrid(const float& xLength, const float& yLength, const int& x, const int& y, const std::wstring& texturePath = L"");
	static RaytracingMeshData RTCyilinder(const float& topRadius, const float& bottomRadius, const float& height, const int& x, const int& y, const std::wstring& texturePath = L"");
	static RaytracingMeshData RTSphere(const float& radius, const int& x, const int& y, const std::wstring& texturePath = L"");

	static void ComputeTangent(Renderer::PbrVertex& v0, Renderer::PbrVertex& v1, Renderer::PbrVertex& v2);

	static void ComputeTangent(RaytracingVertex& v0, RaytracingVertex& v1, RaytracingVertex& v2);
	
	template<typename Veretex>
	static void ComputeTangent(Veretex& v0, Veretex& v1, Veretex& v2)
	{
		using DirectX::SimpleMath::Vector3;
		using DirectX::SimpleMath::Vector2;
		using namespace Renderer;

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

};

