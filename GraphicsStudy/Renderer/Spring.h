#pragma once

#include <vector>
#include "directxtk/SimpleMath.h"
#include "wrl.h"
#include "d3d12.h"
#include "d3dx12.h"
#include "Buffer.h"

struct Spring {
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 velocity;
	DirectX::SimpleMath::Vector3 force;
	float l;
};

class Springs {
public:
	Springs() {};
	~Springs() {};

	void Initialize(int numSprings);
	void BuildResources(Microsoft::WRL::ComPtr <ID3D12Device5>& device,
		Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList>& commandList);

	D3D12_GPU_DESCRIPTOR_HANDLE GetUavHandle() const {
		return mStructureBuffer.GetHandle(0);
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetHandle(int index) const {
		return mStructureBuffer.GetHandle(index);
	}

	ID3D12DescriptorHeap* GetHeap() const {
		return mStructureBuffer.GetHeap();
	}
	UINT GetParticleCount() const {
		return (UINT)mCpu.size();
	}
	ID3D12Resource* GetGpu() const { return mStructureBuffer.Get(); }
	ID3D12Resource* GetReadBack() const { return mStructureBuffer.GetReadBack(); }
	

private:
	std::vector<Spring> mCpu;
	Core::StructureBuffer<Spring> mStructureBuffer;
};