#pragma once

#include "Constants.h"
#include "Utility.h"

namespace Core {
	class StaticMesh {
	public:
		template <typename Vertex>
		void Initialize(std::vector<Vertex>& verticies, std::vector<uint16_t>& indices,
			Microsoft::WRL::ComPtr<ID3D12Device>& device,
			Microsoft::WRL::ComPtr<ID3D12CommandList> & commandList) {
			Renderer::Utility::CreateBuffer(verticies, m_vertexUpload, m_vertexGpu,device, commandList);
			Renderer::Utility::CreateBuffer(indices, m_indexUpload, m_indexGpu, device, commandList);

			m_vertexBufferView.BufferLocation = m_vertexGpu->GetGPUVirtualAddress();
			m_vertexBufferView.SizeInBytes = (UINT)(verticies.size() * sizeof(Vertex));
			m_vertexBufferView.StrideInBytes = sizeof(Vertex);

			m_indexBufferView.BufferLocation = m_indexGpu->GetGPUVirtualAddress();
			m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
			m_indexBufferView.SizeInBytes = UINT(sizeof(uint16_t) * indices.size());

			//Renderer::Utility::CreateUploadBuffer()
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_objectConstantBuffer;
		ObjectConstantData m_objectConstantData;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexUploadBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexUpload;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexGpu;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_indexUpload;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_indexGpu;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	};
}