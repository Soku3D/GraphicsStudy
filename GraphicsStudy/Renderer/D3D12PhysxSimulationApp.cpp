#include "D3D12PhysxSimulationApp.h"

#include "D3D12PhysxSimulationApp.h"


Renderer::D3D12PhysxSimulationApp::D3D12PhysxSimulationApp(const int& width, const int& height)
	:D3D12App(width, height)
{
	bUseTextureApp = false;
	bUseGUI = false;
}

bool Renderer::D3D12PhysxSimulationApp::Initialize()
{
	if (!D3D12App::Initialize())
		return false;

	return true;
}

bool Renderer::D3D12PhysxSimulationApp::InitGUI()
{
	if (!D3D12App::InitGUI())
		return false;
	return true;
}

bool Renderer::D3D12PhysxSimulationApp::InitDirectX()
{
	if (!D3D12App::InitDirectX())
		return false;

	return true;
}

void Renderer::D3D12PhysxSimulationApp::OnResize()
{
	D3D12App::OnResize();
}

void Renderer::D3D12PhysxSimulationApp::Update(float& deltaTime)
{
	D3D12App::Update(deltaTime);
}

void Renderer::D3D12PhysxSimulationApp::UpdateGUI(float& deltaTime)
{
	D3D12App::UpdateGUI(deltaTime);
}

void Renderer::D3D12PhysxSimulationApp::Render(float& deltaTime)
{
}

void Renderer::D3D12PhysxSimulationApp::RenderGUI(float& deltaTime)
{
	D3D12App::RenderGUI(deltaTime);
}
