#pragma once

#include "D3D12App.h"
#include "Renderer.h"
#include "Particle.h"
#include "StableFluids.h"

namespace Renderer {

	class D3D12SimulationApp :public D3D12App {
	public:
		D3D12SimulationApp(const int& width, const int& height);
		virtual ~D3D12SimulationApp() {}

		bool Initialize() override;
		bool InitGUI() override;
		bool InitDirectX() override;
		void OnResize() override;

		void Update(float& deltaTime) override;
		void UpdateGUI(float& deltaTime) override;
		void Render(float& deltaTime) override;
		void ParticleSimulation(float& deltaTime);
		void RenderNoise(float& deltaTime);
		void GeneratePerlinNoise();
		void SPH(float& deltaTime);
		void CFD(float& deltaTime);
		void SimulationPass(float& deltaTime);
		void SPHSimulationPass(float& deltaTime, const std::string& psoName);
		void PostProcessing(float& deltaTime, const std::string& psoName, ID3D12Resource* hdrResource, ID3D12DescriptorHeap* resourceUavHeap, D3D12_RESOURCE_STATES resourceState, int heapIndex = 0);
		void SimulationRenderPass(float& deltaTime);
		void SPHSimulationRenderPass(float& deltaTime);
		void RenderFont(float& deltaTime);
		void CFDPass(float& deltaTime, const std::string& psoName);
		void CFDAdvectionPass(float& deltaTime);
		void CFDComputePressurePass(float& deltaTime);
		void CFDDiffusePass(float& deltaTime);
		void CFDVorticityPass(float& deltaTime, const std::string& psoName, int uavIndex, int srvIndex);
		void CFDApplyPressurePass(float& deltaTime);
		void CFDComputeDivergencePass(float& deltaTime);
		void RenderGUI(float& deltaTime) override;

		void FireParticles(const int& fireCount);

		void CopyResourceToSwapChain(float& deltaTime, ID3D12DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE handle);


		//void CopyDensityToSwapChain(float& deltaTime);

	protected:
		Particles particle;
		Particles sphParticle;
		StableFluids stableFluids;
		Core::ConstantBuffer<SimulationCSConstantData> mSimulationConstantBuffer;
		Core::ConstantBuffer<CFDConstantData> mCFDConstantBuffer;

		std::vector<XMFLOAT3> colorLists;

		const wchar_t* simulationRenderPassEvent = L"Simulation Render Pass ";
		const wchar_t* sphSimulationRenderPassEvent = L"SPH Simulation Render Pass ";
		const wchar_t* simulationPassEvent = L"Simulation Pass ";
		const wchar_t* sphSimulationPassEvent = L"SPH Simulation Pass ";
		const wchar_t* postprocessingEvent = L"Postprocessing Pass ";
		const wchar_t* copyDensityToSwapChainEvent = L" copyDensityToSwapChain ";

		const wchar_t* cfdSourcingEvent = L"CFD Simulation Pass ";
		const wchar_t* cfdAdvectionEvent = L"CFD Advection Pass ";
		const wchar_t* cfdComputePressureEvent = L"CFD ComputePressure Pass ";
		const wchar_t* cfdApplyPressureEvent = L"CFD ApplyPressure Pass ";
		const wchar_t* cfdDiffuseEvent = L"CFD Diffuse Pass ";
		const wchar_t* cfdVorticityEvent = L"CFD Vorticity Pass ";

	private:
		float mGuiVorticity = 0.2f;
		float mGuiViscosity = 0.f;
	};
}