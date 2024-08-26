#pragma once

#include "D3D12App.h"
#include "Renderer.h"

namespace Renderer {
	struct Viewport
	{
		float left;
		float top;
		float right;
		float bottom;
	};

	struct RayGenConstantBuffer
	{
		Viewport viewport;
		Viewport stencil;
	};
	struct __declspec(align(256)) ShaderTable {
		uint8_t m_mappedShaderRecords[D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES];
	};
	class D3D12RayTracingApp :public D3D12App {
	public:
		D3D12RayTracingApp(const int& width, const int& height);
		virtual ~D3D12RayTracingApp() {};

		bool Initialize() override;
		bool InitGUI() override;
		bool InitDirectX() override;
		void OnResize() override;
		void InitRayTracingScene();

		void BuildAccelerationStructures();

		void Update(float& deltaTime) override;
		void UpdateGUI(float& deltaTime) override;
		void Render(float& deltaTime) override;
		void RaytracingPass(float& deltaTime);
		void RenderCubeMap(float& deltaTime) override;
		void RenderGUI(float& deltaTime) override;

	protected:
		const wchar_t* raytracingPass = L"Raytracing Pass ";
		ComPtr<ID3D12GraphicsCommandList4> m_dxrCommandList;

		ComPtr<ID3D12StateObject> m_rtpso;
		ComPtr<ID3D12StateObjectProperties> m_rtpsoInfo;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_globalRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_localRootSignature;

		RayGenConstantBuffer m_rayGenCB;
		ComPtr<ID3D12Resource> m_rgsTable;
		ComPtr<ID3D12Resource> m_missTable;
		ComPtr<ID3D12Resource> m_hitGroupsTable;
		uint8_t* pRgsData;
		uint8_t* pMissData;
		uint8_t* pHitgroupsData;
		ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;

	};
}