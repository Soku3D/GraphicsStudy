#pragma once

#include "Constants.h"
#include "Utility.h"
#include "GeometryGenerator.h"

namespace Core {
	class StaticMesh {
	public:

		StaticMesh();
		~StaticMesh() {};

		template <typename Vertex>
		void Initialize(MeshData<Vertex>& meshData,
			Microsoft::WRL::ComPtr<ID3D12Device>& device,
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList,
			const DirectX::SimpleMath::Vector3& modelPosition = DirectX::SimpleMath::Vector3::Zero)
		{
			Renderer::Utility::CreateBuffer(meshData.m_vertices, m_vertexUpload, m_vertexGpu, device, commandList);
			Renderer::Utility::CreateBuffer(meshData.m_indices, m_indexUpload, m_indexGpu, device, commandList);

			m_vertexBufferView.BufferLocation = m_vertexGpu->GetGPUVirtualAddress();
			m_vertexBufferView.SizeInBytes = (UINT)(meshData.m_vertices.size() * sizeof(Vertex));
			m_vertexBufferView.StrideInBytes = sizeof(Vertex);

			m_indexBufferView.BufferLocation = m_indexGpu->GetGPUVirtualAddress();
			m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
			m_indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * meshData.m_indices.size());
			m_objectConstantData = new ObjectConstantData();
			
			m_objectConstantData->Model = DirectX::XMMatrixTranslation(modelPosition.x, modelPosition.y, modelPosition.z);
			m_objectConstantData->Model = m_objectConstantData->Model.Transpose();
			std::vector<ObjectConstantData> constantData = { *m_objectConstantData	};
			Renderer::Utility::CreateUploadBuffer(constantData, m_objectConstantBuffer, device);
			indexCount = (UINT)meshData.m_indices.size();

			CD3DX12_RANGE range(0, 0);
			ThrowIfFailed(m_objectConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCbvDataBegin)));
			memcpy(m_pCbvDataBegin, m_objectConstantData, sizeof(ObjectConstantData));
			m_texturePath = meshData.GetTexturePath();
		}
		void Render(float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
		void Update(float& deltaTime);
		std::wstring GetTexturePath() const { return m_texturePath; }
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