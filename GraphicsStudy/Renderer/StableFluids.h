#pragma once

#include <vector>
#include "directxtk/SimpleMath.h"
#include "wrl.h"
#include "d3d12.h"

class StableFluids {
public:
	StableFluids() {};
	~StableFluids() {};

	void Initialize();
	void Update(float& deltaTime);
	void BuildResources(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, const int& width, const int& height, DXGI_FORMAT& format);

	void CreateResource(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, const int& width, const int& height, DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& resource);
	
	ID3D12DescriptorHeap* GetUavHeap()const { return mHeap.Get(); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetUavHandle() const { return mHeap->GetGPUDescriptorHandleForHeapStart(); }
	ID3D12Resource* GetDensityResource() const { return mDensity.Get(); }
	ID3D12Resource* GetVelocityResource() const { return mVelocity.Get(); }
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> mDensity;
	Microsoft::WRL::ComPtr<ID3D12Resource> mVelocity;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;
};