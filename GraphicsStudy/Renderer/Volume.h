#pragma once

#include "Buffer.h"
#include "wrl.h"
#include "d3d12.h"
#include "d3dx12.h"

class Volume {

public:
	Volume() {};
	~Volume() {};

	void Render(float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseConstantBuffer);
	void Dispatch(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
	void RenderBoundingBox(float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commmandList);
	void Update(float& deltaTime);

	void Initialize(const int & width, const int& height, const int& depth, Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
	void CopyDescriptors(Microsoft::WRL::ComPtr<ID3D12Device5>& device);

	D3D12_GPU_DESCRIPTOR_HANDLE GetHandle(int index) const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeap->GetGPUDescriptorHandleForHeapStart(), index, offset); }
	ID3D12DescriptorHeap* GetHeap() const { return mHeap.Get();}

	ID3D12Resource* GetDensityTempResource() const { return mDensityTemp.GetResource(); }
	ID3D12Resource* GetDensityResource() const { return mDensity.GetResource(); }
	ID3D12Resource* GetVelocityTempResource() const { return mVelocityTemp.GetResource(); }
	ID3D12Resource* GetVelocityResource() const { return mVelocity.GetResource(); }
private:
	Core::Texture3D mDensity;
	Core::Texture3D mDensityTemp;
	Core::Texture3D mVelocity;
	Core::Texture3D mVelocityTemp;
	Core::Texture3D mPressure;
	Core::Texture3D mPressureTemp;
	Core::Texture3D mDivergence;
	Core::Texture3D mVorticity;

	UINT offset;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;

};