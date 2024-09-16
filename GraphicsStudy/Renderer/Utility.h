#pragma once

#include <string>
#include <windows.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>
#include <sstream> 

#include "Timer.h"
#include <cassert>

#include "dxgi.h"
#include "Vertex.h"
#include "d3d11.h"
#include "wrl.h"
#include "d3dcompiler.h"

#include "d3dx12.h"
#include "d3d12.h"
#include "dxgi1_4.h"
#include <directxtk12/Audio.h>

#include "Camera.h"
#include "Buffer.h"

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}
#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1) 

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif
class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	std::wstring ToString()const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};
namespace Renderer {
	enum DescriptorType {
		RTV,
		UAV,
		SRV,
		DSV,
		CBV
	};

	using Microsoft::WRL::ComPtr;

	class Utility {
	public:
		template<typename Vertex>
		static void CreateVertexBuffer(std::vector<Vertex>& vertices,
			Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer,
			Microsoft::WRL::ComPtr<ID3D12Device5>& device) {
			D3D11_BUFFER_DESC buffDesc;
			buffDesc.ByteWidth = UINT(sizeof(Vertex) * vertices.size());
			buffDesc.Usage = D3D11_USAGE_IMMUTABLE;
			buffDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
			buffDesc.CPUAccessFlags = false;
			buffDesc.MiscFlags = 0;
			buffDesc.StructureByteStride = sizeof(Vertex);

			D3D11_SUBRESOURCE_DATA subData;
			ZeroMemory(&subData, sizeof(subData));
			subData.pSysMem = vertices.data();

			ThrowIfFailed(device->CreateBuffer(&buffDesc, &subData, buffer.GetAddressOf()));
		}
		template<typename Index>
		static void CreateIndexBuffer(std::vector<Index>& indices,
			Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer,
			Microsoft::WRL::ComPtr<ID3D12Device5>& device) {
			D3D11_BUFFER_DESC buffDesc;
			buffDesc.ByteWidth = UINT(sizeof(Index) * indices.size());
			buffDesc.Usage = D3D11_USAGE_IMMUTABLE;
			buffDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
			buffDesc.CPUAccessFlags = false;
			buffDesc.MiscFlags = 0;
			buffDesc.StructureByteStride = sizeof(Index);

			D3D11_SUBRESOURCE_DATA subData;
			ZeroMemory(&subData, sizeof(subData));
			subData.pSysMem = indices.data();

			ThrowIfFailed(device->CreateBuffer(&buffDesc, &subData, buffer.GetAddressOf()));
		}
		
		template<typename V>
		static void CreateUploadBuffer(std::vector<V>& data,
			ComPtr<ID3D12Resource> & buffer,
			ComPtr<ID3D12Device5> & device) {
			
			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(data.size() * sizeof(V)),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&buffer)
			));
		}
		template<typename V>
		static void CreateGPUBuffer(std::vector<V>& data,
			ComPtr<ID3D12Resource>& buffer,
			ComPtr<ID3D12Device5>& device) {
			
			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(data.size() * sizeof(V)), 
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(&buffer)
			));
		}
		// data->UploadBuffer->DefaultBuffer
		template<typename V>
		static void CreateBuffer(std::vector<V>& data,
			ComPtr<ID3D12Resource>& uploadBuffer,
			ComPtr<ID3D12Resource>& gpuBuffer,
			ComPtr<ID3D12Device5>& device,
			ComPtr<ID3D12GraphicsCommandList> & commandList) {

			CreateUploadBuffer<V>(data, uploadBuffer, device);
			CreateGPUBuffer<V>(data, gpuBuffer, device);

			D3D12_SUBRESOURCE_DATA subData;
			subData.pData = data.data();
			subData.RowPitch = data.size() * sizeof(V);
			subData.SlicePitch = subData.RowPitch;

			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpuBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

			UpdateSubresources<1>(commandList.Get(), gpuBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subData);
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpuBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
		}
		
		// Texture Buffer를 만들고 Texture에 대한 Descriptor를 Heap에 넣는다
		static void CreateTextureBuffer(std::wstring path, ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12DescriptorHeap>& heap,
			ComPtr<ID3D12Device5>& device, ComPtr<ID3D12CommandQueue>& commandQueue, ComPtr<ID3D12GraphicsCommandList>& commandList, int offset, int descriptorSize, bool* bIsCubeMap);

		static void CreateDescriptorHeap(ComPtr<ID3D12Device5>& deivce, 
			ComPtr<ID3D12DescriptorHeap>& heap, 
			const Renderer::DescriptorType& type, 
			int Numdescriptors, 
			D3D12_DESCRIPTOR_HEAP_FLAGS flag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			const std::wstring name = L"");

		static void CreateBuffer(ComPtr<ID3D12Device5>& deivce, D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS heapFlag, UINT64 buffersize, 
			D3D12_RESOURCE_FLAGS resourceFlag, D3D12_RESOURCE_STATES resourceState, ComPtr<ID3D12Resource>& buffer);

		static void CreateConstantBuffer(ComPtr<ID3D12Device5>& device, ComPtr<ID3D12GraphicsCommandList>& commandList, UINT64 buffersize, ComPtr<ID3D12Resource>& buffer, void** pBufferData);

		template<typename ConstantStructure>
		static void CreateConstantBuffer(ComPtr<ID3D12Device5>& device, ComPtr<ID3D12GraphicsCommandList>& commandList,
			Core::ConstantBuffer<ConstantStructure>& buffer)
		{
			ThrowIfFailed(
				device->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
					D3D12_HEAP_FLAG_NONE,
					&CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantStructure)),
					D3D12_RESOURCE_STATE_COMMON,
					nullptr,
					IID_PPV_ARGS(buffer.GetAddressOf())));

			commandList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					buffer.Get(),
					D3D12_RESOURCE_STATE_COMMON,
					D3D12_RESOURCE_STATE_GENERIC_READ
				));

			CD3DX12_RANGE range(0, 0);
			ThrowIfFailed(buffer.Get()->Map(0, &range, reinterpret_cast<void**>(buffer.GetData())));
		}
};
}