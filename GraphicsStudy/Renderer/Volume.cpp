#include "Volume.h"
#include "Utility.h"

void Volume::Render(float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, bool bUseConstantBuffer)
{
	mDensity.Render(deltaTime, commandList, bUseConstantBuffer);
}

void Volume::Dispatch(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	commandList->Dispatch((UINT)std::ceil(mDensity.GetWidth() / 16.f),
		(UINT)std::ceil(mDensity.GetHeight() / 16.f),
		(UINT)std::ceil(mDensity.GetDepth() / 4.f));
}

void Volume::RenderBoundingBox(float& deltaTime, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commmandList)
{
	mDensity.RenderBoundingBox(deltaTime, commmandList);
}

void Volume::Update(float& deltaTime)
{
	mDensity.Update(deltaTime);
}

void Volume::Initialize(const int& width, const int& height, const int& depth, Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	offset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	mDensity.Initiailize(width, height, depth, DXGI_FORMAT_R16_FLOAT, device, commandList);
	mDensityTemp.Initiailize(width, height, depth, DXGI_FORMAT_R16_FLOAT, device, commandList);
	mVelocity.Initiailize(width, height, depth, DXGI_FORMAT_R16G16B16A16_FLOAT, device, commandList);
	mVelocityTemp.Initiailize(width, height, depth, DXGI_FORMAT_R16G16B16A16_FLOAT, device, commandList);
	mDivergence.Initiailize(width, height, depth, DXGI_FORMAT_R16_FLOAT, device, commandList);
	mPressure.Initiailize(width, height, depth, DXGI_FORMAT_R16_FLOAT, device, commandList);
	mPressureTemp.Initiailize(width, height, depth, DXGI_FORMAT_R16_FLOAT, device, commandList);
	mVorticity.Initiailize(width, height, depth, DXGI_FORMAT_R16_FLOAT, device, commandList);
	mBoundaryCondition.Initiailize(width, height, depth, DXGI_FORMAT_R8_SINT, device, commandList);

	CopyDescriptors(device);

	mDensity.InitVolumeMesh(2.f, 1.f, 1.f, device, commandList);
}

void Volume::CopyDescriptors(Microsoft::WRL::ComPtr<ID3D12Device5>& device)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NumDescriptors = 18;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mHeap.ReleaseAndGetAddressOf())));
	heapDesc.NumDescriptors = 4;
	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mCPHeap1.ReleaseAndGetAddressOf())));
	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mCPHeap2.ReleaseAndGetAddressOf())));

	heapDesc.NumDescriptors = 5;
	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mDivergenceHeap.ReleaseAndGetAddressOf())));

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mHeap->GetCPUDescriptorHandleForHeapStart());

	device->CopyDescriptorsSimple(1, handle, mDensity.GetNSVUavCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); // 0 density UAV
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mVelocity.GetNSVUavCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); // 1 velocity UAV
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mBoundaryCondition.GetNSVUavCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); // 2 bc UAV
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mDivergence.GetNSVUavCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); // 3 divergence UAV
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mPressure.GetNSVUavCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); // 4 pressure UAV
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mPressureTemp.GetNSVUavCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); // 5 pressureTemp UAV
	handle.Offset(1, offset);

	device->CopyDescriptorsSimple(1, handle, mDensity.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);  // 6
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mVelocity.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mDivergence.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mPressureTemp.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mPressure.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mBoundaryCondition.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mDensityTemp.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); // 12
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mVelocityTemp.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mDensityTemp.GetNSVUavCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); // 14 densityTemp UAV
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mVelocityTemp.GetNSVUavCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); // 15 velocityTemp UAV
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mVorticity.GetNSVUavCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); // 16 vorticity UAV
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mVorticity.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	handle = mCPHeap1->GetCPUDescriptorHandleForHeapStart();
	device->CopyDescriptorsSimple(1, handle, mPressure.GetNSVUavCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mPressureTemp.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mDivergence.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mBoundaryCondition.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	handle = mCPHeap2->GetCPUDescriptorHandleForHeapStart();
	device->CopyDescriptorsSimple(1, handle, mPressureTemp.GetNSVUavCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mPressure.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mDivergence.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mBoundaryCondition.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	handle = mDivergenceHeap->GetCPUDescriptorHandleForHeapStart();
	device->CopyDescriptorsSimple(1, handle, mDivergence.GetNSVUavCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mPressure.GetNSVUavCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mPressureTemp.GetNSVUavCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mVelocity.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, offset);
	device->CopyDescriptorsSimple(1, handle, mBoundaryCondition.GetNSVSrvCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);



}