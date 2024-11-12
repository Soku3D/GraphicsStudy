#include "Particle.h"
#include <random>
#include "Utility.h"

void Particles::Initialize(int numPatricles)
{
	using DirectX::SimpleMath::Vector3;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distribPos(-1.3f, 1.3f);
	std::uniform_real_distribution<> distribColor(0.f, 1.f);
	std::uniform_int_distribution<> colorIndex(0, 6);
	std::uniform_real_distribution<> distribRadius(0.01f, 0.04f);
	XMFLOAT3 colorLists[] =
	{
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 0.65f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.3f, 0.0f, 0.5f },
		{ 0.5f, 0.0f, 1.0f }
	};
	mCpu.resize(numPatricles);
	for (int i = 0; i < numPatricles; i++)
	{
		Particle particle;
		particle.color = colorLists[colorIndex(gen)];
		particle.position = Vector3((float)distribPos(gen), (float)distribPos(gen), 0.f);
		particle.radius = (float)distribRadius(gen);
		particle.radius = 0.04f;
		particle.life = 1.f;
		particle.mass = 1.f;
		mCpu[i] = particle;
	}
}

void Particles::InitializeSPH(int numPatricles)
{
	using DirectX::SimpleMath::Vector3;
	using DirectX::SimpleMath::Vector2;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distribPos(-0.04, 0.04f);
	std::uniform_real_distribution<> distribColor(0.f, 1.f);
	std::uniform_real_distribution<> distribRadius(0.01f, 0.04f);
	std::uniform_real_distribution<> distribVeolcity(-2.f, -0.3f);
	std::uniform_real_distribution<> distribLife(-10.f, 0.f);
	std::uniform_real_distribution<> randomTheta(-3.14f, 3.14f);
	mCpu.resize(numPatricles);
	float radius = 0.06f;
	for (int i = 0; i < numPatricles; i++)
	{
		Particle particle;
		particle.color = Vector3((float)distribColor(gen), (float)distribColor(gen), (float)distribColor(gen));
		//particle.color = Vector3(0.f, 0.f, 0.f);
		particle.position = Vector3(100.f, 0.f, 0.f);
		float theta = (float)randomTheta(gen);
		particle.originPosition = Vector3(-0.5f + cos(theta) * 0.1f, sin(theta) * 0.1f, 0.f);
		particle.radius = radius;
		particle.velocity = Vector3(1.f, 0.f, 0.f);
		particle.originVelocity = particle.velocity;
		particle.force = Vector3::Zero;

		particle.density = 0.f;
		particle.pressure = 0.f;
		particle.mass = 1.f;
		particle.life = (float)distribLife(gen);

		particle.density0 = 1.f;
		particle.pressureCoeff = 2.f;
		particle.viscosity = 0.5f;

		mCpu[i] = particle;
	}
}


void Particles::InitializeCloth(int n)
{
	InitializeCloth(n, n);
}

void Particles::InitializeCloth(int width, int height)
{
	this->width = width;
	this->height = height;

	using DirectX::SimpleMath::Vector3;
	using DirectX::SimpleMath::Vector2;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distribPos(-0.04, 0.04f);
	std::uniform_real_distribution<> distribColor(0.f, 1.f);
	
	float radius = 0.04f;

	float baseX = 0.f;
	float baseY = 0.f;
	float dx = 1.f;
	float dy = 1.f;

	mCpu.resize(width*height);
	
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int i = y * width + x;
			Particle particle;
			particle.color = Vector3((float)distribColor(gen), (float)distribColor(gen), (float)distribColor(gen));
			particle.position = Vector3(baseX + dx * x * 0.85f, baseY - dy * y * 1.1f, baseY - dy * y * 0.1f);

			particle.velocity = Vector3::Zero;
			particle.radius = radius;

			mCpu[i] = particle;
		}
	
	}
}


void Particles::BuildResources(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, DXGI_FORMAT format, int width, int height) {
	CreateResource(device, commandList, width, height, format, mRandom);

	BuildResources(device, commandList);
}

void Particles::BuildResources(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	mStructureBuffer.Initialize(device, commandList, mCpu);
	mStructureBufferTemp.Initialize(device, commandList, mCpu);
	offset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 4;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mHeap.ReleaseAndGetAddressOf())));
	CD3DX12_CPU_DESCRIPTOR_HANDLE destHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE srcHandle(mStructureBuffer.GetNSVCPUHandle(0));
	CD3DX12_CPU_DESCRIPTOR_HANDLE srcTempHandle(mStructureBufferTemp.GetNSVCPUHandle(0));

	device->CopyDescriptorsSimple(2, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	destHandle.Offset(2, offset);
	device->CopyDescriptorsSimple(2, destHandle, srcTempHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	//const UINT bufferSize = (UINT)(sizeof(Particle) * mCpu.size());

	//// default upload buffer 생성
	//D3D12_RESOURCE_DESC resourceDesc;
	//ZeroMemory(&resourceDesc, sizeof(resourceDesc));
	//resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	//resourceDesc.Width = bufferSize;
	//resourceDesc.Height = 1;
	//resourceDesc.DepthOrArraySize = 1;
	//resourceDesc.MipLevels = 1;
	//resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	//resourceDesc.SampleDesc.Count = 1;
	//resourceDesc.SampleDesc.Quality = 0;
	//resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	//resourceDesc.MipLevels = 1;

	//ThrowIfFailed(
	//	device->CreateCommittedResource(
	//		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//		D3D12_HEAP_FLAG_NONE,
	//		&resourceDesc,
	//		D3D12_RESOURCE_STATE_COMMON,
	//		nullptr,
	//		IID_PPV_ARGS(&mGpu)));

	//ThrowIfFailed(device->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
	//	D3D12_HEAP_FLAG_NONE,
	//	&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
	//	D3D12_RESOURCE_STATE_GENERIC_READ,
	//	nullptr,
	//	IID_PPV_ARGS(&mUpload)));

	//ThrowIfFailed(device->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
	//	D3D12_HEAP_FLAG_NONE,
	//	&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
	//	D3D12_RESOURCE_STATE_COPY_DEST,
	//	nullptr,
	//	IID_PPV_ARGS(&mReadBack)));

	//D3D12_SUBRESOURCE_DATA subResourceData = {};
	//subResourceData.pData = mCpu.data();
	//subResourceData.RowPitch = bufferSize;
	//subResourceData.SlicePitch = bufferSize;

	//D3D12_RESOURCE_BARRIER barriers[] = {
	//	CD3DX12_RESOURCE_BARRIER::Transition(
	//		mGpu.Get(),
	//		D3D12_RESOURCE_STATE_COMMON,
	//		D3D12_RESOURCE_STATE_COPY_DEST)
	//};

	//commandList->ResourceBarrier(1, barriers);

	//UpdateSubresources(commandList.Get(), mGpu.Get(), mUpload.Get(), 0, 0, 1, &subResourceData);

	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
	//	mGpu.Get(),
	//	D3D12_RESOURCE_STATE_COPY_DEST,
	//	D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	//CD3DX12_RANGE range(0, 0);
	//mReadBack->Map(0, &range, reinterpret_cast<void**>(&pGpuData));
}

void Particles::BuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	//Renderer::Utility::CreateDescriptorHeap(device, mHeap, Renderer::DescriptorType::UAV, 4, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	//D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};

	//SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//SRVDesc.Buffer.NumElements = (UINT)mCpu.size();
	//SRVDesc.Buffer.FirstElement = 0;

	//SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	//SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	//SRVDesc.Buffer.StructureByteStride = sizeof(Particle);
	//SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	//D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	//UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	//UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	//UAVDesc.Buffer.CounterOffsetInBytes = 0;
	//UAVDesc.Buffer.NumElements = (UINT)mCpu.size();
	//UAVDesc.Buffer.StructureByteStride = sizeof(Particle);
	//UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	//CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mHeap->GetCPUDescriptorHandleForHeapStart());
	//offset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//device->CreateUnorderedAccessView(mGpu.Get(), nullptr, &UAVDesc, handle);
	//handle.Offset(1, offset);
	//device->CreateShaderResourceView(mGpu.Get(), &SRVDesc, handle);

	//if (mRandom.Get() != nullptr) {

	//	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	//	uavDesc.Texture2D.MipSlice = 0;
	//	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	//	uavDesc.Format = mRandom->GetDesc().Format;

	//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//	srvDesc.Format = mRandom->GetDesc().Format;
	//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//	srvDesc.Texture2D.MipLevels = 1;

	//	handle.Offset(1, offset);
	//	device->CreateUnorderedAccessView(mRandom.Get(), nullptr, &uavDesc, handle);
	//	handle.Offset(1, offset);
	//	device->CreateShaderResourceView(mRandom.Get(), &srvDesc, handle);
	//}
}

void Particles::CopyToCpu()
{
	//memcpy(mCpu.data(), pGpuData, mCpu.size() * sizeof(Particle));
}

void Particles::CopyToGpu(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList) {
	/*const UINT bufferSize = (UINT)(sizeof(Particle) * mCpu.size());
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = mCpu.data();
	subResourceData.RowPitch = bufferSize;
	subResourceData.SlicePitch = bufferSize;

	commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			mGpu.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_COPY_DEST));

	UpdateSubresources(commandList.Get(), mGpu.Get(), mUpload.Get(), 0, 0, 1, &subResourceData);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		mGpu.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS));*/
}

void Particles::CreateResource(Microsoft::WRL::ComPtr<ID3D12Device5>& device,
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