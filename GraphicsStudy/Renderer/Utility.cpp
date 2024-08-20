#include "Utility.h"
#include <comdef.h>
#include "directxtk12/DDSTextureLoader.h"
#include "directxtk12/WICTextureLoader.h"
#include "directxtk12/ResourceUploadBatch.h"
#include <DirectXTexEXR.h>

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) :
	ErrorCode(hr),
	FunctionName(functionName),
	Filename(filename),
	LineNumber(lineNumber)
{
}

std::wstring DxException::ToString()const
{
	// Get the string description of the error code.
	_com_error err(ErrorCode);
	std::wstring msg = err.ErrorMessage();

	return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}

void Renderer::Utility::CreateTextureBuffer(std::wstring path, ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12DescriptorHeap>& heap,
	ComPtr<ID3D12Device>& device, ComPtr<ID3D12CommandQueue>& commandQueue, ComPtr<ID3D12GraphicsCommandList>& commandList, int offset, int descriptorSize, bool* bIsCubeMap)
{
	using namespace DirectX;

	ResourceUploadBatch resourceUpload(device.Get());

	resourceUpload.Begin();
	if (path.substr(path.size() - 3) == L"dds") {
		if (bIsCubeMap != nullptr && *bIsCubeMap) {
			ThrowIfFailed(
				CreateDDSTextureFromFile(device.Get(), resourceUpload, path.c_str(),
					texture.ReleaseAndGetAddressOf(), false, 0, nullptr, bIsCubeMap));
		}
		else {
			ThrowIfFailed(
				CreateDDSTextureFromFile(device.Get(), resourceUpload, path.c_str()
					,texture.ReleaseAndGetAddressOf(), false));
		}
	}
	else {
		ThrowIfFailed(CreateWICTextureFromFileEx(device.Get(), resourceUpload, path.c_str(), 0,
			D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE,
			WIC_LOADER_MIP_AUTOGEN | WIC_LOADER_FORCE_SRGB,
			texture.ReleaseAndGetAddressOf()));
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texture->GetDesc().Format;

	std::wcout << path.c_str() << ' ' << texture->GetDesc().MipLevels << ' ' 
		<< texture->GetDesc().Format << std::endl;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (bIsCubeMap != nullptr && *bIsCubeMap == true) {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = texture->GetDesc().MipLevels;
		srvDesc.TextureCube.MostDetailedMip = 0.f;
	}
	else {
		srvDesc.Texture2D.MipLevels = texture->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	}


	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(heap->GetCPUDescriptorHandleForHeapStart());
	handle.Offset(offset, descriptorSize);
	device->CreateShaderResourceView(texture.Get(), &srvDesc, handle);

	auto uploadResourcesFinished = resourceUpload.End(
		commandQueue.Get());

	uploadResourcesFinished.wait();

}

void Renderer::Utility::CreateDescriptorHeap(ComPtr<ID3D12Device>& deivce,
	ComPtr<ID3D12DescriptorHeap>& heap,
	const Renderer::DescriptorType& type,
	int Numdescriptors,
	D3D12_DESCRIPTOR_HEAP_FLAGS flag)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	ZeroMemory(&heapDesc, sizeof(heapDesc));
	heapDesc.NumDescriptors = Numdescriptors;

	if (type == DescriptorType::DSV)
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	else if (type == DescriptorType::RTV)
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	else if (type == DescriptorType::SRV)
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	else if (type == DescriptorType::UAV)
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	else if (type == DescriptorType::CBV)
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	heapDesc.Flags = flag;
	heapDesc.NodeMask = 0;

	ThrowIfFailed(deivce->CreateDescriptorHeap(
		&heapDesc, IID_PPV_ARGS(heap.GetAddressOf())));
}