#include "D3D12FontApp.h"

Renderer::D3D12FontApp::D3D12FontApp(const int& width, const int& height)
	:D3D12App(width, height)
{
}

bool Renderer::D3D12FontApp::Initialize()
{
	using namespace DirectX;
	if (!D3D12App::Initialize())
		return false;

	
	return true;
}

bool Renderer::D3D12FontApp::InitDirectX()
{
	if (!D3D12App::InitDirectX())
		return false;

	return true;
}

void Renderer::D3D12FontApp::OnResize()
{
	D3D12App::OnResize();
}

void Renderer::D3D12FontApp::Update(float& deltaTime)
{
	D3D12App::Update(deltaTime);
}

void Renderer::D3D12FontApp::Render(float& deltaTime)
{
	

	D3D12App::Render(deltaTime);

}
