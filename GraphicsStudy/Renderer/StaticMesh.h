#pragma once

#include "Constants.h"
#include "Utility.h"
#include "GeometryGenerator.h"

namespace Core {
	class StaticMesh {
	public:

		StaticMesh();
		~StaticMesh() {
			m_objectConstantData = nullptr;
			m_objectConstantBuffer.Reset();
			m_pCbvDataBegin = nullptr;
			m_vertexUploadBuffer.Reset();
			m_vertexUpload.Reset();
			m_vertexGpu.Reset();
			m_indexUpload.Reset();
			m_indexGpu.Reset();
		};
		std::vector<Animation::AnimationClip::Key> m_keys;
		std::string m_name;
		double m_secondPerFrames = 0.f;
		double m_animationSpeed = 1.f;
		DirectX::SimpleMath::Matrix m_inverseMat = DirectX::SimpleMath::Matrix();
		DirectX::SimpleMath::Matrix m_transformFBXAnimation = DirectX::SimpleMath::Matrix();
		bool m_loopAnimation = false;
		float frame = 0.f;

		template <typename Vertex>
		void Initialize(MeshData<Vertex>& meshData,
			Microsoft::WRL::ComPtr<ID3D12Device>& device,
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList,
			const DirectX::SimpleMath::Vector3& modelPosition = DirectX::SimpleMath::Vector3::Zero,
			Material& material = Material(),
			bool bUseAoMap = false,
			bool bUseHeightMap = false,
			bool bUseMetalnessMap = false,
			bool bUseNormalMap = false,
			bool bUseRoughnessMap = false, 
			bool bUseTesslation = false)
		{
			m_name = meshData.m_name;


			Renderer::Utility::CreateBuffer(meshData.m_vertices, m_vertexUpload, m_vertexGpu, device, commandList);
			Renderer::Utility::CreateBuffer(meshData.m_indices, m_indexUpload, m_indexGpu, device, commandList);

			m_vertexBufferView.BufferLocation = m_vertexGpu->GetGPUVirtualAddress();
			m_vertexBufferView.SizeInBytes = (UINT)(meshData.m_vertices.size() * sizeof(Vertex));
			m_vertexBufferView.StrideInBytes = sizeof(Vertex);

			m_indexBufferView.BufferLocation = m_indexGpu->GetGPUVirtualAddress();
			m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
			m_indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * meshData.m_indices.size());
			m_objectConstantData = new ObjectConstantData();

			m_objectConstantData->Material = material;
			m_objectConstantData->bUseAoMap = bUseAoMap;
			m_objectConstantData->bUseHeightMap = bUseHeightMap;
			m_objectConstantData->bUseMetalnessMap = bUseMetalnessMap;
			m_objectConstantData->bUseNormalMap = bUseNormalMap;
			m_objectConstantData->bUseRoughnessMap = bUseRoughnessMap;
			m_objectConstantData->bUseTesslation = bUseTesslation;

			m_objectConstantData->Model = DirectX::XMMatrixTranslation(modelPosition.x, modelPosition.y, modelPosition.z);
			m_transformFBXAnimation = m_objectConstantData->Model;
			m_objectConstantData->invTranspose = m_objectConstantData->Model.Invert();
			m_objectConstantData->Model = m_objectConstantData->Model.Transpose();

			std::vector<ObjectConstantData> constantData = { *m_objectConstantData };
			Renderer::Utility::CreateUploadBuffer(constantData, m_objectConstantBuffer, device);
			indexCount = (UINT)meshData.m_indices.size();

			CD3DX12_RANGE range(0, 0);
			ThrowIfFailed(m_objectConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCbvDataBegin)));
			memcpy(m_pCbvDataBegin, m_objectConstantData, sizeof(ObjectConstantData));
			m_texturePath = meshData.GetTexturePath();
		}

		void Render(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat = true);
		void RenderNormal(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat);
		void UpdateAnimation(const float& deltaTime, Animation::AnimationData& animationData);
		void Update(const float& deltaTime);

		//void UpdateDomain(const float& deltaTime, float gui_edge0, float gui_edge1, float gui_edge2, float gui_edge3, float gui_inside0, float gui_inside1);

		std::wstring GetTexturePath() const { return m_texturePath; }
		void SetTexturePath(std::wstring path) { m_texturePath = path; }
		void UpdateWorldRow(const DirectX::SimpleMath::Matrix& worldRow);
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_objectConstantBuffer;
		ObjectConstantData* m_objectConstantData;
		UINT8* m_pCbvDataBegin;
		float m_delTeta = 1.f / (3.141592f * 2.f);
		float m_currTheta = 0.f;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexUploadBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexUpload;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexGpu;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_indexUpload;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_indexGpu;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
		UINT indexCount = 0;
		std::wstring m_texturePath = L"";

	};
}