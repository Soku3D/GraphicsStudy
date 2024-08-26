#include "PipelineState.h"
#include "Utility.h"

namespace Renderer {
    GraphicsPSO::GraphicsPSO(const char* Name)
        : PSO(Name)
    {
        ZeroMemory(&m_psoDesc, sizeof(m_psoDesc));
        m_psoDesc.NodeMask = 1;
        m_psoDesc.SampleMask = 0xFFFFFFFFu;
        m_psoDesc.SampleDesc.Count = 1;
        m_psoDesc.InputLayout.NumElements = 0;
    }

    

    GraphicsPSO::GraphicsPSO()
        :PSO("")
    {
        ZeroMemory(&m_psoDesc, sizeof(m_psoDesc));
        m_psoDesc.NodeMask = 1;
        m_psoDesc.SampleMask = 0xFFFFFFFFu;
        m_psoDesc.SampleDesc.Count = 1;
        m_psoDesc.InputLayout.NumElements = 0;
    }

    void GraphicsPSO::operator=(GraphicsPSO& pso)
    {
        this->m_psoDesc = pso.m_psoDesc;
        this->m_rootSignature = pso.m_rootSignature;
        this->m_pso = pso.m_pso;
        //this->m_Name = pso.m_Name;
    }

    void GraphicsPSO::SetBlendState(const D3D12_BLEND_DESC& BlendDesc)
    {
        m_psoDesc.BlendState = BlendDesc;
    }

    void GraphicsPSO::SetRasterizerState(const D3D12_RASTERIZER_DESC& RasterizerDesc)
    {
        m_psoDesc.RasterizerState = RasterizerDesc;
    }

    void GraphicsPSO::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc)
    {
        m_psoDesc.DepthStencilState = DepthStencilDesc;
    }

    void GraphicsPSO::SetSampleMask(UINT SampleMask)
    {
        m_psoDesc.SampleMask = SampleMask;
    }

    void GraphicsPSO::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType)
    {
        m_psoDesc.PrimitiveTopologyType = TopologyType;
    }

    void GraphicsPSO::SetPrimitiveRestart(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBProps)
    {
        m_psoDesc.IBStripCutValue = IBProps;
    }

    void GraphicsPSO::SetDepthTargetFormat(DXGI_FORMAT DSVFormat, UINT MsaaCount, UINT MsaaQuality)
    {
        SetRenderTargetFormats(0, nullptr, DSVFormat, MsaaCount, MsaaQuality);
    }

    void GraphicsPSO::SetRenderTargetFormat(DXGI_FORMAT RTVFormat, DXGI_FORMAT DSVFormat, UINT MsaaCount, UINT MsaaQuality)
    {
        SetRenderTargetFormats(1, &RTVFormat, DSVFormat, MsaaCount, MsaaQuality);
    }

    void GraphicsPSO::SetRenderTargetFormats(UINT NumRTVs, const DXGI_FORMAT* RTVFormats, DXGI_FORMAT DSVFormat, UINT MsaaCount, UINT MsaaQuality)
    {
        for (UINT i = 0; i < NumRTVs; ++i)
        {
            m_psoDesc.RTVFormats[i] = RTVFormats[i];
        }
        for (UINT i = NumRTVs; i < m_psoDesc.NumRenderTargets; ++i)
            m_psoDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
        m_psoDesc.NumRenderTargets = NumRTVs;
        m_psoDesc.DSVFormat = DSVFormat;
        m_psoDesc.SampleDesc.Count = MsaaCount;
        m_psoDesc.SampleDesc.Quality = MsaaQuality;
    }

    void GraphicsPSO::SetInputLayout(UINT NumElements, const D3D12_INPUT_ELEMENT_DESC* pInputElementDescs)
    {
        m_psoDesc.InputLayout.NumElements = NumElements;
       
        if (NumElements > 0)
        {
            m_psoDesc.InputLayout.pInputElementDescs = pInputElementDescs;
        }
        else
            m_psoDesc.InputLayout.pInputElementDescs = nullptr;
    }
    void GraphicsPSO::Finalize(Microsoft::WRL::ComPtr<ID3D12Device5>& device)
    {
        if (m_rootSignature != nullptr){
            m_psoDesc.pRootSignature = m_rootSignature->Get();
        }
        ThrowIfFailed(device->CreateGraphicsPipelineState(&m_psoDesc, IID_PPV_ARGS(&m_pso)));
    }

    void GraphicsPSO::PrintMSAAData()
    {
        std::cout << "Count : " << m_psoDesc.SampleDesc.Count << "Quality : " << m_psoDesc.SampleDesc.Quality << std::endl;
    }

    ComputePSO::ComputePSO(const char* Name)
        :PSO(Name)
    {
        ZeroMemory(&m_psoDesc, sizeof(m_psoDesc));
    }

    ComputePSO::ComputePSO()
        :PSO("")
    {
        ZeroMemory(&m_psoDesc, sizeof(m_psoDesc));
    }

    void ComputePSO::operator=(ComputePSO& pso)
    {
        this->m_psoDesc = pso.m_psoDesc;
        this->m_rootSignature = pso.m_rootSignature;
        this->m_pso = pso.m_pso;
        //this->m_Name = pso.m_Name;
    }

    void ComputePSO::Finalize(Microsoft::WRL::ComPtr<ID3D12Device5>& device)
    {
        if (m_rootSignature != nullptr) {
            m_psoDesc.pRootSignature = m_rootSignature->Get();
        }
        ThrowIfFailed(device->CreateComputePipelineState(&m_psoDesc, IID_PPV_ARGS(&m_pso)));
    }

}

