#pragma once

#include <vector>
#include "directxtk/SimpleMath.h"
#include "wrl.h"
#include "d3d12.h"

struct Particle {
	DirectX::SimpleMath::Vector3 mPosition;
	DirectX::SimpleMath::Vector3 mOriginPosition;
	DirectX::SimpleMath::Vector3 mColor;
	DirectX::SimpleMath::Vector2 mVelocity;
	DirectX::SimpleMath::Vector2 mOriginVelocity;
	float mLife;
	float mRadius;
};

class Particles {
public:
	Particles() {};
	~Particles() {};

	void Initialize(int numPatricles);
	void InitializeSPH(int numPatricles);
	void BuildResources(Microsoft::WRL::ComPtr <ID3D12Device5> & device,
		Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList>& commandList);
	void BuildDescriptors(Microsoft::WRL::ComPtr <ID3D12Device5>& device,
		Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList>& commandList);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandle() const {
		return mSrvHeap->GetGPUDescriptorHandleForHeapStart();
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetUavHandle() const {
		return m_uavHeap->GetGPUDescriptorHandleForHeapStart();
	}
	ID3D12DescriptorHeap* GetSrvHeap() const {
		return mSrvHeap.Get();
	}
	ID3D12DescriptorHeap* GetUavHeap() const {
		return m_uavHeap.Get();
	}
	UINT GetParticleCount() const {
		return (UINT)mCpu.size();
	}
	Particle& operator[](int index) {
		return mCpu[index];
	}
	void CopyToCpu();
	void CopyToGpu(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
	ID3D12Resource* GetGpu() const { return mGpu.Get(); }
	ID3D12Resource* GetReadBack() const { return mReadBack.Get(); }
private:
	std::vector<Particle> mCpu;
	Microsoft::WRL::ComPtr<ID3D12Resource> mGpu;
	Microsoft::WRL::ComPtr<ID3D12Resource> mUpload;
	Microsoft::WRL::ComPtr<ID3D12Resource> mReadBack;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_uavHeap;
	void* pGpuData;
};