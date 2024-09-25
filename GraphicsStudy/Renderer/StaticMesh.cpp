#include "StaticMesh.h"

Core::StaticMesh::StaticMesh() :
	m_pCbvDataBegin(nullptr),
	m_objectConstantData(nullptr)
{
}

void Core::StaticMesh::Render(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat, int i)
{
	commandList->IASetVertexBuffers(0, 1, &mVertexBufferView[i]);
	commandList->IASetIndexBuffer(&mIndexBufferView[i]);

	if (bUseModelMat)
	{
		commandList->SetGraphicsRootConstantBufferView(1, m_objectConstantBuffer->GetGPUVirtualAddress());
	}
	commandList->DrawIndexedInstanced(mIndexCount[i], 1, 0, 0, 0);
}

void Core::StaticMesh::RenderNormal(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat, int i)
{
	commandList->IASetVertexBuffers(0, 1, &mVertexBufferView[i]);
	commandList->IASetIndexBuffer(&mIndexBufferView[i]);

	if (bUseModelMat)
	{
		commandList->SetGraphicsRootConstantBufferView(1, m_objectConstantBuffer->GetGPUVirtualAddress());
	}
	commandList->DrawInstanced(mVertexCount[i], 1, 0, 0);
}

void Core::StaticMesh::RenderBoundingBox(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	commandList->SetGraphicsRootConstantBufferView(0, m_objectConstantBuffer->GetGPUVirtualAddress());
	commandList->DrawInstanced(1, 1, 0, 0);
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
	memcpy(m_pCbvDataBegin, m_objectConstantData, sizeof(ObjectConstantData));
}

void Core::StaticMesh::SetBoundingBoxHalfLength(const float& halfLength)
{
	SetBoundingBoxHalfLength(halfLength, halfLength, halfLength);
}

void Core::StaticMesh::SetBoundingBoxHalfLength(const float& halfLengthX, const float& halfLengthY, const float& halfLengthZ)
{
	m_objectConstantData->boundingBoxHalfLengthX = halfLengthX;
	m_objectConstantData->boundingBoxHalfLengthY = halfLengthY;
	m_objectConstantData->boundingBoxHalfLengthZ = halfLengthZ;
}

void Core::StaticMesh::UpdateWorldRow(const DirectX::SimpleMath::Matrix& worldRow)
{
	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	Matrix world = worldRow;
	
	m_objectConstantData->Model = worldRow.Transpose();
	world.Translation(Vector3(0.f));
	m_objectConstantData->invTranspose = world.Invert();

	m_primitiveConstantData.invTranspose = world.Invert();
}

void Core::StaticMesh::UpdateMaterial(const Material& material)
{
	m_objectConstantData->Material = material;
}
Material& Core::StaticMesh::GetMaterial() const
{
	return m_objectConstantData->Material;
}
//
//void Core::StaticMesh::UpdateDomain(const float& deltaTime,
//	float gui_edge0,
//	float gui_edge1,
//	float gui_edge2,
//	float gui_edge3,
//	float gui_inside0 ,
//	float gui_inside1)
//{
//	/*static float angle = 0.f;
//
//	float speed =1.f *deltaTime;
//	angle += speed;
//	m_objectConstantData->Model = DirectX::XMMatrixRotationZ(angle);
//	m_objectConstantData->invTranspose = m_objectConstantData->Model.Invert();
//
//	m_objectConstantData->Model = m_objectConstantData->Model.Transpose();
//	*/
//	
//	m_objectConstantData->edge0 = gui_edge0;
//	m_objectConstantData->edge1 = gui_edge1;
//	m_objectConstantData->edge2 = gui_edge2;
//	m_objectConstantData->edge3 = gui_edge3;
//	m_objectConstantData->inside0 = gui_inside0;
//	m_objectConstantData->inside1 = gui_inside1;
//
//	CD3DX12_RANGE range(0, 0);
//	ThrowIfFailed(m_objectConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCbvDataBegin)));
//	memcpy(m_pCbvDataBegin, m_objectConstantData, sizeof(ObjectConstantData));
//
//}
