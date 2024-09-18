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

	void DescriptorInsert(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& destHeap, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srcHeap, const int& srcIndex, const int& destIndex, const int& count);

	ID3D12DescriptorHeap* GetHeap()const { return mHeap.Get(); }
	ID3D12DescriptorHeap* GetPHeap()const { return mHeapP.Get(); }
	ID3D12DescriptorHeap* GetPTHeap()const { return mHeapPT.Get(); }

	ID3D12Resource* GetDensityResource() const { return mDensity.Get(); }
	ID3D12Resource* GetDensityTempResource() const { return mDensityTemp.Get(); }
	ID3D12Resource* GetVelocityResource() const { return mVelocity.Get(); }
	ID3D12Resource* GetVelocityTempResource() const { return mVelocityTemp.Get(); }

	D3D12_GPU_DESCRIPTOR_HANDLE GetHandle(const int& index = 0) const {
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeap->GetGPUDescriptorHandleForHeapStart(),
			index,
			offset);
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetPTHandle(const int& index = 0) const {
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeapPT->GetGPUDescriptorHandleForHeapStart(),
			index,
			offset);
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetPHandle(const int& index = 0) const {
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeapP->GetGPUDescriptorHandleForHeapStart(),
			index,
			offset);
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> mDensity;
	Microsoft::WRL::ComPtr<ID3D12Resource> mDensityTemp;
	Microsoft::WRL::ComPtr<ID3D12Resource> mVelocity;
	Microsoft::WRL::ComPtr<ID3D12Resource> mVelocityTemp;
	Microsoft::WRL::ComPtr<ID3D12Resource> mPressure;
	Microsoft::WRL::ComPtr<ID3D12Resource> mPressureTemp;
	Microsoft::WRL::ComPtr<ID3D12Resource> mDivergence;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeapNSV;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeapPT;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeapP;

	UINT offset;
};