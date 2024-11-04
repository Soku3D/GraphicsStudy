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
		~ConstantBuffer();
		
		void Initialize(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
		ID3D12Resource** GetAddressOf() { return mSimuationConstantBuffer.ReleaseAndGetAddressOf(); }
		ID3D12Resource* Get() { return mSimuationConstantBuffer.Get(); }
		void** GetData() { return &pSimulationConstant; }
		void UpdateBuffer() 
		{
			memcpy(pSimulationConstant, &mStructure, sizeof(ConstantStructure));
		}
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() { return mSimuationConstantBuffer->GetGPUVirtualAddress(); }
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
	inline ConstantBuffer<ConstantStructure>::~ConstantBuffer()
	{
		pSimulationConstant = nullptr;
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
				IID_PPV_ARGS(mSimuationConstantBuffer.ReleaseAndGetAddressOf())));

		CD3DX12_RANGE range(0, 0);
		ThrowIfFailed(mSimuationConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&pSimulationConstant)));
	}

	class Texture2D {
	public:
		Texture2D() {};
		~Texture2D() {};

		void Initiailize(UINT width, UINT height, UINT depth, DXGI_FORMAT format, Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commmandList);

	private:
		UINT mWidth;
		UINT mHeight;

	};

	class Texture3D {
	public:
		Texture3D() {};
		~Texture3D() {};

		void Initiailize(UINT width, UINT height, UINT depth, DXGI_FORMAT format, Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commmandList);

		void InitVolumeMesh(const float& halfLength, Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);

		void InitVolumeMesh(const float& x, const float& y, const float& z, Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);

		void Update(float& deltaTime);

		ID3D12DescriptorHeap* GetTextureHeap() const { return mVolumeTextureHeap.Get(); }
		ID3D12Resource* GetResource() const { return mVolumeTexture.Get(); }

		D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGPUHandle() const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(mVolumeTextureHeap->GetGPUDescriptorHandleForHeapStart(), 1, offset); }
		D3D12_GPU_DESCRIPTOR_HANDLE GetUavGPUHandle() const { return mVolumeTextureHeap->GetGPUDescriptorHandleForHeapStart(); }

		D3D12_CPU_DESCRIPTOR_HANDLE GetNSVUavCPUHandle() const { return mVolumeTextureHeapNSV->GetCPUDescriptorHandleForHeapStart(); }
		D3D12_CPU_DESCRIPTOR_HANDLE GetNSVSrvCPUHandle() const { return CD3DX12_CPU_DESCRIPTOR_HANDLE(mVolumeTextureHeapNSV->GetCPUDescriptorHandleForHeapStart(), 1, offset); }

		UINT GetWidth() const { return volumeWidth; }
		UINT GetHeight() const { return volumeHeight; }
		UINT GetDepth() const { return volumeDepth; }

		void Render(float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commmandList, bool bUseConstantBuffer);

		void RenderBoundingBox(float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commmandList);

	private:
		std::shared_ptr<Core::StaticMesh> mVolumeMesh;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mVolumeTextureHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mVolumeTextureHeapNSV;
		Microsoft::WRL::ComPtr<ID3D12Resource> mVolumeTexture;
		DXGI_FORMAT mVolumeFormat;
		UINT volumeWidth;
		UINT volumeHeight;
		UINT volumeDepth;
		UINT offset;
	};

	template<typename Structure>
	class StructureBuffer {
	public:
		StructureBuffer() {}
		~StructureBuffer() {}

		void Initialize(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, std::vector<Structure>& cpuData);
		ID3D12Resource** GetAddressOf() const { return mStructureBuffer.GetAddressOf(); }
		ID3D12Resource* Get() const { return mStructureBuffer.Get(); }
		ID3D12Resource* GetReadBack() const { return mReadBack.Get(); }
		ID3D12DescriptorHeap* GetHeap() const { return mHeap.Get(); }
		// 0 : uav , 1 : srv
		D3D12_GPU_DESCRIPTOR_HANDLE GetHandle(int index) const {
			return	CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeap->GetGPUDescriptorHandleForHeapStart(), index, offset);
		}
		// 0 : uav , 1 : srv
		D3D12_GPU_DESCRIPTOR_HANDLE GetNSVGPUHandle(int index = 0) const {
			return	CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeapNSV->GetGPUDescriptorHandleForHeapStart(), index, offset);
		}
		// 0 : uav , 1 : srv
		D3D12_CPU_DESCRIPTOR_HANDLE GetNSVCPUHandle(int index = 0) const {
			return	CD3DX12_CPU_DESCRIPTOR_HANDLE(mHeapNSV->GetCPUDescriptorHandleForHeapStart(), index, offset);
		}
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() { return mStructureBuffer->GetGPUVirtualAddress(); }

	private:
		UINT bufferSize;
		UINT offset;
		void* pGpuData;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeapNSV;

		Microsoft::WRL::ComPtr<ID3D12Resource> mStructureBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> mUpload;
		Microsoft::WRL::ComPtr<ID3D12Resource> mReadBack;
	};
	template<typename Structure>
	inline void StructureBuffer<Structure>::Initialize(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList,
		std::vector<Structure>& cpuData)
	{
		bufferSize = (UINT)(sizeof(Structure) * cpuData.size());

		// default upload buffer 생성
		D3D12_RESOURCE_DESC resourceDesc;
		ZeroMemory(&resourceDesc, sizeof(resourceDesc));
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = bufferSize;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		resourceDesc.MipLevels = 1;

		ThrowIfFailed(
			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(&mStructureBuffer)));

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mUpload)));

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&mReadBack)));

		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = cpuData.data();
		subResourceData.RowPitch = bufferSize;
		subResourceData.SlicePitch = bufferSize;

		D3D12_RESOURCE_BARRIER barriers[] = {
			CD3DX12_RESOURCE_BARRIER::Transition(
				mStructureBuffer.Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_COPY_DEST)
		};

		commandList->ResourceBarrier(1, barriers);

		UpdateSubresources(commandList.Get(), mStructureBuffer.Get(), mUpload.Get(), 0, 0, 1, &subResourceData);

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			mStructureBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


		CD3DX12_RANGE range(0, 0);
		mReadBack->Map(0, &range, reinterpret_cast<void**>(&pGpuData));

		Renderer::Utility::CreateDescriptorHeap(device, mHeapNSV, Renderer::DescriptorType::UAV, 2, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		Renderer::Utility::CreateDescriptorHeap(device, mHeap, Renderer::DescriptorType::UAV, 2, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};

		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.Buffer.NumElements = (UINT)cpuData.size();
		SRVDesc.Buffer.FirstElement = 0;

		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
		SRVDesc.Buffer.StructureByteStride = sizeof(Structure);
		SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
		UAVDesc.Buffer.CounterOffsetInBytes = 0;
		UAVDesc.Buffer.NumElements = (UINT)cpuData.size();
		UAVDesc.Buffer.StructureByteStride = sizeof(Structure);
		UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mHeapNSV->GetCPUDescriptorHandleForHeapStart());
		offset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		device->CreateUnorderedAccessView(mStructureBuffer.Get(), nullptr, &UAVDesc, handle);
		handle.Offset(1, offset);
		device->CreateShaderResourceView(mStructureBuffer.Get(), &SRVDesc, handle);

		device->CopyDescriptorsSimple(2, mHeap->GetCPUDescriptorHandleForHeapStart(), mHeapNSV->GetCPUDescriptorHandleForHeapStart(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

}