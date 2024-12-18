#include "RootSignature.h"
#include "Utility.h"

void Renderer::RootSignature::Initialize(UINT srvCount, UINT uavCount, UINT cbCount, std::vector<D3D12_STATIC_SAMPLER_DESC>& sampler)
{
	rangeTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, uavCount, 0);
	rangeTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srvCount, 0);

	paramenters.resize(cbCount + 2);
	paramenters[0].InitAsDescriptorTable(1, &rangeTable);
	paramenters[1].InitAsDescriptorTable(1, &rangeTable2);

	for (UINT i = 2; i <= 2; i++)
	{
		paramenters[i].InitAsConstantBufferView(i - 2);
	}
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	
	m_samplerArray = sampler;
	//m_samplerArray = sampler;
	if (m_samplerArray.empty()) {
		m_rootSignatureDesc.Init_1_1(cbCount + 2, paramenters.data(), 0, nullptr, rootSignatureFlags);
	}
	else
	{
		m_rootSignatureDesc.Init_1_1(cbCount + 2, paramenters.data(), (UINT)m_samplerArray.size(),
			m_samplerArray.data(), rootSignatureFlags);
	}
	
}
void Renderer::RootSignature::InitializeRaytracing(UINT uavCount, UINT srvCount, UINT cbCount, int numSamplers, D3D12_STATIC_SAMPLER_DESC* sampler)
{
	rangeTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, uavCount, 0);
	rangeTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srvCount, 9);

	paramenters.resize(cbCount + 3);
	paramenters[0].InitAsDescriptorTable(1, &rangeTable);
	paramenters[1].InitAsDescriptorTable(1, &rangeTable2);

	for (UINT i = 2; i <= 2; i++)
	{
		paramenters[i].InitAsShaderResourceView(i - 2);
	}
	for (UINT i = 3; i < 3 + cbCount; i++)
	{
		paramenters[i].InitAsConstantBufferView(i - 3);
	}
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_NONE;

	m_sampler = sampler;
	if (m_sampler == nullptr) {
		m_rootSignatureDesc.Init_1_1((UINT)paramenters.size(), paramenters.data(), 0, nullptr, rootSignatureFlags);
	}
	else
	{
		m_rootSignatureDesc.Init_1_1((UINT)paramenters.size(), paramenters.data(), 1, m_sampler, rootSignatureFlags);
	}
}
void Renderer::RootSignature::InitializeUavSrv(UINT uavCount, UINT srvCount, UINT cbCount, std::vector<D3D12_STATIC_SAMPLER_DESC>& sampler)
{
	rangeTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, uavCount, 0);

	paramenters.resize(cbCount + srvCount + 1);
	paramenters[0].InitAsDescriptorTable(1, &rangeTable);

	for (UINT i = 1; i <  1 + srvCount; i++)
	{
		paramenters[i].InitAsShaderResourceView(i - 1);
	}
	for (UINT i = 1 + srvCount; i < 1 + cbCount + srvCount; i++)
	{
		paramenters[i].InitAsConstantBufferView(i - 1 - srvCount);
	}
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_NONE;


	m_samplerArray = sampler;
	//m_samplerArray = sampler;
	if (m_samplerArray.empty()) {
		m_rootSignatureDesc.Init_1_1((UINT)paramenters.size(), paramenters.data(), 0, nullptr, rootSignatureFlags);
	}
	else
	{
		m_rootSignatureDesc.Init_1_1((UINT)paramenters.size(), paramenters.data(), (UINT)m_samplerArray.size(),
			m_samplerArray.data(), rootSignatureFlags);
	}
}

void Renderer::RootSignature::InitializeUAV(UINT uavCount, UINT cbCount, int numSamplers, D3D12_STATIC_SAMPLER_DESC* sampler) {
	
	rangeTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, uavCount, 0);
	paramenters.resize(cbCount + 1);
	
	paramenters[0].InitAsDescriptorTable(1, &rangeTable);

	for (UINT i = 1; i <= cbCount; i++)
	{
		paramenters[i].InitAsConstantBufferView(i - 1);
	}

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	m_sampler = sampler;
	if (m_sampler == nullptr) {
		m_rootSignatureDesc.Init_1_1(cbCount + 1, paramenters.data(), 0, nullptr, rootSignatureFlags);
	}
	else
	{
		m_rootSignatureDesc.Init_1_1(cbCount + 1, paramenters.data(), 1, m_sampler, rootSignatureFlags);
	}
}

void Renderer::RootSignature::InitializeUAV(UINT uavCount, UINT cbCount, std::vector<D3D12_STATIC_SAMPLER_DESC>& sampler)
{
	rangeTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, uavCount, 0);
	paramenters.resize(cbCount + 1);

	paramenters[0].InitAsDescriptorTable(1, &rangeTable);

	for (UINT i = 1; i <= cbCount; i++)
	{
		paramenters[i].InitAsConstantBufferView(i - 1);
	}

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	m_samplerArray = sampler;
	if (m_samplerArray.empty()) {
		m_rootSignatureDesc.Init_1_1((UINT)paramenters.size(), paramenters.data(), 0, nullptr, rootSignatureFlags);
	}
	else
	{
		m_rootSignatureDesc.Init_1_1((UINT)paramenters.size(), paramenters.data(), (UINT)m_samplerArray.size(),
			m_samplerArray.data(), rootSignatureFlags);
	}
}

void Renderer::RootSignature::Initialize(UINT srvCount, UINT cbCount, int numSamplers, D3D12_STATIC_SAMPLER_DESC* sampler)
{
	rangeTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srvCount, 0, 0,  D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

	paramenters.resize(cbCount+ 1);

	paramenters[0].InitAsDescriptorTable(1 , &rangeTable, D3D12_SHADER_VISIBILITY_ALL);

	for (UINT i = 1; i <= cbCount; i++)
	{
		paramenters[i].InitAsConstantBufferView(i - 1);
	}

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	m_sampler = sampler;
	if (m_sampler == nullptr) {
		m_rootSignatureDesc.Init_1_1(cbCount + 1, paramenters.data(), 0, nullptr, rootSignatureFlags);
	}
	else
	{
		m_rootSignatureDesc.Init_1_1(cbCount + 1, paramenters.data(), 1, m_sampler, rootSignatureFlags);
	}
}

void Renderer::RootSignature::Initialize(UINT srvCount, UINT cbCount, std::vector<D3D12_STATIC_SAMPLER_DESC>& sampler)
{
	rangeTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srvCount, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

	paramenters.resize(cbCount + 1);

	paramenters[0].InitAsDescriptorTable(1, &rangeTable, D3D12_SHADER_VISIBILITY_ALL);

	for (UINT i = 1; i <= cbCount; i++)
	{
		paramenters[i].InitAsConstantBufferView(i - 1);
	}

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	m_samplerArray = sampler;
	if (m_samplerArray.empty()) {
		m_rootSignatureDesc.Init_1_1(cbCount + 1, paramenters.data(), 0, nullptr, rootSignatureFlags);
	}
	else
	{
		m_rootSignatureDesc.Init_1_1(cbCount + 1, paramenters.data(), (UINT)m_samplerArray.size(),
			m_samplerArray.data(), rootSignatureFlags);
	}
}
void Renderer::RootSignature::Initialize(UINT cbCount)
{
	paramenters.resize(cbCount);

	for (UINT i = 0; i < cbCount; i++)
	{
		paramenters[i].InitAsConstantBufferView(i);
	}

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	m_rootSignatureDesc.Init_1_1(cbCount, paramenters.data(), 0, nullptr, rootSignatureFlags);
}
void Renderer::RootSignature::Initialize1(UINT cbCount)
{
	
}
void Renderer::RootSignature::InitializeDoubleSrvHeap(UINT srvCount1, UINT srvCount2, UINT cbCount, 
	std::vector<D3D12_STATIC_SAMPLER_DESC> & sampler)
{
	rangeTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srvCount1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
	rangeTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srvCount2, srvCount1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
	
	paramenters.resize(cbCount + 2);
	paramenters[0].InitAsDescriptorTable(1, &rangeTable, D3D12_SHADER_VISIBILITY_PIXEL);
	paramenters[1].InitAsDescriptorTable(1, &rangeTable2, D3D12_SHADER_VISIBILITY_PIXEL);

	for (UINT i = 2; i <= cbCount+1; i++)
	{
		paramenters[i].InitAsConstantBufferView(i - 2);
	}

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	m_samplerArray = sampler;
	if (m_samplerArray.empty()) {
		m_rootSignatureDesc.Init_1_1(cbCount + 2, paramenters.data(), 0 , nullptr, rootSignatureFlags);
	}
	else
	{
		m_rootSignatureDesc.Init_1_1(cbCount + 2, paramenters.data(), (UINT)m_samplerArray.size(),
			m_samplerArray.data(), rootSignatureFlags);
	}
}
void Renderer::RootSignature::Finalize(Microsoft::WRL::ComPtr<ID3D12Device5>& device)
{
	
	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&m_rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), error.GetAddressOf()));
	device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

}
