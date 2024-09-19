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
	CreateResource(device, commandList, width, height, format, mPressure);
	CreateResource(device, commandList, width, height, format, mDivergence);

	CreateResource(device, commandList, width, height, format, mDensityTemp);
	CreateResource(device, commandList, width, height, format, mVelocityTemp);
	CreateResource(device, commandList, width, height, format, mPressureTemp);
	CreateResource(device, commandList, width, height, format, mVorticity);
	CreateResource(device, commandList, width, height, format, mVorticityDir);

	mDensity->SetName(L"Density");
	mVelocity->SetName(L"Velocity");
	mPressure->SetName(L"Pressure");
	mDivergence->SetName(L"Divergence");
	mDensityTemp->SetName(L"DensityTemp");
	mVelocityTemp->SetName(L"VelocityTemp");
	mPressureTemp->SetName(L"PressureTemp");
	mVorticity->SetName(L"Vorticity");
	mVorticityDir->SetName(L"VorticityDir");

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 18;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mHeapNSV.ReleaseAndGetAddressOf())));
	
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mHeap.ReleaseAndGetAddressOf())));
	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mHeapPT.ReleaseAndGetAddressOf())));
	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mHeapP.ReleaseAndGetAddressOf())));

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mDensityRTVHeap.ReleaseAndGetAddressOf())));


	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Format = format;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = format;

	offset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mHeapNSV->GetCPUDescriptorHandleForHeapStart());

	device->CreateUnorderedAccessView(mDensity.Get(), nullptr, &uavDesc, handle);
	handle.Offset(1, offset);
	device->CreateUnorderedAccessView(mVelocity.Get(), nullptr, &uavDesc, handle);
	handle.Offset(1, offset);
	device->CreateUnorderedAccessView(mDivergence.Get(), nullptr, &uavDesc, handle);
	handle.Offset(1, offset);
	device->CreateUnorderedAccessView(mPressure.Get(), nullptr, &uavDesc, handle);
	handle.Offset(1, offset);
	device->CreateUnorderedAccessView(mPressureTemp.Get(), nullptr, &uavDesc, handle);

	handle.Offset(1, offset);
	device->CreateShaderResourceView(mDensity.Get(), &srvDesc, handle);  // 5
	handle.Offset(1, offset);
	device->CreateShaderResourceView(mVelocity.Get(), &srvDesc, handle);
	handle.Offset(1, offset);
	device->CreateShaderResourceView(mDivergence.Get(), &srvDesc, handle);
	handle.Offset(1, offset);
	device->CreateShaderResourceView(mPressureTemp.Get(), &srvDesc, handle);
	handle.Offset(1, offset);
	device->CreateShaderResourceView(mPressure.Get(), &srvDesc, handle);
	handle.Offset(1, offset);
	device->CreateShaderResourceView(mDensityTemp.Get(), &srvDesc, handle); //10
	handle.Offset(1, offset);
	device->CreateShaderResourceView(mVelocityTemp.Get(), &srvDesc, handle);
	handle.Offset(1, offset);
	device->CreateUnorderedAccessView(mDensityTemp.Get(), nullptr, &uavDesc, handle); //12
	handle.Offset(1, offset);
	device->CreateUnorderedAccessView(mVelocityTemp.Get(), nullptr, &uavDesc, handle);
	handle.Offset(1, offset);
	device->CreateUnorderedAccessView(mVorticity.Get(), nullptr, &uavDesc, handle); // 14
	handle.Offset(1, offset);
	device->CreateUnorderedAccessView(mVorticityDir.Get(), nullptr, &uavDesc, handle);
	handle.Offset(1, offset);
	device->CreateShaderResourceView(mVorticity.Get(), &srvDesc, handle); // 16
	handle.Offset(1, offset);
	device->CreateShaderResourceView(mVorticityDir.Get(), &srvDesc, handle);

	device->CreateRenderTargetView(mDensity.Get(), &rtvDesc, mDensityRTVHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(mDensity.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		CD3DX12_RESOURCE_BARRIER::Transition(mVelocity.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		CD3DX12_RESOURCE_BARRIER::Transition(mDivergence.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		CD3DX12_RESOURCE_BARRIER::Transition(mPressure.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		CD3DX12_RESOURCE_BARRIER::Transition(mPressureTemp.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		CD3DX12_RESOURCE_BARRIER::Transition(mDensityTemp.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		CD3DX12_RESOURCE_BARRIER::Transition(mVelocityTemp.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		CD3DX12_RESOURCE_BARRIER::Transition(mVorticity.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		CD3DX12_RESOURCE_BARRIER::Transition(mVorticityDir.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	};

	commandList->ResourceBarrier(9, barriers);

	DescriptorInsert(device, mHeap, mHeapNSV, 0, 0, 18);

	DescriptorInsert(device, mHeapP, mHeapNSV, 3, 0, 1);
	DescriptorInsert(device, mHeapPT, mHeapNSV, 4, 0, 1); // Insert PressureTemp Uav 
	DescriptorInsert(device, mHeapP, mHeapNSV, 8, 1, 1);
	DescriptorInsert(device, mHeapPT, mHeapNSV, 9, 1, 1);
	DescriptorInsert(device, mHeapP, mHeapNSV, 7, 2, 1);
	DescriptorInsert(device, mHeapPT, mHeapNSV, 7, 2, 1);
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
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(resource.ReleaseAndGetAddressOf())));

}

void StableFluids::DescriptorInsert(Microsoft::WRL::ComPtr<ID3D12Device5>& device,
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& destHeap,
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srcHeap,
	const int& srcIndex,
	const int& destIndex,
	const int& count)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE dest(destHeap->GetCPUDescriptorHandleForHeapStart(), destIndex, offset);
	CD3DX12_CPU_DESCRIPTOR_HANDLE src(srcHeap->GetCPUDescriptorHandleForHeapStart(), srcIndex, offset);
	device->CopyDescriptorsSimple(count, dest, src, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}
