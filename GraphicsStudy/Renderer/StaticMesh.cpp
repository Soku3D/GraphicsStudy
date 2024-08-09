#include "StaticMesh.h"

Core::StaticMesh::StaticMesh():
	m_vertexBufferView(D3D12_VERTEX_BUFFER_VIEW()),
	m_indexBufferView(D3D12_INDEX_BUFFER_VIEW()),
	m_pCbvDataBegin(nullptr),
	m_objectConstantData(nullptr)
{
}

void Core::StaticMesh::Render(float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bIsCubeMap)
{
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);
	
	if (!bIsCubeMap) {
		commandList->SetGraphicsRootConstantBufferView(1, m_objectConstantBuffer->GetGPUVirtualAddress());
	}
	commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}

void Core::StaticMesh::Update(float& deltaTime)
{
	m_currTheta += deltaTime * m_delTeta;
	m_objectConstantData->Model = DirectX::SimpleMath::Matrix::CreateRotationZ(m_currTheta);
	m_objectConstantData->Model = m_objectConstantData->Model.Transpose();

	CD3DX12_RANGE range(0, 0);
	ThrowIfFailed(m_objectConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCbvDataBegin)));
	memcpy(m_pCbvDataBegin, m_objectConstantData, sizeof(ObjectConstantData));
}

