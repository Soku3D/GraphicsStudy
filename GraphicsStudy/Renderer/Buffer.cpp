#include "Buffer.h"
#include "GeometryGenerator.h"

void Core::Texture3D::Initiailize(UINT width, UINT height, UINT depth, DXGI_FORMAT format, Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	
	volumeWidth = width;
	volumeHeight = height;
	volumeDepth = depth;
	mVolumeFormat = format;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NumDescriptors = 2;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mVolumeTextureHeap.ReleaseAndGetAddressOf())));

	D3D12_RESOURCE_DESC rDesc = {};
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	rDesc.Format = mVolumeFormat;
	rDesc.MipLevels = 1;
	rDesc.Width = volumeWidth;
	rDesc.Height = volumeHeight;
	rDesc.DepthOrArraySize = volumeDepth;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rDesc.SampleDesc.Count = 1;
	rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&rDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(mVolumeTexture.ReleaseAndGetAddressOf())));

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Format = mVolumeFormat;
	uavDesc.Texture3D.MipSlice = 1;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Format = mVolumeFormat;

	offset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	srvDesc.Texture3D.MipLevels = 1;
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mVolumeTextureHeap->GetCPUDescriptorHandleForHeapStart());
	device->CreateUnorderedAccessView(mVolumeTexture.Get(), nullptr, nullptr, handle);
	handle.Offset(1, offset);
	device->CreateShaderResourceView(mVolumeTexture.Get(), nullptr, handle);
}

void Core::Texture3D::InitVolumeMesh(const float& halfLength, Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	InitVolumeMesh(halfLength, halfLength, halfLength, device, commandList);
}

void Core::Texture3D::InitVolumeMesh(const float& x, const float& y, const float& z, Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	mVolumeMesh = std::make_shared<Core::StaticMesh>();
	mVolumeMesh->Initialize(GeometryGenerator::PbrBox(x,y,z, L""), device, commandList);
	mVolumeMesh->SetBoundingBoxHalfLength(x, y, z);
}
void Core::Texture3D::Update(float& deltaTime)
{
	mVolumeMesh->Update(deltaTime);
}

void Core::Texture3D::Render(float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commmandList, bool bUseConstantBuffer)
{
	mVolumeMesh->Render(deltaTime, commmandList, bUseConstantBuffer);
}

void Core::Texture3D::RenderBoundingBox(float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commmandList)
{
	mVolumeMesh->RenderBoundingBox(deltaTime, commmandList);
}

