#include "FBX.h"



void Animation::FBX::Initialize(std::vector<BasicMeshData>& meshData, AnimationData& animationData, Microsoft::WRL::ComPtr<ID3D12Device>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList,
	bool loopAnimation, float animationSpeed )
{
	using namespace Core;

	for (auto& mesh : meshData)
	{
		std::shared_ptr<Core::StaticMesh> newMesh = std::make_shared<Core::StaticMesh>();
		newMesh->Initialize(mesh, device, commandList, Vector3(0.f, 1.5f, 0.f));
		newMesh->m_inverseMat = animationData.clips[0].keys[animationData.meshNameToId[newMesh->m_name]][0].GetTransform().Invert();
		m_staticMeshes.push_back(newMesh);
	}

	InitAnimation(animationData,
		animationData.clips[0].ticksPerSec,
		animationSpeed,
		loopAnimation);
}

void Animation::FBX::Render(const float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseModelMat,
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& heap,
	std::map<std::wstring, unsigned int>& textureMap,
	UINT heapSize)
{
	for (auto& mesh : m_staticMeshes) {
		CD3DX12_GPU_DESCRIPTOR_HANDLE handle(heap->GetGPUDescriptorHandleForHeapStart());

		if (textureMap.count(mesh->GetTexturePath()) > 0) {
			handle.Offset(textureMap[mesh->GetTexturePath()], heapSize);
		}
		else {
			handle.Offset(textureMap[L"default.png"], heapSize);
		}
		commandList->SetGraphicsRootDescriptorTable(0, handle);
		mesh->Render(deltaTime, commandList, bUseModelMat);
	}
}

void Animation::FBX::Update(float& deltaTime)
{
	m_animationData.Update((int)m_frame);
	
	for (auto& mesh : m_staticMeshes) {
		mesh->UpdateAnimation(deltaTime, m_animationData);
	}

	
	if (m_lastFrame <= (int)m_frame)
	{
		if (m_loopAnimation)
		{
			m_frame = 0;
		}
		else {
			m_frame = m_lastFrame;
		}
	}
	else {
		m_frame += m_animationSpeed * m_framPerSecond * deltaTime;
	}
	//std::cout << (int)m_frame << ' ' << (int)deltaTime << ' ';
}

void Animation::FBX::InitAnimation(AnimationData& animationData, double& tickPerSecond, double animationSpeed, bool bLoop)
{

	m_animationData = animationData;
	m_framPerSecond = tickPerSecond;
	m_animationSpeed = animationSpeed;
	m_loopAnimation = bLoop;
	m_lastFrame = m_animationData.clips[0].keys[0].size() - 1;

}
