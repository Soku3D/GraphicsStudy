#pragma once

#include "wrl.h"
#include "d3d12.h"

namespace Core {

	template<typename ConstantStructure>
	class ConstantBuffer 
	{
	public:
		ConstantBuffer();
		
		ID3D12Resource** GetAddressOf() { return mSimuationConstantBuffer.ReleaseAndGetAddressOf(); }
		ID3D12Resource* Get() { return mSimuationConstantBuffer.Get(); }
		void** GetData() { return &pSimulationConstant; }
		void UpdateBuffer() 
		{
			memcpy(pSimulationConstant, &mStructure, sizeof(ConstantStructure));
		}
		D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress() { return mSimuationConstantBuffer->GetGPUVirtualAddress(); }
		ConstantStructure mStructure;
	private:
		UINT bufferSize;
		
		void* pSimulationConstant;
		Microsoft::WRL::ComPtr<ID3D12Resource> mSimuationConstantBuffer;
	};

	template<typename ConstantStructure>
	inline ConstantBuffer<ConstantStructure>::ConstantBuffer()
	{

	}

}