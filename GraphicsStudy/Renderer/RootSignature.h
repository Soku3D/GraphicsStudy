#pragma once
#include "wrl.h"
#include "d3d12.h"
#include "d3dx12.h"

namespace Renderer {
	class RootSignature {
	public:
		RootSignature(): m_sampler(CD3DX12_STATIC_SAMPLER_DESC()){};
		~RootSignature() {}
		void Initialize(UINT srvCount, UINT uavCount, UINT cbCount, D3D12_STATIC_SAMPLER_DESC& sampler);
		void Initialize(UINT srvCount, UINT cbCount, D3D12_STATIC_SAMPLER_DESC& sampler);
		void Finalize(Microsoft::WRL::ComPtr<ID3D12Device>& device);
		ID3D12RootSignature* Get()const { return m_rootSignature.Get(); }
		
	private:
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		D3D12_STATIC_SAMPLER_DESC m_sampler;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC m_rootSignatureDesc;
		std::vector<CD3DX12_ROOT_PARAMETER1> paramenters;
		CD3DX12_DESCRIPTOR_RANGE1 rangeTable;
	};
}