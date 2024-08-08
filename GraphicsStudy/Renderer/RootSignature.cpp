#include "RootSignature.h"
#include "Utility.h"

void Renderer::RootSignature::Initialize(UINT srvCount, UINT uavCount, UINT cbCount, D3D12_STATIC_SAMPLER_DESC& sampler)
{
	CD3DX12_DESCRIPTOR_RANGE1 rangeTable[2];
	rangeTable[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srvCount, 0);
	rangeTable[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, uavCount, 0);

	paramenters.resize(cbCount + 1);
	paramenters[0].InitAsDescriptorTable(2, rangeTable, D3D12_SHADER_VISIBILITY_PIXEL);

	for (UINT i = 1; i <= cbCount; i++)
	{
		paramenters[i].InitAsConstantBufferView(i - 1);
	}
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	
	m_sampler = sampler;

	m_rootSignatureDesc.Init_1_1(cbCount + 1, paramenters.data(), 1, &m_sampler, rootSignatureFlags);
}

void Renderer::RootSignature::Initialize(UINT srvCount, UINT cbCount, D3D12_STATIC_SAMPLER_DESC& sampler)
{
	rangeTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	paramenters.resize(cbCount+1);
	paramenters[0].InitAsDescriptorTable(1, &rangeTable, D3D12_SHADER_VISIBILITY_PIXEL);

	for (UINT i = 1; i <= cbCount; i++)
	{
		paramenters[i].InitAsConstantBufferView(i - 1);
	}

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	m_rootSignatureDesc.Init_1_1(cbCount + 1, paramenters.data(), 1, &m_sampler, rootSignatureFlags);

	m_sampler = sampler;
}

void Renderer::RootSignature::Finalize(Microsoft::WRL::ComPtr<ID3D12Device>& device)
{
	
	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&m_rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), error.GetAddressOf()));
	device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

}
