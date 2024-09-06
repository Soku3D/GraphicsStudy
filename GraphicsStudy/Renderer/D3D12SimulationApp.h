#pragma once

#include "D3D12App.h"
#include "Renderer.h"
#include "Particle.h"
#include "Buffer.h"

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
		void SPH(float& deltaTime);
		void SimulationPass(float& deltaTime);
		void SPHSimulationPass(float& deltaTime);
		void PostProcessing(float& deltaTime);
		void SimulationRenderPass(float& deltaTime);
		void RenderGUI(float& deltaTime) override;

	protected:
		Particles particle;
		Core::ConstantBuffer<SimulationCSConstantData> mSimulationConstantBuffer;


		const wchar_t* simulationRenderPassEvent = L"Simulation Render Pass ";
		const wchar_t* simulationPassEvent = L"Simulation Pass ";
		const wchar_t* postprocessingEvent = L"Postprocessing Pass ";

	};
}