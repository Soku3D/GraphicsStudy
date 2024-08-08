#pragma once

#include "PipelineState.h"

#include <map>
#include <string>
#include <vector>

namespace Renderer {
	
	extern std::map<std::string, GraphicsPSO > psoList;
	extern std::vector<std::string> psoListNames;
	extern UINT msaaCount;
	extern UINT msaaQuality;

	extern GraphicsPSO defaultPso; 


	extern RootSignature defaultSignature;
	
	extern std::vector<D3D12_INPUT_ELEMENT_DESC> defaultElement;

	extern D3D12_STATIC_SAMPLER_DESC defaultSampler;

	extern D3D12_RASTERIZER_DESC defaultRasterizer;
	extern D3D12_RASTERIZER_DESC wireRasterizer;

	extern D3D12_BLEND_DESC defaultBlender;
	extern D3D12_BLEND_DESC alphaBlender;

	void Initialize(void);
	void Finalize(Microsoft::WRL::ComPtr<ID3D12Device>& device);
}


