#pragma once

#include <vector>
#include "directxtk/SimpleMath.h"
#include "wrl.h"
#include "d3d12.h"
#include "d3dx12.h"
#include "Buffer.h"

struct Particle {
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 originPosition;
	DirectX::SimpleMath::Vector3 color;
	DirectX::SimpleMath::Vector3 velocity;
	DirectX::SimpleMath::Vector3 originVelocity;
	DirectX::SimpleMath::Vector3 force;
	float pressure;
	float pressureCoeff;
	float viscosity;
	float density;
	float density0;
	float life;
	float radius;
	float mass;
};

class Particles {
public:
	Particles() {};
	~Particles() {};

	void Initialize(int numPatricles);
	void InitializeSPH(int numPatricles);
	void InitializeCloth(int n);
	void InitializeCloth(int width, int height);
	void BuildResources(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, DXGI_FORMAT format, int width, int height);
	//void BuildResources(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, int width, int height);
	void BuildResources(Microsoft::WRL::ComPtr <ID3D12Device5> & device,
		Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList>& commandList);
	void BuildDescriptors(Microsoft::WRL::ComPtr <ID3D12Device5>& device,
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

	ID3D12DescriptorHeap* GetIntegratedHeap() const {
		return mHeap.Get();
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetIntegratedGpuHandle(int index) const {
		return CD3DX12_GPU_DESCRIPTOR_HANDLE( mHeap->GetGPUDescriptorHandleForHeapStart(), index, offset);
	}
	D3D12_CPU_DESCRIPTOR_HANDLE GetIntegratedCpuHandle(int index) const {
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(mHeap->GetCPUDescriptorHandleForHeapStart(), index, offset);
	}

	UINT GetParticleCount() const {
		return (UINT)mCpu.size();
	}
	Particle& operator[](int index) {
		return mCpu[index];
	}
	ID3D12Resource* GetRandomResource() const { return mRandom.Get(); }
	ID3D12Resource* GetParticleResource() const { return mStructureBuffer.Get(); }
	ID3D12Resource* GetTempParticleResource() const { return  mStructureBufferTemp.Get();
	}

	void CopyToCpu();
	void CopyToGpu(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
	void CreateResource(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, const int& width, const int& height, DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& resource);
	ID3D12Resource* GetGpu() const { return mStructureBuffer.Get(); }
	ID3D12Resource* GetReadBack() const { return mStructureBuffer.GetReadBack(); }
	std::vector<Particle> mCpu;

	int width;
	int height;

private:
	
	//Microsoft::WRL::ComPtr<ID3D12Resource> mGpu;
	//Microsoft::WRL::ComPtr<ID3D12Resource> mUpload;
	//Microsoft::WRL::ComPtr<ID3D12Resource> mReadBack;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;
	UINT offset;
	//void* pGpuData;
	Core::StructureBuffer<Particle> mStructureBuffer;
	Core::StructureBuffer<Particle> mStructureBufferTemp;
	Microsoft::WRL::ComPtr<ID3D12Resource> mRandom;

};