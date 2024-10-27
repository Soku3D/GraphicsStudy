#include "Spring.h"

void Springs::Initialize(int numSprings)
{
	mCpu.resize(numSprings);
	XMFLOAT3 pos = XMFLOAT3(-1, 0, 0);
	for (int i = 0; i < numSprings; i++)
	{
		Spring spring;
		spring.position = pos;
		spring.l = 0.1f;
		float dx = 2.f / numSprings;
		
		pos.x += dx;
		mCpu[i] = spring;
	}
}

void Springs::BuildResources(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	mStructureBuffer.Initialize(device, commandList, mCpu);
}
