#pragma once
#include "wrl.h"
#include "d3d12.h"
#include "RootSignature.h"
#include "d3dx12.h"

namespace Renderer {

    class PSO {
    public:
        PSO(const char* Name) : m_Name(Name), m_rootSignature(nullptr), m_pso(nullptr) {}

        void SetRootSignature(RootSignature* bindMappings)
        {
            m_rootSignature = bindMappings;
        }
        std::string GetName() { return std::string(m_Name); }
        ID3D12RootSignature* GetRootSignature(void) const
        {
            return m_rootSignature->Get();
        }

        ID3D12PipelineState* GetPipelineStateObject(void) const { return m_pso.Get(); }

    protected:
        const char* m_Name;
        RootSignature* m_rootSignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;
    };

    class GraphicsPSO : public PSO {
    public:
        GraphicsPSO(const char* Name);
        GraphicsPSO();
        ~GraphicsPSO() {}

        void operator=(GraphicsPSO& pso);
        void SetBlendState(const D3D12_BLEND_DESC& BlendDesc);
        void SetRasterizerState(const D3D12_RASTERIZER_DESC& RasterizerDesc);
        void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc);
        void SetSampleMask(UINT SampleMask);
        void SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType);
        void SetDepthTargetFormat(DXGI_FORMAT DSVFormat, UINT MsaaCount = 1, UINT MsaaQuality = 0);
        void SetRenderTargetFormat(DXGI_FORMAT RTVFormat, DXGI_FORMAT DSVFormat, UINT MsaaCount = 1, UINT MsaaQuality = 0);
        void SetRenderTargetFormats(UINT NumRTVs, const DXGI_FORMAT* RTVFormats, DXGI_FORMAT DSVFormat, UINT MsaaCount = 1, UINT MsaaQuality = 0);
        void SetInputLayout(UINT NumElements, const D3D12_INPUT_ELEMENT_DESC* pInputElementDescs);
        void SetPrimitiveRestart(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBProps);

        void SetVertexShader(const void* Binary, size_t Size) { m_psoDesc.VS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
        void SetPixelShader(const void* Binary, size_t Size) { m_psoDesc.PS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
        void SetGeometryShader(const void* Binary, size_t Size) { m_psoDesc.GS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
        void SetHullShader(const void* Binary, size_t Size) { m_psoDesc.HS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
        void SetDomainShader(const void* Binary, size_t Size) { m_psoDesc.DS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }

        void SetVertexShader(const D3D12_SHADER_BYTECODE& Binary) { m_psoDesc.VS = Binary; }
        void SetPixelShader(const D3D12_SHADER_BYTECODE& Binary) { m_psoDesc.PS = Binary; }
        void SetGeometryShader(const D3D12_SHADER_BYTECODE& Binary) { m_psoDesc.GS = Binary; }
        void SetHullShader(const D3D12_SHADER_BYTECODE& Binary) { m_psoDesc.HS = Binary; }
        void SetDomainShader(const D3D12_SHADER_BYTECODE& Binary) { m_psoDesc.DS = Binary; }

        // Perform validation and compute a hash value for fast state block comparisons
        void Finalize(Microsoft::WRL::ComPtr<ID3D12Device>& device);
        void PrintMSAAData();

    private:
        D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc;
    };

    class ComputePSO : public PSO {
    public:
        ComputePSO(const char* Name);
        ComputePSO();
        ~ComputePSO() {}

        void operator=(ComputePSO& pso);
        void Finalize(Microsoft::WRL::ComPtr<ID3D12Device>& device);
        void SetComputeShader(const D3D12_SHADER_BYTECODE& Binary) { m_psoDesc.CS = Binary; }
        void SetComputeShader(const void* Binary, size_t Size) { m_psoDesc.CS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }

    private:
        D3D12_COMPUTE_PIPELINE_STATE_DESC m_psoDesc;
    };
}