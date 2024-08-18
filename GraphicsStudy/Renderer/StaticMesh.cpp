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

void Core::StaticMesh::RenderNormal(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat)
{
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);

	if (bUseModelMat) {
		commandList->SetGraphicsRootConstantBufferView(0, m_objectConstantBuffer->GetGPUVirtualAddress());
	}
	commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}

void Core::StaticMesh::UpdateAnimation(const float& deltaTime, Animation::AnimationData& animationData)
{
	if (animationData.clips[0].keys.size() > 0) {

		m_objectConstantData->Model = m_inverseMat * animationData.Get(animationData.meshNameToId[m_name]) * m_transformFBXAnimation;
		m_objectConstantData->invTranspose = m_objectConstantData->Model.Invert();
		m_objectConstantData->Model = m_objectConstantData->Model.Transpose();

		CD3DX12_RANGE range(0, 0);
		ThrowIfFailed(m_objectConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCbvDataBegin)));
		memcpy(m_pCbvDataBegin, m_objectConstantData, sizeof(ObjectConstantData));

	}
}


void Core::StaticMesh::Update(const float& deltaTime)
{
	/*static float angle = 0.f;
	
	float speed =1.f *deltaTime;
	angle += speed;
	m_objectConstantData->Model = DirectX::XMMatrixRotationZ(angle);
	m_objectConstantData->invTranspose = m_objectConstantData->Model.Invert();

	m_objectConstantData->Model = m_objectConstantData->Model.Transpose();
	*/
	CD3DX12_RANGE range(0, 0);
	ThrowIfFailed(m_objectConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCbvDataBegin)));
	memcpy(m_pCbvDataBegin, m_objectConstantData, sizeof(ObjectConstantData));


}