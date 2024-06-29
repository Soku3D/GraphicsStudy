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
	// 1. 명령 목록, 명령 큐, 명령 할당자 생성

	// 2. 서술자 뷰 크기 저장

	// 3. dxgifactory와 스왑체인 생성

	// 4. 각 스왑체인 버퍼에 대한 RTV생성

	// 5. ScisorRect 생성

	// 6. vertex, index Buffer 생성

	// 7. vertex pixel Shader 생성

	// 8.
	return true;
}

bool Renderer::D3D12App::InitDirectX()
{
	return false;
}

void Renderer::D3D12App::OnResize()
{
}

void Renderer::D3D12App::Update(const double& deltaTime)
{
}

void Renderer::D3D12App::Render(const double& deltaTime)
{
}

