#include "D3D12App.h"

Renderer::D3D12App::D3D12App(const int& width, const int& height)
	:SimpleApp(width, height)
{
}

Renderer::D3D12App::~D3D12App()
{
}

bool Renderer::D3D12App::Initialize()
{
	if (!SimpleApp::Initialize()) {
		return false;
	}
	return true;
}

bool Renderer::D3D12App::InitDirectX()
{
	return false;
}

void Renderer::D3D12App::OnResize()
{
}

void Renderer::D3D12App::Render(const double& deltaTime)
{
}

void Renderer::D3D12App::Update(const double& deltaTime)
{
}
