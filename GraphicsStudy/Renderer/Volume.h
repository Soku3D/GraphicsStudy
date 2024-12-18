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
	void DispatchUpScale(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
	void RenderBoundingBox(float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commmandList);
	void Update(float& deltaTime);

	void Initialize(const int& width, const int& height, const int& depth, const int& upScale, Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
	void CopyDescriptors(Microsoft::WRL::ComPtr<ID3D12Device5>& device);

	D3D12_GPU_DESCRIPTOR_HANDLE GetHandle(int index) const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeap->GetGPUDescriptorHandleForHeapStart(), index, offset); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetDownSampleHandle(int index) const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(mDownSampleHeap->GetGPUDescriptorHandleForHeapStart(), index, offset); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetUpSampleHandle(int index) const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(mUpSampleHeap->GetGPUDescriptorHandleForHeapStart(), index, offset); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetAdvectionHandle(int index) const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(mAdvectionHeap->GetGPUDescriptorHandleForHeapStart(), index, offset); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetCPHandle(int i, int index) const {
		if (i == 0)
			return CD3DX12_GPU_DESCRIPTOR_HANDLE(mCPHeap1->GetGPUDescriptorHandleForHeapStart(), index, offset);
		else
			return CD3DX12_GPU_DESCRIPTOR_HANDLE(mCPHeap2->GetGPUDescriptorHandleForHeapStart(), index, offset);
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetVorticityHandle(int i, int index) const {
		if (i == 0)
			return CD3DX12_GPU_DESCRIPTOR_HANDLE(mComputeVorticityHeap->GetGPUDescriptorHandleForHeapStart(), index, offset);
		else
			return CD3DX12_GPU_DESCRIPTOR_HANDLE(mVorticityConfinementHeap->GetGPUDescriptorHandleForHeapStart(), index, offset);
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetDivergenceHandle(int index) const {	return CD3DX12_GPU_DESCRIPTOR_HANDLE(mDivergenceHeap->GetGPUDescriptorHandleForHeapStart(), index, offset);	}
	ID3D12DescriptorHeap* GetHeap() const { return mHeap.Get(); }
	ID3D12DescriptorHeap* GetDivergenceHeap() const { return mDivergenceHeap.Get(); }
	ID3D12DescriptorHeap* GetDownSampleHeap() const { return mDownSampleHeap.Get(); }
	ID3D12DescriptorHeap* GetUpSampleHeap() const { return mUpSampleHeap.Get(); }
	ID3D12DescriptorHeap* GetAdvectionHeap() const { return mAdvectionHeap.Get(); }
	ID3D12DescriptorHeap* GetCPHeap(int index) const { 
		if (index == 0)
			return mCPHeap1.Get();
		else
			return mCPHeap2.Get();
	}
	ID3D12DescriptorHeap* GetVorticityHeap(int index) const {
		if (index == 0)
			return mComputeVorticityHeap.Get();
		else
			return mVorticityConfinementHeap.Get();
	}
	ID3D12Resource* GetDensityTempResource() const { return mDensityTemp.GetResource(); }
	ID3D12Resource* GetDensityResource() const { return mDensity.GetResource(); }
	ID3D12Resource* GetVelocityTempResource() const { return mVelocityTemp.GetResource(); }
	ID3D12Resource* GetVelocityResource() const { return mVelocity.GetResource(); }

	ID3D12Resource* GetDensityUpTempResource() const { return mDensityUpTemp.GetResource(); }
	ID3D12Resource* GetDensityUpResource() const { return mDensityUp.GetResource(); }
	ID3D12Resource* GetVelocityUpTempResource() const { return mVelocityUpTemp.GetResource(); }
	ID3D12Resource* GetVelocityUpResource() const { return mVelocityUp.GetResource(); }

private:
	Core::Texture3D mDensity;
	Core::Texture3D mDensityTemp;
	Core::Texture3D mVelocity;
	Core::Texture3D mVelocityTemp;
	Core::Texture3D mPressure;
	Core::Texture3D mPressureTemp;
	Core::Texture3D mDivergence;
	Core::Texture3D mVorticity;
	Core::Texture3D mBoundaryCondition;

	Core::Texture3D mDensityUp;
	Core::Texture3D mDensityUpTemp;
	Core::Texture3D mVelocityUp;
	Core::Texture3D mVelocityUpTemp;

	UINT offset;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;

	//Compute Pressure Heap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCPHeap1; // pressure uav, pressureTemp srv, divergence srv
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCPHeap2; // pressureTemp uav, pressure srv, divergence srv
	// divergence uav, pressure uav, PressureTemp uav, velocity srv boundaryCondition srv
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDivergenceHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mComputeVorticityHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mVorticityConfinementHeap;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDownSampleHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mUpSampleHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mAdvectionHeap;

};