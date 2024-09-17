#pragma once
#include "wrl.h"
#include "d3d12.h"
#include "d3dx12.h"

namespace Renderer {
	class RootSignature {
	public:
		RootSignature(): m_sampler(&CD3DX12_STATIC_SAMPLER_DESC()){};
		~RootSignature() {}
		void Initialize(UINT srvCount, UINT uavCount, UINT cbCount, std::vector<D3D12_STATIC_SAMPLER_DESC>& sampler);
		void InitializeRaytracing(UINT uavCount, UINT srvCount, UINT cbCount, int numSamplers, D3D12_STATIC_SAMPLER_DESC* sampler);
		//void InitializeSrv(UINT uavCount, UINT srvCount, UINT cbCount, int numSamplers, D3D12_STATIC_SAMPLER_DESC* sampler);
		void InitializeUAV(UINT uavCount, UINT cbCount, int numSamplers = 0, D3D12_STATIC_SAMPLER_DESC* sampler = nullptr);
		void Initialize(UINT srvCount, UINT cbCount, int numSamplers = 0, D3D12_STATIC_SAMPLER_DESC* sampler = nullptr);
		void Initialize(UINT srvCount, UINT cbCount, std::vector<D3D12_STATIC_SAMPLER_DESC>& sampler);
		void Initialize(UINT cbCount);
		void Initialize1(UINT cbCount);
		void InitializeDoubleSrvHeap(UINT srvCount1, UINT srvCount2, UINT cbCount, std::vector<D3D12_STATIC_SAMPLER_DESC>& sampler);
		void Finalize(Microsoft::WRL::ComPtr<ID3D12Device5>& device);
		ID3D12RootSignature* Get()const { return m_rootSignature.Get(); }
		ID3D12RootSignature*const* GetAddressOf()const { return m_rootSignature.GetAddressOf(); }
	private:
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		D3D12_STATIC_SAMPLER_DESC* m_sampler;
		std::vector<D3D12_STATIC_SAMPLER_DESC> m_samplerArray;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC m_rootSignatureDesc;
		std::vector<CD3DX12_ROOT_PARAMETER1> paramenters;
		CD3DX12_DESCRIPTOR_RANGE1 rangeTable;
		CD3DX12_DESCRIPTOR_RANGE1 rangeTable2;
		CD3DX12_DESCRIPTOR_RANGE1 table[2];
	};
}