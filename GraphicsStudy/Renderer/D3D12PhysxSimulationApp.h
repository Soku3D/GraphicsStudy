#pragma once
#include "D3D12App.h"
#include "Renderer.h"

#include "physx/PxPhysicsAPI.h"

namespace Renderer {

	class D3D12PhysxSimulationApp :public D3D12App {
	public:
		D3D12PhysxSimulationApp(const int& width, const int& height);
		virtual ~D3D12PhysxSimulationApp() {}

		bool Initialize() override;
		bool InitGUI() override;
		bool InitDirectX() override;
		void OnResize() override;

		void Update(float& deltaTime) override;
		void UpdateGUI(float& deltaTime) override;
		void Render(float& deltaTime) override;
		void SimulationPass(float& deltaTime);
		void SimulationRenderPass(float& deltaTime);
		void RenderGUI(float& deltaTime) override;

	protected:
		Particles particle;
		const wchar_t* simulationRenderPassEvent = L"Simulation Render Pass ";
		const wchar_t* simulationPassEvent = L"Simulation Pass ";
	};
}