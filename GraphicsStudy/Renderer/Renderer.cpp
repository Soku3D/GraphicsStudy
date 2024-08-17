#include "Renderer.h"

#include "TestVS.h"
#include "TestPS.h"
#include "TestCS.h"

#include "CubeMapVS.h"
#include "CubeMapPS.h"

#include "CopyVS.h"
#include "CopyPS.h"

#include "GeometryPassVS.h"
#include "GeometryPassPS.h"

#include "LightPassVS.h"
#include "LightPassPS.h"

#include "DrawNormalPassVS.h"
#include "DrawNormalPassGS.h"
#include "DrawNormalPassPS.h"


namespace Renderer {
	DXGI_FORMAT backbufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	//DXGI_FORMAT backbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT hdrFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	DXGI_FORMAT geometryPassFormats[geometryPassNum] =
	{
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_R16G16B16A16_FLOAT
	};

	UINT msaaCount = 8;
	UINT msaaQuality = 0;

	GraphicsPSO defaultPso("Default");
	ComputePSO computePso("Compute");

	std::map<std::string, GraphicsPSO > modePsoLists;
	std::map<std::string, GraphicsPSO > passPsoLists;
	std::map<std::string, GraphicsPSO > cubePsoLists;
	std::map<std::string, GraphicsPSO > utilityPsoLists;

	std::vector<std::string> modePsoListNames;
	std::vector<std::string> passPsoListNames; 
	std::vector<std::string> cubePsoListNames;
	std::vector<std::string> utilityPsoListNames;

	std::map<std::string, ComputePSO > computePsoList;
	std::vector<std::string> computePsoListNames;

	RootSignature defaultSignature;
	RootSignature geometryPassSignature;
	RootSignature computeSignature;
	RootSignature cubeMapSignature;
    RootSignature copySignature;
	RootSignature lightPassSignature;
	RootSignature NormalPassSignature;


	std::vector<D3D12_INPUT_ELEMENT_DESC> defaultElement;
	std::vector<D3D12_INPUT_ELEMENT_DESC> simpleElement;
	std::vector<D3D12_INPUT_ELEMENT_DESC> pbrElement;

	D3D12_STATIC_SAMPLER_DESC wrapLinearSampler;
	D3D12_STATIC_SAMPLER_DESC clampLinearSampler;
	std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;

	D3D12_RASTERIZER_DESC defaultRasterizer;
	D3D12_RASTERIZER_DESC wireRasterizer;

	D3D12_BLEND_DESC defaultBlender;
	D3D12_BLEND_DESC alphaBlender;

	void Initialize(void)
	{

		wrapLinearSampler = {};
		wrapLinearSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		wrapLinearSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		wrapLinearSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		wrapLinearSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		wrapLinearSampler.MipLODBias = 0;
		wrapLinearSampler.MaxAnisotropy = 1;
		wrapLinearSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		wrapLinearSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		wrapLinearSampler.MinLOD = 0.0f;
		wrapLinearSampler.MaxLOD = D3D12_FLOAT32_MAX;
		wrapLinearSampler.ShaderRegister = 0;
		wrapLinearSampler.RegisterSpace = 0;
		wrapLinearSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		clampLinearSampler = wrapLinearSampler;
		clampLinearSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		clampLinearSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		clampLinearSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

		//samplers.push_back(wrapLinearSampler);
		//samplers.push_back(clampLinearSampler);
		// Init Signatures
		defaultSignature.Initialize(1, 3, 1, &wrapLinearSampler);
		computeSignature.InitializeUAV(1, 1, 0, nullptr);
		cubeMapSignature.Initialize(1, 2, 1, &wrapLinearSampler);
		copySignature.Initialize(1, 0, 1, &wrapLinearSampler);
		
		geometryPassSignature.Initialize(2, 2, 1, &wrapLinearSampler);
		lightPassSignature.InitializeDoubleSrvHeap(4, 4, 1, &wrapLinearSampler);
		NormalPassSignature.Initialize(2);

		GraphicsPSO msaaPso("Msaa");
		GraphicsPSO wirePso("Wire");

		GraphicsPSO cubeMapPso("DefaultCubeMap");
		GraphicsPSO msaaCubeMapPso("MsaaCubeMap");
		GraphicsPSO wireCubeMapPso("WireCubeMap");
		
		GraphicsPSO defaultGeometryPassPso("DefaultGeometryPass");
		GraphicsPSO msaaGeometryPassPso("MsaaGeometryPass");
		GraphicsPSO wireGeometryPassPso("WireGeometryPass");

		GraphicsPSO lightPassPso("LightPass");

		GraphicsPSO DrawNormalPassPso("NormalPass");

		GraphicsPSO copyPso("Copy");
		
				
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
		pbrElement =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		defaultRasterizer = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		wireRasterizer = defaultRasterizer;
		wireRasterizer.FillMode = D3D12_FILL_MODE_WIREFRAME;
		
		//D3D12_DEPTH_STENCILOP_DESC depthStencilDesc;

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
		defaultPso.SetRenderTargetFormat(hdrFormat, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, 0);
		defaultPso.SetRootSignature(&defaultSignature);
		
		defaultGeometryPassPso = defaultPso;
		defaultGeometryPassPso.SetRootSignature(&geometryPassSignature);
		defaultGeometryPassPso.SetVertexShader(g_pGeometryPassVS, sizeof(g_pGeometryPassVS));
		defaultGeometryPassPso.SetPixelShader(g_pGeometryPassPS, sizeof(g_pGeometryPassPS));
		defaultGeometryPassPso.SetRenderTargetFormats(geometryPassNum, geometryPassFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, 0);
		defaultGeometryPassPso.SetInputLayout((UINT)pbrElement.size(), pbrElement.data());

		msaaGeometryPassPso = defaultGeometryPassPso;
		msaaGeometryPassPso.SetRenderTargetFormats(geometryPassNum, geometryPassFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, msaaCount, msaaQuality - 1);

		wireGeometryPassPso = defaultGeometryPassPso;
		wireGeometryPassPso.SetRasterizerState(wireRasterizer);

		lightPassPso = defaultPso;
		lightPassPso.SetRootSignature(&lightPassSignature);
		lightPassPso.SetVertexShader(g_pLightPassVS, sizeof(g_pLightPassVS));
		lightPassPso.SetPixelShader(g_pLightPassPS, sizeof(g_pLightPassPS));
		lightPassPso.SetInputLayout((UINT)simpleElement.size(), simpleElement.data());
		lightPassPso.SetRenderTargetFormat(hdrFormat, DXGI_FORMAT_UNKNOWN, 1, 0);

		DrawNormalPassPso = defaultPso;
		DrawNormalPassPso.SetVertexShader(g_pDrawNormalPassVS, sizeof(g_pDrawNormalPassVS));
		DrawNormalPassPso.SetPixelShader(g_pDrawNormalPassPS, sizeof(g_pDrawNormalPassPS));
		DrawNormalPassPso.SetInputLayout((UINT)pbrElement.size(), pbrElement.data());
		DrawNormalPassPso.SetGeometryShader(g_pDrawNormalPassGS, sizeof(g_pDrawNormalPassGS));
		DrawNormalPassPso.SetRenderTargetFormat(hdrFormat, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, 0);
		DrawNormalPassPso.SetRootSignature(&NormalPassSignature);
		DrawNormalPassPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);

		computePso.SetRootSignature(&computeSignature);
		computePso.SetComputeShader(g_pTestCS, sizeof(g_pTestCS));

		msaaPso = defaultPso;
		msaaPso.SetRenderTargetFormat(hdrFormat, DXGI_FORMAT_D24_UNORM_S8_UINT, msaaCount, msaaQuality - 1);
		
		wirePso = defaultPso;
		wirePso.SetRasterizerState(wireRasterizer);

		cubeMapPso = defaultPso;
		cubeMapPso.SetInputLayout((UINT)simpleElement.size(), simpleElement.data());
		cubeMapPso.SetRootSignature(&cubeMapSignature);
		cubeMapPso.SetVertexShader(g_pCubeMapVS, sizeof(g_pCubeMapVS));
		cubeMapPso.SetPixelShader(g_pCubeMapPS, sizeof(g_pCubeMapPS));
		cubeMapPso.SetRasterizerState(cubeMapRasterizer);
		cubeMapPso.SetRenderTargetFormat(hdrFormat, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, 0);

		wireCubeMapPso = cubeMapPso;
		wireCubeMapPso.SetRasterizerState(wireRasterizer);

		msaaCubeMapPso = cubeMapPso;
		//msaaCubeMapPso.SetRenderTargetFormat(hdrFormat, DXGI_FORMAT_D24_UNORM_S8_UINT, msaaCount, msaaQuality - 1);
		msaaCubeMapPso.SetRenderTargetFormat(hdrFormat, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, 0);

		copyPso = defaultPso;
		copyPso.SetRenderTargetFormat(backbufferFormat, DXGI_FORMAT_UNKNOWN, 1, 0);
		copyPso.SetRootSignature(&copySignature);
		copyPso.SetVertexShader(g_pCopyVS, sizeof(g_pCopyVS));
		copyPso.SetPixelShader(g_pCopyPS, sizeof(g_pCopyPS));

		modePsoLists[defaultPso.GetName()] = defaultPso;
		modePsoLists[wirePso.GetName()] = wirePso;
		modePsoLists[msaaPso.GetName()] = msaaPso;
		
		passPsoLists[defaultGeometryPassPso.GetName()] = defaultGeometryPassPso;
		passPsoLists[wireGeometryPassPso.GetName()] = wireGeometryPassPso;
		passPsoLists[msaaGeometryPassPso.GetName()] = msaaGeometryPassPso;
		passPsoLists[lightPassPso.GetName()] = lightPassPso;
		passPsoLists[DrawNormalPassPso.GetName()] = DrawNormalPassPso;

		utilityPsoLists[copyPso.GetName()] = copyPso;
		
		cubePsoLists[cubeMapPso.GetName()] = cubeMapPso;
		cubePsoLists[msaaCubeMapPso.GetName()] = msaaCubeMapPso;
		cubePsoLists[wireCubeMapPso.GetName()] = wireCubeMapPso;

		computePsoList[computePso.GetName()] = computePso;

	}

	void Finalize(Microsoft::WRL::ComPtr<ID3D12Device>& device)
	{
		defaultSignature.Finalize(device);
		computeSignature.Finalize(device);
		cubeMapSignature.Finalize(device);
		copySignature.Finalize(device);
		geometryPassSignature.Finalize(device);
		lightPassSignature.Finalize(device);
		NormalPassSignature.Finalize(device);

		for (auto& pso : modePsoLists) {
			pso.second.Finalize(device);
			modePsoListNames.push_back(pso.first);
		}
		for (auto& pso : cubePsoLists) {
			pso.second.Finalize(device);
			cubePsoListNames.push_back(pso.first);
		}
		for (auto& pso : passPsoLists) {
			pso.second.Finalize(device);
			passPsoListNames.push_back(pso.first);
		}
		for (auto& pso : utilityPsoLists) {
			pso.second.Finalize(device);
			utilityPsoListNames.push_back(pso.first);
		}
		for (auto& pso : computePsoList) {
			pso.second.Finalize(device);
			computePsoListNames.push_back(pso.first);
		}
	}
}

