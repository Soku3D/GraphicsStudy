#include "StaticMesh.h"

Core::StaticMesh::StaticMesh() :
	m_vertexBufferView(D3D12_VERTEX_BUFFER_VIEW()),
	m_indexBufferView(D3D12_INDEX_BUFFER_VIEW()),
	m_pCbvDataBegin(nullptr),
	m_objectConstantData(nullptr)
{
}

void Core::StaticMesh::Render(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat)
{
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);

	if (bUseModelMat) {
		commandList->SetGraphicsRootConstantBufferView(1, m_objectConstantBuffer->GetGPUVirtualAddress());
	}
	commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}

void Core::StaticMesh::Update(const float& deltaTime)
{

	if (m_keys.size() > 0) {
		static float frame = 0.f;
		if (m_keys.size() <= (int)frame)
		{
			if (m_loopAnimation)
			{
				frame = (int)frame % m_keys.size();
			}
			else {
				frame = m_keys.size() - 1;
			}
		}
		m_objectConstantData->Model = m_invertTranspose * m_keys[(int)frame].GetTransform() * m_transformFBXAnimation;
		m_objectConstantData->Model = m_objectConstantData->Model.Transpose();

		CD3DX12_RANGE range(0, 0);
		ThrowIfFailed(m_objectConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCbvDataBegin)));
		memcpy(m_pCbvDataBegin, m_objectConstantData, sizeof(ObjectConstantData));

		if (frame != (m_keys.size() - 1))
		{
			frame += m_secondPerFrames * m_animationSpeed;
		}
	}
}
