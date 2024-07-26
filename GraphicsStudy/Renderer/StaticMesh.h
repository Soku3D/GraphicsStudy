#pragma once

#include "Constants.h"
#include "Utility.h"

namespace Core {
	class StaticMesh {
	public:

		StaticMesh() {};
		~StaticMesh() {};

		template <typename Vertex>
		void Initialize(std::vector<Vertex>& verticies, std::vector<uint16_t>& indices,
			Microsoft::WRL::ComPtr<ID3D12Device>& device,
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList) {
			Renderer::Utility::CreateBuffer(verticies, m_vertexUpload, m_vertexGpu, device, commandList);
			Renderer::Utility::CreateBuffer(indices, m_indexUpload, m_indexGpu, device, commandList);

			m_vertexBufferView.BufferLocation = m_vertexGpu->GetGPUVirtualAddress();
			m_vertexBufferView.SizeInBytes = (UINT)(verticies.size() * sizeof(Vertex));
			m_vertexBufferView.StrideInBytes = sizeof(Vertex);

			m_indexBufferView.BufferLocation = m_indexGpu->GetGPUVirtualAddress();
			m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
			m_indexBufferView.SizeInBytes = UINT(sizeof(uint16_t) * indices.size());
			m_objectConstantData = new ObjectConstantData();
			//m_objectConstantData->Model = DirectX::SimpleMath::Matrix::CreateRotationZ(3.141592/2.f);
			m_objectConstantData->Model = m_objectConstantData->Model.Transpose();
			std::vector<ObjectConstantData> constantData = {*m_objectConstantData	};
			Renderer::Utility::CreateUploadBuffer(constantData, m_objectConstantBuffer, device);
			indexCount = indices.size();

			CD3DX12_RANGE range(0, 0);
			ThrowIfFailed(m_objectConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCbvDataBegin)));
			memcpy(m_pCbvDataBegin, m_objectConstantData, sizeof(ObjectConstantData));
		}
		void Render(float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
		void Update(float& deltaTime);
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
	};
}