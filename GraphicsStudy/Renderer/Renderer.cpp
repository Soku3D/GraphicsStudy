#include "Renderer.h"

#include "TestVS.h"
#include "TestPS.h"

namespace Renderer {

	GraphicsPSO defaultPso("Default");
	
	std::map<std::string, GraphicsPSO > psoList;
	std::vector<std::string> psoListNames;

	RootSignature defaultSignature;
	std::vector<D3D12_INPUT_ELEMENT_DESC> defaultElement;
	D3D12_STATIC_SAMPLER_DESC defaultSampler;

	D3D12_RASTERIZER_DESC defaultRasterizer;
	D3D12_RASTERIZER_DESC wireRasterizer;

	D3D12_BLEND_DESC defaultBlender;
	D3D12_BLEND_DESC alphaBlender;

	void Initialize(void)
	{
		GraphicsPSO wirePso("Wire");

		defaultElement =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};
		
		defaultRasterizer = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		wireRasterizer = defaultRasterizer;
		wireRasterizer.FillMode = D3D12_FILL_MODE_WIREFRAME;

		defaultBlender = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		alphaBlender = defaultBlender;
		alphaBlender.RenderTarget[0].BlendEnable = true;
		alphaBlender.RenderTarget[0].BlendEnable = TRUE;
		alphaBlender.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		alphaBlender.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		alphaBlender.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		alphaBlender.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		alphaBlender.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		alphaBlender.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		alphaBlender.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		defaultSampler = {};
		defaultSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		defaultSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		defaultSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		defaultSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		defaultSampler.MipLODBias = 0;
		defaultSampler.MaxAnisotropy = 0;
		defaultSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		defaultSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		defaultSampler.MinLOD = 0.0f;
		defaultSampler.MaxLOD = D3D12_FLOAT32_MAX;
		defaultSampler.ShaderRegister = 0;
		defaultSampler.RegisterSpace = 0;
		defaultSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		defaultSignature.Initialize(1, 2, defaultSampler);

		defaultPso.SetVertexShader(g_pTestVS, sizeof(g_pTestVS));
		defaultPso.SetPixelShader(g_pTestPS, sizeof(g_pTestPS));
		defaultPso.SetInputLayout(defaultElement.size(), defaultElement.data());
		defaultPso.SetRasterizerState(CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT));
		
		defaultPso.SetBlendState(alphaBlender);
		defaultPso.SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
		defaultPso.SetSampleMask(UINT_MAX);
		defaultPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		defaultPso.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT);
		defaultPso.SetRootSignature(&defaultSignature);

		wirePso = defaultPso;
		wirePso.SetRasterizerState(wireRasterizer);

		psoList[defaultPso.GetName()] = defaultPso;
		psoList[wirePso.GetName()] = wirePso;
	}

	void Finalize(Microsoft::WRL::ComPtr<ID3D12Device>& device)
	{
		defaultSignature.Finalize(device);

		for (auto& pso : psoList) {
			pso.second.Finalize(device);
			psoListNames.push_back(pso.first);
		}
		
	
	}

}


