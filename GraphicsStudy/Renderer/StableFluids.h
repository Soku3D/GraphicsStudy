#pragma once

#include <vector>
#include "directxtk/SimpleMath.h"
#include "wrl.h"
#include "d3d12.h"
#include "d3dx12.h"

class StableFluids {
public:
	StableFluids() {};
	~StableFluids() {};

	void Initialize();
	void Update(float& deltaTime);
	void BuildResources(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, const int& width, const int& height, DXGI_FORMAT& format);

	void CreateResource(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, const int& width, const int& height, DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& resource);

	ID3D12DescriptorHeap* GetHeap()const { return mHeap.Get(); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetUavHandle() const { return mHeap->GetGPUDescriptorHandleForHeapStart(); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetTempSrvHandle() const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeap->GetGPUDescriptorHandleForHeapStart(), 4, offset); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetDensitySrvHandle() const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeap->GetGPUDescriptorHandleForHeapStart(), 2, offset); }
	ID3D12Resource* GetDensityResource() const { return mDensity.Get(); }
	ID3D12Resource* GetDensityTempResource() const { return mDensityTemp.Get(); }
	ID3D12Resource* GetVelocityResource() const { return mVelocity.Get(); }
	ID3D12Resource* GetVelocityTempResource() const { return mVelocityTemp.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> mDensity;
	Microsoft::WRL::ComPtr<ID3D12Resource> mDensityTemp;
	Microsoft::WRL::ComPtr<ID3D12Resource> mVelocity;
	Microsoft::WRL::ComPtr<ID3D12Resource> mVelocityTemp;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;
	UINT offset;
};