#pragma once

#include "wrl.h"
#include "d3d12.h"
#include "StaticMesh.h"
#include "d3dx12.h"
#include "Utility.h"

namespace Core {

	template<typename ConstantStructure>
	class ConstantBuffer 
	{
	public:
		ConstantBuffer();
		
		void Initialize(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
		ID3D12Resource** GetAddressOf() { return mSimuationConstantBuffer.ReleaseAndGetAddressOf(); }
		ID3D12Resource* Get() { return mSimuationConstantBuffer.Get(); }
		void** GetData() { return &pSimulationConstant; }
		void UpdateBuffer() 
		{
			memcpy(pSimulationConstant, &mStructure, sizeof(ConstantStructure));
		}
		D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress() { return mSimuationConstantBuffer->GetGPUVirtualAddress(); }
		ConstantStructure mStructure;
	private:
		UINT bufferSize;
		
		void* pSimulationConstant;
		Microsoft::WRL::ComPtr<ID3D12Resource> mSimuationConstantBuffer;
	};

	template<typename ConstantStructure>
	inline ConstantBuffer<ConstantStructure>::ConstantBuffer()
	{

	}

	template<typename ConstantStructure>
	inline void ConstantBuffer<ConstantStructure>::Initialize(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
	{
		ThrowIfFailed(
			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantStructure)),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(mSimuationConstantBuffer.GetAddressOf())));

		CD3DX12_RANGE range(0, 0);
		ThrowIfFailed(mSimuationConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&pSimulationConstant)));
	}

	class Texture3D {
	public:
		Texture3D() {};
		~Texture3D() {};

		void Initiailize(UINT width, UINT height, UINT depth, DXGI_FORMAT format, Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commmandList);

		ID3D12DescriptorHeap* GetTextureHeap() const { return mVolumeTextureHeap.Get(); }
		D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandle() const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(mVolumeTextureHeap->GetGPUDescriptorHandleForHeapStart(), 1, offset); }
		D3D12_GPU_DESCRIPTOR_HANDLE GetUavHandle() const { return mVolumeTextureHeap->GetGPUDescriptorHandleForHeapStart(); }
		UINT GetWidth() const { return volumeWidth; }
		UINT GetHeight() const { return volumeHeight; }
		UINT GetDepth() const { return volumeDepth; }

		void Render(float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commmandList, bool bUseConstantBuffer);

	private:
		std::shared_ptr<Core::StaticMesh> mVolumeMesh;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mVolumeTextureHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource> mVolumeTexture;
		DXGI_FORMAT mVolumeFormat;
		UINT volumeWidth;
		UINT volumeHeight;
		UINT volumeDepth;
		UINT offset;
	};

}