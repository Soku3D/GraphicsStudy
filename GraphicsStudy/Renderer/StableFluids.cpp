#include "StableFluids.h"
#include "Utility.h"

void StableFluids::Initialize()
{
}

void StableFluids::Update(float& deltaTime)
{
}

void StableFluids::BuildResources(Microsoft::WRL::ComPtr<ID3D12Device5>& device,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList,
	const int& width,
	const int& height,
	DXGI_FORMAT& format)
{
	CreateResource(device, commandList, width, height, format, mDensity);
	CreateResource(device, commandList, width, height, format, mVelocity);
	
	CreateResource(device, commandList, width, height, format, mDensityTemp);
	CreateResource(device, commandList, width, height, format, mVelocityTemp);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 6;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mHeap.ReleaseAndGetAddressOf())));

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Format = format;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	offset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mHeap->GetCPUDescriptorHandleForHeapStart());
	device->CreateUnorderedAccessView(mDensity.Get(), nullptr, &uavDesc, handle);
	handle.Offset(1, offset);
	device->CreateUnorderedAccessView(mVelocity.Get(), nullptr, &uavDesc, handle);
	handle.Offset(1, offset);
	device->CreateShaderResourceView(mDensity.Get(), &srvDesc, handle);
	handle.Offset(1, offset);
	device->CreateShaderResourceView(mVelocity.Get(), &srvDesc, handle);
	handle.Offset(1, offset);
	device->CreateShaderResourceView(mDensityTemp.Get(), &srvDesc, handle);
	handle.Offset(1, offset);
	device->CreateShaderResourceView(mVelocityTemp.Get(), &srvDesc, handle);
	
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(mDensity.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		CD3DX12_RESOURCE_BARRIER::Transition(mVelocity.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	};

	commandList->ResourceBarrier(2, barriers);
}

void StableFluids::CreateResource(Microsoft::WRL::ComPtr<ID3D12Device5>& device,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList,
	const int& width,
	const int& height,
	DXGI_FORMAT& format,
	Microsoft::WRL::ComPtr<ID3D12Resource>& resource)
{
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Format = format;
	resourceDesc.MipLevels = 0;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(resource.ReleaseAndGetAddressOf())));

}