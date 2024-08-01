#include "StaticMesh.h"

void Core::StaticMesh::Render(float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);

	commandList->SetGraphicsRootConstantBufferView(1, m_objectConstantBuffer->GetGPUVirtualAddress());

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

