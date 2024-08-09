#include "Renderer.h"

#include "TestVS.h"
#include "TestPS.h"
#include "TestCS.h"

#include "CubeMapVS.h"
#include "CubeMapPS.h"

namespace Renderer {

	UINT msaaCount = 4;
	UINT msaaQuality = 0;

	GraphicsPSO defaultPso("Default");
	ComputePSO computePso("Compute");

	std::map<std::string, GraphicsPSO > grphicsPsoList;
	std::vector<std::string> graphicsPsoListNames;
	std::vector<std::string> cubePsoListNames;

	std::map<std::string, GraphicsPSO > cubePsoList;

	std::map<std::string, ComputePSO > computePsoList;
	std::vector<std::string> computePsoListNames;

	RootSignature defaultSignature;
	RootSignature computeSignature;
	RootSignature cubeMapSignature;

	std::vector<D3D12_INPUT_ELEMENT_DESC> defaultElement;
	std::vector<D3D12_INPUT_ELEMENT_DESC> simpleElement;

	D3D12_STATIC_SAMPLER_DESC defaultSampler;

	D3D12_RASTERIZER_DESC defaultRasterizer;
	D3D12_RASTERIZER_DESC wireRasterizer;

	D3D12_BLEND_DESC defaultBlender;
	D3D12_BLEND_DESC alphaBlender;

	void Initialize(void)
	{
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

		defaultSignature.Initialize(1, 3, &defaultSampler);
		computeSignature.InitializeUAV(1, 0, nullptr);
		cubeMapSignature.Initialize(1, 1, &defaultSampler);

		GraphicsPSO msaaPso("Msaa");
		GraphicsPSO wirePso("Wire");
		GraphicsPSO cubeMapPso("DefaultCubeMap");
		GraphicsPSO msaaCubeMapPso("MsaaCubeMap");
		GraphicsPSO wireCubeMapPso("WireCubeMap");
		
		defaultElement =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};
		simpleElement =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		defaultRasterizer = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		wireRasterizer = defaultRasterizer;
		wireRasterizer.FillMode = D3D12_FILL_MODE_WIREFRAME;
		
		D3D12_RASTERIZER_DESC cubeMapRasterizer = defaultRasterizer;
		//cubeMapRasterizer.FrontCounterClockwise = TRUE;
		//cubeMapRasterizer.FillMode = D3D12_FILL_MODE_WIREFRAME;
		//cubeMapRasterizer.CullMode = D3D12_CULL_MODE_BACK;

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

		defaultPso.SetVertexShader(g_pTestVS, sizeof(g_pTestVS));
		defaultPso.SetPixelShader(g_pTestPS, sizeof(g_pTestPS));
		defaultPso.SetInputLayout((UINT)defaultElement.size(), defaultElement.data());
		defaultPso.SetRasterizerState(CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT));
		
		defaultPso.SetBlendState(alphaBlender);
		defaultPso.SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
		defaultPso.SetSampleMask(UINT_MAX);
		defaultPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		defaultPso.SetRenderTargetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, 0);
		defaultPso.SetRootSignature(&defaultSignature);
		
		computePso.SetRootSignature(&computeSignature);
		computePso.SetComputeShader(g_pTestCS, sizeof(g_pTestCS));

		msaaPso = defaultPso;
		msaaPso.SetRenderTargetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT, msaaCount, msaaQuality - 1);
		
		wirePso = defaultPso;
		wirePso.SetRasterizerState(wireRasterizer);

		cubeMapPso = defaultPso;
		cubeMapPso.SetInputLayout((UINT)simpleElement.size(), simpleElement.data());
		cubeMapPso.SetRootSignature(&cubeMapSignature);
		cubeMapPso.SetVertexShader(g_pCubeMapVS, sizeof(g_pCubeMapVS));
		cubeMapPso.SetPixelShader(g_pCubeMapPS, sizeof(g_pCubeMapPS));
		cubeMapPso.SetRasterizerState(cubeMapRasterizer);
		cubeMapPso.SetRenderTargetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, 0);

		wireCubeMapPso = cubeMapPso;
		wireCubeMapPso.SetRasterizerState(wireRasterizer);

		msaaCubeMapPso = cubeMapPso;
		msaaCubeMapPso.SetRenderTargetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT, msaaCount, msaaQuality - 1);

		grphicsPsoList[defaultPso.GetName()] = defaultPso;
		grphicsPsoList[wirePso.GetName()] = wirePso;
		grphicsPsoList[msaaPso.GetName()] = msaaPso;
		
		cubePsoList[cubeMapPso.GetName()] = cubeMapPso;
		cubePsoList[msaaCubeMapPso.GetName()] = msaaCubeMapPso;
		cubePsoList[wireCubeMapPso.GetName()] = wireCubeMapPso;

		computePsoList[computePso.GetName()] = computePso;

	}

	void Finalize(Microsoft::WRL::ComPtr<ID3D12Device>& device)
	{
		defaultSignature.Finalize(device);
		computeSignature.Finalize(device);
		cubeMapSignature.Finalize(device);

		for (auto& pso : grphicsPsoList) {
			pso.second.Finalize(device);
			graphicsPsoListNames.push_back(pso.first);
		}
		for (auto& pso : cubePsoList) {
			pso.second.Finalize(device);
			cubePsoListNames.push_back(pso.first);
		}

		for (auto& pso : computePsoList) {
			pso.second.Finalize(device);
			computePsoListNames.push_back(pso.first);
		}
	}
}
