#pragma once

#include "D3D12App.h"
#include "Renderer.h"
#include "RaytracingHlslCompat.h"
#include "RaytracingHelper.h"

namespace Renderer {

	struct __declspec(align(256)) RaytraingSceneConstantData {
		DirectX::SimpleMath::Matrix view = DirectX::SimpleMath::Matrix();
		DirectX::SimpleMath::Vector3 cameraPosition = DirectX::SimpleMath::Vector3(0,0,0);
		float dummy = 0.5;
	};

	class D3D12RayTracingApp :public D3D12App {
	public:
		D3D12RayTracingApp(const int& width, const int& height);
		virtual ~D3D12RayTracingApp();

		bool Initialize() override;
		bool InitGUI() override;
		void CreateStateObjects();
		void CreateShaderTable();
		void InitializeViews();
		void CreateConstantBuffer() override;
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

		ComPtr<ID3D12Resource> m_rgsTable;
		ComPtr<ID3D12Resource> m_missTable;
		ComPtr<ID3D12Resource> m_hitGroupsTable;
		uint8_t* pRgsData;
		uint8_t* pMissData;
		uint8_t* pHitgroupsData;
		uint8_t* pHitgroupsData1;
		ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;

		class ShaderTable* hitShaderTable;
		Core::StaticMesh* mCharacterMesh;
		//std::vector<D3D12_STATE_SUBOBJECT> subObjects;
		//D3D12_STATE_OBJECT_DESC rtStateObject;
		//D3D12_STATE_SUBOBJECT config;
	protected:
		std::vector< D3D12_RAYTRACING_INSTANCE_DESC> m_instances;
		ComPtr<ID3D12Resource> m_tlas;
		ComPtr<ID3D12Resource> mScratchResource;
		ComPtr<ID3D12Resource> m_instanceDescs;
		D3D12_GPU_VIRTUAL_ADDRESS GetTlas() { return m_tlas->GetGPUVirtualAddress(); }
	protected:
		ComPtr<ID3D12RootSignature> m_raytracingLocalSignature;
		const wchar_t* rayGenerationShaderName = L"RayGen";
		const wchar_t* closestHitShaderName = L"Hit";
		std::vector<const wchar_t*> hitGroupNames;
		std::vector<const wchar_t*> ShadowhitGroupNames;
		std::vector<const wchar_t*> characterHitGroupNames;
		std::vector<const wchar_t*> characterShadowHitGroupNames;

		const wchar_t* missShaderNames[2] = { L"Miss", L"Miss_ShadwRay" };

		/*RayGenConstantBufferData m_rayGenCB;*/
		
		SceneConstantBuffer m_sceneCBData;
		uint8_t* pSceneBegin;
		ComPtr<ID3D12Resource> m_sceneCB;

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc;
		uint8_t* pInstancesMappedData;

	protected:
		std::vector<ComPtr<ID3D12DescriptorHeap>> m_raytracingHeaps;
		std::vector<ComPtr<ID3D12DescriptorHeap>> mChracterRaytracingHeaps;
		ComPtr<ID3D12DescriptorHeap> m_raytracingGlobalHeap;
		const wchar_t* cubeMapTextureName = L"DefaultEnvHDR.dds";

	protected:
		Core::StaticMesh* characterMesh;
		UINT characterInstanceId = 0;
	};
}