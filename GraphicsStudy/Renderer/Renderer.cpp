#include "Renderer.h"

#include "TestVS.h"
#include "TestPS.h"
#include "PostprocessingCS.h"
#include "PerlinNoiseCS.h"

#include "CubeMapVS.h"
#include "CubeMapPS.h"

#include "CopyVS.h"
#include "CopyPS.h"

#include "GeometryPassVS.h"
#include "GeometryPassTesslationVS.h"
#include "GeometryPassHS.h"
#include "GeometryPassDS.h"
#include "GeometryPassTriangleHS.h"
#include "GeometryPassTriangleDS.h"
#include "GeometryPassPS.h"
#include "FbxGeometryPassVS.h"

#include "LightPassVS.h"
#include "LightPassPS.h"

#include "DrawNormalPassVS.h"
#include "DrawNormalPassGS.h"
#include "DrawNormalPassPS.h"

#include "RenderBoundingBoxPassVS.h"
#include "RenderBoundingBoxPassPS.h"
#include "RenderBoundingBoxPassGS.h"


#include "SimulationParticlesVS.h"
#include "SimulationParticlesGS.h"
#include "SimulationParticlesPS.h"
#include "SimulationParticlesCS.h"
#include "SimulationPostProcessingCS.h"

#include "SphSimulationParticlesCS.h"
#include "SphComputeRhoCS.h"
#include "SphComputeForcesCS.h"

#include "CFDSourcingCS.h"
#include "CFDAdvectionCS.h"
#include "CFDComputePressureCS.h"
#include "CFDComputeDivergenceCS.h"
#include "CFDApplyPressureCS.h"
#include "CFDComputeDiffuseCS.h"
#include "CFDComputeVorticityCS.h"
#include "CFDVorticityConfinementCS.h"

namespace Renderer {
	//DXGI_FORMAT backbufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	DXGI_FORMAT backbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT hdrFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	DXGI_FORMAT geometryPassFormats[geometryPassNum] =
	{
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT
	};
	DXGI_FORMAT cubeMapPassFormats[2] =
	{
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
	
	};
	UINT msaaCount = 4;
	UINT msaaQuality = 0;

	GraphicsPSO defaultPso("Default");
	ComputePSO postProcessingPso("PostProcessing");

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
	RootSignature cubeMapSignature;
	RootSignature copySignature;
	RootSignature lightPassSignature;
	RootSignature NormalPassSignature;

	RootSignature computeSignature;
	RootSignature simulationComputeSignature;
	RootSignature simulationSignature;
	RootSignature simulationPostProcessingSignature;

	RootSignature cfdSourcingSignature;
	RootSignature cfdComputeDivergenceSignature;
	RootSignature cfdComputePressureSignature;
	RootSignature cfdAdvectionSignature;
	RootSignature cfdApplyPressureSignature;
	RootSignature cfdComputeDiffuseSignature;
	RootSignature cfdVorticitySignature;

	RootSignature raytracingGlobalSignature;

	std::vector<D3D12_INPUT_ELEMENT_DESC> defaultElement;
	std::vector<D3D12_INPUT_ELEMENT_DESC> simpleElement;
	std::vector<D3D12_INPUT_ELEMENT_DESC> pbrElement;

	D3D12_STATIC_SAMPLER_DESC wrapLinearSampler;
	D3D12_STATIC_SAMPLER_DESC wrapPointSampler;
	D3D12_STATIC_SAMPLER_DESC clampLinearSampler;
	D3D12_STATIC_SAMPLER_DESC testSampler;

	std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;
	std::vector<D3D12_STATIC_SAMPLER_DESC> wrapSamplers;

	D3D12_RASTERIZER_DESC defaultRasterizer;
	D3D12_RASTERIZER_DESC wireRasterizer;

	D3D12_BLEND_DESC defaultBlender;
	D3D12_BLEND_DESC alphaBlender;
	D3D12_BLEND_DESC simulationBlender;
	D3D12_BLEND_DESC addColorBlender;

	void Initialize(void)
	{

		wrapLinearSampler = {};
		wrapLinearSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		wrapLinearSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		wrapLinearSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		wrapLinearSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		wrapLinearSampler.MipLODBias = 0;
		wrapLinearSampler.MaxAnisotropy = 0;
		wrapLinearSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		wrapLinearSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		wrapLinearSampler.MinLOD = 0.0f;
		wrapLinearSampler.MaxLOD = D3D12_FLOAT32_MAX;
		wrapLinearSampler.ShaderRegister = 0;
		wrapLinearSampler.RegisterSpace = 0;
		wrapLinearSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		
		wrapPointSampler = wrapLinearSampler;
		wrapPointSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		wrapPointSampler.ShaderRegister = 1;

		testSampler = wrapLinearSampler;
		testSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		clampLinearSampler = wrapLinearSampler;
		clampLinearSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		clampLinearSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		clampLinearSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		clampLinearSampler.ShaderRegister = 1;

		samplers.push_back(wrapLinearSampler);
		samplers.push_back(clampLinearSampler);

		wrapSamplers.push_back(wrapLinearSampler);
		wrapSamplers.push_back(wrapPointSampler);

		// Init Signatures
		defaultSignature.Initialize(1, 3, 1, &wrapLinearSampler);
		computeSignature.InitializeUAV(1, 0, 0, nullptr);
		//computeSignature.InitializeUAV(1, 0, 0, nullptr);
		cubeMapSignature.Initialize(1, 2, 1, &wrapLinearSampler);
		copySignature.Initialize(1, 0, 1, &wrapLinearSampler);

		geometryPassSignature.Initialize(6, 2, samplers);
		lightPassSignature.InitializeDoubleSrvHeap(4, 4, 1, samplers);
		NormalPassSignature.Initialize(2);
		
		simulationComputeSignature.InitializeUAV(1, 1, 0, nullptr);
		simulationSignature.Initialize(1, 0, 0, nullptr);
		simulationPostProcessingSignature.InitializeUAV(1, 1, 0, nullptr);
		
		cfdSourcingSignature.InitializeUAV(2, 1, 1, &wrapLinearSampler);
		cfdAdvectionSignature.Initialize(2, 2, 1, wrapSamplers);
		cfdComputeDivergenceSignature.Initialize(1, 3, 1, wrapSamplers);
		cfdComputePressureSignature.Initialize(2, 1, 1, wrapSamplers);
		cfdApplyPressureSignature.Initialize(1, 1, 1, wrapSamplers);
		cfdComputeDiffuseSignature.Initialize(2, 2, 1, wrapSamplers);
		cfdVorticitySignature.Initialize(1, 1, 1, wrapSamplers);

		raytracingGlobalSignature.InitializeRaytracing(1, 4, 1, 1, &wrapLinearSampler);

		GraphicsPSO msaaPso("Msaa");
		GraphicsPSO wirePso("Wire");

		GraphicsPSO cubeMapPso("DefaultCubeMap");
		GraphicsPSO msaaCubeMapPso("MsaaCubeMap");
		GraphicsPSO wireCubeMapPso("WireCubeMap");

		GraphicsPSO defaultGeometryPassPso("DefaultGeometryPass");
		GraphicsPSO msaaGeometryPassPso("MsaaGeometryPass");
		GraphicsPSO wireGeometryPassPso("WireGeometryPass");
		GraphicsPSO fbxGeometryPassPso("DefaultFbxGeometryPass");
		GraphicsPSO fbxMsaaGeometryPassPso("MsaaFbxGeometryPass");
		GraphicsPSO fbxWireGeometryPassPso("WireFbxGeometryPass");

		GraphicsPSO defaultLightPassPso("DefaultLightPass");
		GraphicsPSO msaaLightPassPso("MsaaLightPass");

		GraphicsPSO renderNormalPassPso("NormalPass");
		GraphicsPSO renderBoundingBoxPassPso("BoundingBoxPass");

		GraphicsPSO copyPso("Copy");
		GraphicsPSO copyUnormPso("CopyUnorm");
		GraphicsPSO copyDensityPso("CopyDensity");

		ComputePSO simulationPostProcessingPso("SimulationPostProcessing");
		ComputePSO simulationComputePso("SimulationCompute");
		GraphicsPSO simulationRenderPso("SimulationRenderPass");

		ComputePSO sphSimulationComputePso("SphSimulationCompute");
		ComputePSO sphComputeRhoPso("SphComputeRho");
		ComputePSO sphComputeForcesPso("SphComputeForces");

		ComputePSO CFDSourcingPso("CFDSourcing");
		ComputePSO CFDComputePressurePso("CFDComputePressure");
		ComputePSO CFDComputeDivergencePso("CFDComputeDivergence");
		ComputePSO CFDAdvectionPso("CFDAdvection");
		ComputePSO CFDApplyPressurePso("CFDApplyPressure");
		ComputePSO CFDComputeDiffusePso("CFDComputeDiffuse");
		ComputePSO CFDComputeVorticityPso("CFDComputeVorticity");
		ComputePSO CFDVorticityConfinementPso("CFDVorticityConfinement");

		ComputePSO perlinNoisePso("PerlinNoise");

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
		alphaBlender.RenderTarget[0].BlendEnable = TRUE;
		alphaBlender.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		alphaBlender.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		alphaBlender.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		alphaBlender.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		alphaBlender.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		alphaBlender.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		alphaBlender.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		
		simulationBlender = alphaBlender;
		simulationBlender.RenderTarget[0].SrcBlend= D3D12_BLEND_ONE;
		simulationBlender.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		simulationBlender.RenderTarget[0].BlendOp = D3D12_BLEND_OP_MAX;

		addColorBlender = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		addColorBlender.RenderTarget[0].BlendEnable = TRUE;
		addColorBlender.RenderTarget[0].SrcBlend = D3D12_BLEND_BLEND_FACTOR;
		addColorBlender.RenderTarget[0].DestBlend = D3D12_BLEND_BLEND_FACTOR;
		addColorBlender.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		addColorBlender.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		addColorBlender.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		addColorBlender.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		addColorBlender.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

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

		// Geometry Pass
		defaultGeometryPassPso = defaultPso;
		defaultGeometryPassPso.SetRootSignature(&geometryPassSignature);
		defaultGeometryPassPso.SetVertexShader(g_pGeometryPassVS, sizeof(g_pGeometryPassVS));
		//defaultGeometryPassPso.SetHullShader(g_pGeometryPassHS, sizeof(g_pGeometryPassHS));
		//defaultGeometryPassPso.SetDomainShader(g_pGeometryPassDS, sizeof(g_pGeometryPassDS));
		defaultGeometryPassPso.SetPixelShader(g_pGeometryPassPS, sizeof(g_pGeometryPassPS));
		defaultGeometryPassPso.SetRenderTargetFormats(geometryPassNum, geometryPassFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, 0);
		defaultGeometryPassPso.SetInputLayout((UINT)pbrElement.size(), pbrElement.data());
		defaultGeometryPassPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		fbxGeometryPassPso = defaultPso;
		fbxGeometryPassPso.SetRootSignature(&geometryPassSignature);
		fbxGeometryPassPso.SetVertexShader(g_pFbxGeometryPassVS, sizeof(g_pFbxGeometryPassVS));
		fbxGeometryPassPso.SetPixelShader(g_pGeometryPassPS, sizeof(g_pGeometryPassPS));
		fbxGeometryPassPso.SetRenderTargetFormats(geometryPassNum, geometryPassFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, 0);
		fbxGeometryPassPso.SetInputLayout((UINT)pbrElement.size(), pbrElement.data());
		fbxGeometryPassPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		msaaGeometryPassPso = defaultGeometryPassPso;
		msaaGeometryPassPso.SetRenderTargetFormats(geometryPassNum, geometryPassFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, msaaCount, msaaQuality - 1);

		fbxMsaaGeometryPassPso = fbxGeometryPassPso;
		fbxMsaaGeometryPassPso.SetRenderTargetFormats(geometryPassNum, geometryPassFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, msaaCount, msaaQuality - 1);

		wireGeometryPassPso = defaultGeometryPassPso;
		wireGeometryPassPso.SetRasterizerState(wireRasterizer);

		fbxWireGeometryPassPso = fbxGeometryPassPso;
		fbxWireGeometryPassPso.SetRasterizerState(wireRasterizer);

		// Light Pass
		defaultLightPassPso = defaultPso;
		defaultLightPassPso.SetRootSignature(&lightPassSignature);
		defaultLightPassPso.SetVertexShader(g_pLightPassVS, sizeof(g_pLightPassVS));
		defaultLightPassPso.SetPixelShader(g_pLightPassPS, sizeof(g_pLightPassPS));
		defaultLightPassPso.SetInputLayout((UINT)simpleElement.size(), simpleElement.data());
		defaultLightPassPso.SetRenderTargetFormat(hdrFormat, DXGI_FORMAT_UNKNOWN, 1, 0);

		msaaLightPassPso = defaultLightPassPso;
		msaaLightPassPso.SetRenderTargetFormat(hdrFormat, DXGI_FORMAT_UNKNOWN, msaaCount, msaaQuality - 1);

		renderNormalPassPso = defaultPso;
		renderNormalPassPso.SetVertexShader(g_pDrawNormalPassVS, sizeof(g_pDrawNormalPassVS));
		renderNormalPassPso.SetPixelShader(g_pDrawNormalPassPS, sizeof(g_pDrawNormalPassPS));
		renderNormalPassPso.SetInputLayout((UINT)pbrElement.size(), pbrElement.data());
		renderNormalPassPso.SetGeometryShader(g_pDrawNormalPassGS, sizeof(g_pDrawNormalPassGS));
		renderNormalPassPso.SetRenderTargetFormat(hdrFormat, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, 0);
		renderNormalPassPso.SetRootSignature(&NormalPassSignature);
		renderNormalPassPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);

		renderBoundingBoxPassPso = defaultPso;
		renderBoundingBoxPassPso.SetVertexShader(g_pRenderBoundingBoxPassVS, sizeof(g_pRenderBoundingBoxPassVS));
		renderBoundingBoxPassPso.SetPixelShader(g_pRenderBoundingBoxPassPS, sizeof(g_pRenderBoundingBoxPassPS));
		renderBoundingBoxPassPso.SetGeometryShader(g_pRenderBoundingBoxPassGS, sizeof(g_pRenderBoundingBoxPassGS));
		renderBoundingBoxPassPso.SetRenderTargetFormat(hdrFormat, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, 0);
		renderBoundingBoxPassPso.SetRootSignature(&NormalPassSignature);
		renderBoundingBoxPassPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);

		postProcessingPso.SetRootSignature(&computeSignature);
		postProcessingPso.SetComputeShader(g_pPostprocessingCS, sizeof(g_pPostprocessingCS));

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
		//cubeMapPso.SetRenderTargetFormat(hdrFormat, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, 0);
		cubeMapPso.SetRenderTargetFormats(2, cubeMapPassFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, 0);

		wireCubeMapPso = cubeMapPso;
		wireCubeMapPso.SetRasterizerState(wireRasterizer);

		msaaCubeMapPso = cubeMapPso;
		//msaaCubeMapPso.SetRenderTargetFormat(hdrFormat, DXGI_FORMAT_D24_UNORM_S8_UINT, msaaCount, msaaQuality - 1);
		msaaCubeMapPso.SetRenderTargetFormats(2, cubeMapPassFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, msaaCount, msaaQuality - 1);

		copyPso = defaultPso;
		copyPso.SetRenderTargetFormat(backbufferFormat, DXGI_FORMAT_UNKNOWN, 1, 0);
		copyPso.SetRootSignature(&copySignature);
		copyPso.SetVertexShader(g_pCopyVS, sizeof(g_pCopyVS));
		copyPso.SetPixelShader(g_pCopyPS, sizeof(g_pCopyPS));

		copyDensityPso = copyPso;
		copyDensityPso.SetRenderTargetFormat(backbufferFormat, DXGI_FORMAT_UNKNOWN, 1, 0);
		//copyDensityPso.SetBlendState(addColorBlender);

		simulationRenderPso = defaultPso;
		simulationRenderPso.SetRootSignature(&simulationSignature);
		simulationRenderPso.SetVertexShader(g_pSimulationParticlesVS, sizeof(g_pSimulationParticlesVS));
		simulationRenderPso.SetGeometryShader(g_pSimulationParticlesGS, sizeof(g_pSimulationParticlesGS));
		simulationRenderPso.SetPixelShader(g_pSimulationParticlesPS, sizeof(g_pSimulationParticlesPS));
		simulationRenderPso.SetRenderTargetFormat(hdrFormat, DXGI_FORMAT_UNKNOWN, 1, 0);
		simulationRenderPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
		simulationRenderPso.SetBlendState(addColorBlender);

		simulationComputePso.SetComputeShader(g_pSimulationParticlesCS, sizeof(g_pSimulationParticlesCS));
		simulationComputePso.SetRootSignature(&simulationComputeSignature);

		sphSimulationComputePso.SetComputeShader(g_pSphSimulationParticlesCS, sizeof(g_pSphSimulationParticlesCS));
		sphSimulationComputePso.SetRootSignature(&simulationComputeSignature);

		sphComputeRhoPso.SetComputeShader(g_pSphComputeRhoCS, sizeof(g_pSphComputeRhoCS));
		sphComputeRhoPso.SetRootSignature(&simulationComputeSignature);
		
		sphComputeForcesPso.SetComputeShader(g_pSphComputeForcesCS, sizeof(g_pSphComputeForcesCS));
		sphComputeForcesPso.SetRootSignature(&simulationComputeSignature);

		simulationPostProcessingPso.SetRootSignature(&simulationPostProcessingSignature);
		simulationPostProcessingPso.SetComputeShader(g_pSimulationPostProcessingCS, sizeof(g_pSimulationPostProcessingCS));

		CFDSourcingPso.SetComputeShader(g_pCFDSourcingCS, sizeof(g_pCFDSourcingCS));
		CFDSourcingPso.SetRootSignature(&cfdSourcingSignature);

		CFDComputePressurePso.SetComputeShader(g_pCFDComputePressureCS, sizeof(g_pCFDComputePressureCS));
		CFDComputePressurePso.SetRootSignature(&cfdComputePressureSignature);

		CFDComputeDivergencePso.SetComputeShader(g_pCFDComputeDivergenceCS, sizeof(g_pCFDComputeDivergenceCS));
		CFDComputeDivergencePso.SetRootSignature(&cfdComputeDivergenceSignature);

		CFDAdvectionPso.SetComputeShader(g_pCFDAdvectionCS, sizeof(g_pCFDAdvectionCS));
		CFDAdvectionPso.SetRootSignature(&cfdAdvectionSignature);

		CFDApplyPressurePso.SetComputeShader(g_pCFDApplyPressureCS, sizeof(g_pCFDApplyPressureCS));
		CFDApplyPressurePso.SetRootSignature(&cfdAdvectionSignature);

		CFDComputeDiffusePso.SetComputeShader(g_pCFDComputeDiffuseCS, sizeof(g_pCFDComputeDiffuseCS));
		CFDComputeDiffusePso.SetRootSignature(&cfdComputeDiffuseSignature);

		CFDComputeVorticityPso.SetComputeShader(g_pCFDComputeVorticityCS, sizeof(g_pCFDComputeVorticityCS));
		CFDComputeVorticityPso.SetRootSignature(&cfdVorticitySignature);

		CFDVorticityConfinementPso.SetComputeShader(g_pCFDVorticityConfinementCS, sizeof(g_pCFDVorticityConfinementCS));
		CFDVorticityConfinementPso.SetRootSignature(&cfdVorticitySignature);

		perlinNoisePso.SetComputeShader(g_pPerlinNoiseCS, sizeof(g_pPerlinNoiseCS));
		perlinNoisePso.SetRootSignature(&simulationComputeSignature);

		modePsoLists[defaultPso.GetName()] = defaultPso;
		modePsoLists[wirePso.GetName()] = wirePso;
		modePsoLists[msaaPso.GetName()] = msaaPso;

		passPsoLists[defaultGeometryPassPso.GetName()] = defaultGeometryPassPso;
		passPsoLists[wireGeometryPassPso.GetName()] = wireGeometryPassPso;
		passPsoLists[msaaGeometryPassPso.GetName()] = msaaGeometryPassPso;
		passPsoLists[fbxGeometryPassPso.GetName()] = fbxGeometryPassPso;
		passPsoLists[fbxMsaaGeometryPassPso.GetName()] = fbxMsaaGeometryPassPso;
		passPsoLists[fbxWireGeometryPassPso.GetName()] = fbxWireGeometryPassPso;
		passPsoLists[defaultLightPassPso.GetName()] = defaultLightPassPso;
		passPsoLists[msaaLightPassPso.GetName()] = msaaLightPassPso;
		passPsoLists[renderNormalPassPso.GetName()] = renderNormalPassPso;
		passPsoLists[simulationRenderPso.GetName()] = simulationRenderPso;
		passPsoLists[renderBoundingBoxPassPso.GetName()] = renderBoundingBoxPassPso;

		utilityPsoLists[copyPso.GetName()] = copyPso;
		utilityPsoLists[copyDensityPso.GetName()] = copyDensityPso;

		cubePsoLists[cubeMapPso.GetName()] = cubeMapPso;
		cubePsoLists[msaaCubeMapPso.GetName()] = msaaCubeMapPso;
		cubePsoLists[wireCubeMapPso.GetName()] = wireCubeMapPso;

		computePsoList[postProcessingPso.GetName()] = postProcessingPso;
		computePsoList[simulationComputePso.GetName()] = simulationComputePso;
		computePsoList[simulationPostProcessingPso.GetName()] = simulationPostProcessingPso;
		
		computePsoList[sphSimulationComputePso.GetName()] = sphSimulationComputePso;
		computePsoList[sphComputeRhoPso.GetName()] = sphComputeRhoPso;
		computePsoList[sphComputeForcesPso.GetName()] = sphComputeForcesPso;

		computePsoList[CFDSourcingPso.GetName()] = CFDSourcingPso;
		computePsoList[CFDAdvectionPso.GetName()] = CFDAdvectionPso;
		computePsoList[CFDComputePressurePso.GetName()] = CFDComputePressurePso;
		computePsoList[CFDComputeDivergencePso.GetName()] = CFDComputeDivergencePso;
		computePsoList[CFDApplyPressurePso.GetName()] = CFDApplyPressurePso;
		computePsoList[CFDComputeDiffusePso.GetName()] = CFDComputeDiffusePso;
		computePsoList[CFDVorticityConfinementPso.GetName()] = CFDVorticityConfinementPso;
		computePsoList[CFDComputeVorticityPso.GetName()] = CFDComputeVorticityPso;
		
		computePsoList[perlinNoisePso.GetName()] = perlinNoisePso;

	}

	void Finalize(Microsoft::WRL::ComPtr<ID3D12Device5>& device)
	{
		defaultSignature.Finalize(device);
		computeSignature.Finalize(device);
		cubeMapSignature.Finalize(device);
		copySignature.Finalize(device);
		geometryPassSignature.Finalize(device);
		lightPassSignature.Finalize(device);
		NormalPassSignature.Finalize(device);
		
		simulationComputeSignature.Finalize(device);
		simulationSignature.Finalize(device);
		simulationPostProcessingSignature.Finalize(device);
		
		cfdSourcingSignature.Finalize(device);
		cfdAdvectionSignature.Finalize(device);
		cfdComputeDivergenceSignature.Finalize(device);
		cfdComputePressureSignature.Finalize(device);
		cfdApplyPressureSignature.Finalize(device);
		cfdComputeDiffuseSignature.Finalize(device);
		cfdVorticitySignature.Finalize(device);


		raytracingGlobalSignature.Finalize(device);

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

