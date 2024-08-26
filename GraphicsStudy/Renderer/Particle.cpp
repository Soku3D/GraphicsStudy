#include "Particle.h"
#include <random>
#include "Utility.h"

void Particles::Initialize(int numPatricles)
{
	using DirectX::SimpleMath::Vector3;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distribPos(-1.f, 1.f);
	std::uniform_real_distribution<> distribColor(0.f, 1.f);
	m_cpu.resize(numPatricles);
	for (int i = 0; i < numPatricles; i++)
	{
		Particle particle;
		particle.m_color = Vector3((float)distribColor(gen), (float)distribColor(gen), (float)distribColor(gen));
		particle.m_position = Vector3((float)distribPos(gen), (float)distribPos(gen), 0.f);

		m_cpu[i] = particle;
	}
}

void Particles::BuildResources(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	const UINT bufferSize = sizeof(Particle) * m_cpu.size();

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
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_gpu)
		));

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_upload)));


	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = m_cpu.data();
	subResourceData.RowPitch = bufferSize;
	subResourceData.SlicePitch = bufferSize;

	UpdateSubresources(commandList.Get(), m_gpu.Get(), m_upload.Get(), 0, 0, 1, &subResourceData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_gpu.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS));



}

void Particles::BuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Device5>& device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	Renderer::Utility::CreateDescriptorHeap(device, m_srvHeap, Renderer::DescriptorType::SRV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	Renderer::Utility::CreateDescriptorHeap(device, m_uavHeap, Renderer::DescriptorType::UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.Buffer.NumElements = m_cpu.size();
	SRVDesc.Buffer.StructureByteStride = sizeof(Particle);
	SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	SRVDesc.Buffer.FirstElement = 0;

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	UAVDesc.Buffer.CounterOffsetInBytes = 0;
	UAVDesc.Buffer.NumElements = m_cpu.size();
	UAVDesc.Buffer.StructureByteStride = sizeof(Particle);
	UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	device->CreateShaderResourceView(m_gpu.Get(), &SRVDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	device->CreateUnorderedAccessView(m_gpu.Get(), nullptr, &UAVDesc, m_uavHeap->GetCPUDescriptorHandleForHeapStart());
}
