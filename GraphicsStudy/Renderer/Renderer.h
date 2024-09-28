#pragma once

#include "PipelineState.h"

#include <map>
#include <string>
#include <vector>

namespace Renderer {
	
	extern std::map<std::string, GraphicsPSO > modePsoLists;
	extern std::map<std::string, GraphicsPSO > passPsoLists;
	extern std::map<std::string, GraphicsPSO > cubePsoLists;
	extern std::map<std::string, GraphicsPSO > utilityPsoLists;

	extern std::vector<std::string> modePsoListNames;
	extern std::vector<std::string> passPsoListNames;
	extern std::vector<std::string> cubePsoListNames;
	extern std::vector<std::string> utilityPsoListNames;
	
	static const int geometryPassNum = 4;
	extern DXGI_FORMAT geometryPassFormats[geometryPassNum];
	extern DXGI_FORMAT cubeMapPassFormats[2];
	extern DXGI_FORMAT backbufferFormat;
	extern DXGI_FORMAT hdrFormat;

	extern std::map<std::string, ComputePSO > computePsoList;
	extern std::vector<std::string> computePsoListNames;

	extern UINT msaaCount;
	extern UINT msaaQuality;

	extern GraphicsPSO defaultPso;
	extern ComputePSO postProcessingPso;

	extern RootSignature defaultSignature;

	extern RootSignature cubeMapSignature;
	extern RootSignature geometryPassSignature;
	extern RootSignature lightPassSignature;
	extern RootSignature NormalPassSignature;

	extern RootSignature copySignature;
	
	extern RootSignature computeSignature;
	extern RootSignature simulationComputeSignature;
	extern RootSignature simulationSignature;
	extern RootSignature simulationPostProcessingSignature;

	extern RootSignature cfdSourcingSignature;
	extern RootSignature cfdComputeDivergenceSignature;
	extern RootSignature cfdComputePressureSignature;
	extern RootSignature cfdAdvectionSignature;

	extern RootSignature computeVolumeDensitySignature;
	extern RootSignature renderVolumeSignature;

	extern RootSignature raytracingGlobalSignature;

	extern std::vector<D3D12_INPUT_ELEMENT_DESC> defaultElement;
	extern std::vector<D3D12_INPUT_ELEMENT_DESC> simpleElement;
	extern std::vector<D3D12_INPUT_ELEMENT_DESC> pbrElement;

	extern D3D12_STATIC_SAMPLER_DESC wrapLinearSampler;
	extern D3D12_STATIC_SAMPLER_DESC wrapPointSampler;
	extern D3D12_STATIC_SAMPLER_DESC clampLinearSampler;
	extern D3D12_STATIC_SAMPLER_DESC clampPointSampler;

	extern D3D12_STATIC_SAMPLER_DESC testSampler;

	extern std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;
	extern std::vector<D3D12_STATIC_SAMPLER_DESC> wrapSamplers;

	extern D3D12_RASTERIZER_DESC defaultRasterizer;
	extern D3D12_RASTERIZER_DESC wireRasterizer;

	extern D3D12_BLEND_DESC defaultBlender;
	extern D3D12_BLEND_DESC alphaBlender;
	extern D3D12_BLEND_DESC simulationBlender;

	void Initialize(void);
	void Finalize(Microsoft::WRL::ComPtr<ID3D12Device5>& device);
}


