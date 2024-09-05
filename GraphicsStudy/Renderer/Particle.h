#pragma once

#include <vector>
#include "directxtk/SimpleMath.h"
#include "wrl.h"
#include "d3d12.h"

struct Particle {
	DirectX::SimpleMath::Vector3 m_position;
	DirectX::SimpleMath::Vector3 m_color;
	float radius;
};

class Particles {
public:
	Particles() {};
	~Particles() {};

	void Initialize(int numPatricles);
	void BuildResources(Microsoft::WRL::ComPtr <ID3D12Device5> & device,
		Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList>& commandList);
	void BuildDescriptors(Microsoft::WRL::ComPtr <ID3D12Device5>& device,
		Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList>& commandList);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandle() const {
		return m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetUavHandle() const {
		return m_uavHeap->GetGPUDescriptorHandleForHeapStart();
	}
	ID3D12DescriptorHeap* GetSrvHeap() const {
		return m_srvHeap.Get();
	}
	ID3D12DescriptorHeap* GetUavHeap() const {
		return m_uavHeap.Get();
	}
	UINT GetParticleCount() const {
		return (UINT)m_cpu.size();
	}
private:
	std::vector<Particle> m_cpu;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_gpu;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_upload;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_uavHeap;
};