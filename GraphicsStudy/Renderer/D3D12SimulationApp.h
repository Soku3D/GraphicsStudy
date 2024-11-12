#pragma once

#include "D3D12App.h"
#include "Renderer.h"
#include "Particle.h"
#include "Spring.h"
#include "StableFluids.h"
#include "Volume.h"


namespace Renderer {

	class D3D12SimulationApp :public D3D12App {
	public:
		D3D12SimulationApp(const int& width, const int& height);
		virtual ~D3D12SimulationApp();

		bool Initialize() override;
		bool InitGUI() override;
		bool InitDirectX() override;
		void InitSimulationScene();
		void OnResize() override;

		void Update(float& deltaTime) override;
		void UpdateGUI(float& deltaTime) override;
		void Render(float& deltaTime) override;

		void ParticleSimulation(float& deltaTime);
		void ClothSimulation(float& deltaTime);
		void RenderNoise(float& deltaTime);
		void GeneratePerlinNoise();
		void SPH(float& deltaTime);
		void CFD(float& deltaTime);
		void VolumeRendering(float& deltaTime);
		void SimulationPass(float& deltaTime, const std::string& particleName);
		void SmokeSimulationPass(float& deltaTime);

		void SPHSimulationPass(float& deltaTime, const std::string& psoName);
		void PostProcessing(float& deltaTime, const std::string& psoName, ID3D12Resource* hdrResource, ID3D12DescriptorHeap* resourceUavHeap, D3D12_RESOURCE_STATES resourceState, int heapIndex = 0);
		void SimulationRenderPass(float& deltaTime, const std::string& psoName, const std::string& particleName);
		void ClothRenderPass(float& deltaTime, const std::string& psoName, const std::string& particleName);
		void SPHSimulationRenderPass(float& deltaTime);
		void RenderFont(float& deltaTime);
		void CFDPass(float& deltaTime, const std::string& psoName);
		void CFDAdvectionPass(float& deltaTime);
		void CFDComputePressurePass(float& deltaTime);
		void CFDDiffusePass(float& deltaTime);
		void CFDVorticityPass(float& deltaTime, const std::string& psoName, int uavIndex, int srvIndex);
		void CFDApplyPressurePass(float& deltaTime);
		void CFDComputeDivergencePass(float& deltaTime);

		void ComputeVolumeDensityPass(float& deltaTime);

		void SmokeVorticityPass(float& deltaTime, const std::string& psoName, int index);
		void SmokeDownSamplePass(float& deltaTime);
		void SmokeSourcingDensityPass(float& deltaTime);
		void SmokeComputeDivergencePass(float& deltaTime);
		void SmokeComputePressurePass(float& deltaTime);
		void SmokeApplyPressurePass(float& deltaTime);
		void SmokeDiffUpSamplePass(float& deltaTime);
		void SmokeAdvectionPass(float& deltaTime);

		void ClothImplicitMethodPass(float& deltaTime, const std::string& psoName, const std::string& particleName);

		void RenderVolumMesh(float& deltaTime);
		void RenderBoundingBox(float& deltaTime);
		void RenderCubeMap(float& deltaTime);
		void RenderGUI(float& deltaTime) override;

		void FireParticles(const int& fireCount);

		void CopyResourceToSwapChain(float& deltaTime, ID3D12DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE handle);


		//void CopyDensityToSwapChain(float& deltaTime);

	protected:
		std::map<std::string, Particles> particles;
		//Particles sphParticle;
		//Particles clothParticle;
		Springs mSpring;
		StableFluids stableFluids;
		Core::ConstantBuffer<SimulationCSConstantData> mSimulationConstantBuffer;
		Core::ConstantBuffer<ClothSimulationConstantData> mClothSimulationConstantBuffer;
		Core::ConstantBuffer<CFDConstantData> mCFDConstantBuffer;
		Core::ConstantBuffer<VolumeConstantData> mVolumeConstantBuffer;
		Core::ConstantBuffer<CubeMapConstantData> mCubeMapConstantBuffer;

		Core::Texture3D mCloud;
		Volume* mSmoke;
		int upscale = 1;
		int smokeWidth = 128;
		int smokeHeight = 64;
		int smokeDepth = 64;

		bool bRenderCloud = false;
		bool bRenderSmoke = false;

		std::vector<XMFLOAT3> colorLists;

		const wchar_t* simulationRenderPassEvent = L"Simulation Render Pass ";
		const wchar_t* sphSimulationRenderPassEvent = L"SPH Simulation Render Pass ";
		const wchar_t* simulationPassEvent = L"Simulation Pass ";
		const wchar_t* sphSimulationPassEvent = L"SPH Simulation Pass ";
		const wchar_t* postprocessingEvent = L"Postprocessing Pass ";
		const wchar_t* copyDensityToSwapChainEvent = L" copyDensityToSwapChain ";

		const wchar_t* cfdSourcingEvent = L"CFD Simulation Pass ";
		const wchar_t* cfdComputeDivergenceEvent = L"CFD ComputeDivergence Pass ";
		const wchar_t* cfdAdvectionEvent = L"CFD Advection Pass ";
		const wchar_t* cfdComputePressureEvent = L"CFD ComputePressure Pass ";
		const wchar_t* cfdApplyPressureEvent = L"CFD ApplyPressure Pass ";
		const wchar_t* cfdDiffuseEvent = L"CFD Diffuse Pass ";
		const wchar_t* cfdVorticityEvent = L"CFD Vorticity Pass ";

		const wchar_t* volumeRenderEvent = L"Volume Render Pass ";


	private:
		float mGuiVorticity = 0.8f;
		float mGuiViscosity = 0.f;
		float mGuiSourceStrength = 1.33f;
	};
}