#pragma once

#include "PipelineState.h"

#include <map>
#include <string>
#include <vector>

namespace Renderer {
	
	extern std::map<std::string, GraphicsPSO > grphicsPsoList;
	extern std::map<std::string, GraphicsPSO > cubePsoList;
	extern std::map<std::string, GraphicsPSO > utilityPsoList;

	extern std::vector<std::string> graphicsPsoListNames;
	extern std::vector<std::string> cubePsoListNames;
	extern std::vector<std::string> utilityPsoListNames;
	
	static const int geometryPassNum = 4;
	extern DXGI_FORMAT geometryPassFormats[geometryPassNum];
	extern DXGI_FORMAT backbufferFormat;
	extern DXGI_FORMAT hdrFormat;

	extern std::map<std::string, ComputePSO > computePsoList;
	extern std::vector<std::string> computePsoListNames;

	extern UINT msaaCount;
	extern UINT msaaQuality;

	extern GraphicsPSO defaultPso;
	extern ComputePSO computePso;

	extern RootSignature defaultSignature;
	extern RootSignature computeSignature;
	extern RootSignature cubeMapSignature;
	extern RootSignature geometryPassSignature;

	extern RootSignature copySignature;
	
	extern std::vector<D3D12_INPUT_ELEMENT_DESC> defaultElement;
	extern std::vector<D3D12_INPUT_ELEMENT_DESC> simpleElement;

	extern D3D12_STATIC_SAMPLER_DESC defaultSampler;

	extern D3D12_RASTERIZER_DESC defaultRasterizer;
	extern D3D12_RASTERIZER_DESC wireRasterizer;

	extern D3D12_BLEND_DESC defaultBlender;
	extern D3D12_BLEND_DESC alphaBlender;

	void Initialize(void);
	void Finalize(Microsoft::WRL::ComPtr<ID3D12Device>& device);
}


