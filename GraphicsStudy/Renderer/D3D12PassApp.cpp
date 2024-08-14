#include "D3D12PassApp.h"
#include "Renderer.h"
#include <DirectXTexEXR.h>


Renderer::D3D12PassApp::D3D12PassApp(const int& width, const int& height)
	:D3D12App(width, height)
{
	bUseGUI = false;
}

bool Renderer::D3D12PassApp::Initialize()
{
	if (!D3D12App::Initialize())
		return false;


	return true;
}

bool Renderer::D3D12PassApp::InitGUI()
{
	if (!D3D12App::InitGUI())
		return false;
	return true;
}

bool Renderer::D3D12PassApp::InitDirectX()
{
	if (!D3D12App::InitDirectX())
		return false;

	return true;
}

void Renderer::D3D12PassApp::OnResize()
{
	D3D12App::OnResize();
}

void Renderer::D3D12PassApp::Update(float& deltaTime)
{
	D3D12App::Update(deltaTime);
}

void Renderer::D3D12PassApp::UpdateGUI(float& deltaTime)
{
	D3D12App::UpdateGUI(deltaTime);
}

void Renderer::D3D12PassApp::Render(float& deltaTime)
{
	GeometryPass(deltaTime);
	CopyResource(m_commandList, HDRRenderTargetBuffer(), m_geometryPassResources[0].Get());

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* plists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

	FlushCommandQueue();
}

void Renderer::D3D12PassApp::PostProcessing(float& deltaTime) {

}

void Renderer::D3D12PassApp::RenderGUI(float& deltaTime)
{
	D3D12App::RenderGUI(deltaTime);
}
