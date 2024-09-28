#include "D3D12DLSSApp.h"


Renderer::D3D12DLSSApp::D3D12DLSSApp(const int& width, const int& height)
	:D3D12App(width, height)
{
	bUseGUI = true;
}

bool Renderer::D3D12DLSSApp::Initialize()
{
	if (!D3D12App::Initialize())
		return false;

	return true;
}

bool Renderer::D3D12DLSSApp::InitGUI()
{
	if (!D3D12App::InitGUI())
		return false;

	return true;
}

bool Renderer::D3D12DLSSApp::InitDirectX()
{
	if (!D3D12App::InitDirectX())
		return false;

	return true;
}

void Renderer::D3D12DLSSApp::OnResize()
{
	D3D12App::OnResize();
}
void Renderer::D3D12DLSSApp::InitializeDLSS()
{
	NVSDK_NGX_Result result;
	result = NVSDK_NGX_D3D12_Init(NVSDK_NGX_Version_API, L"app_id", m_device.Get());

	if (NVSDK_NGX_FAILED(result))
	{
		printf("DLSS 초기화 실패: %d\n", result);
	}
}
bool Renderer::D3D12DLSSApp::CreateDLSSFeature(ID3D12Resource* inputResource, ID3D12Resource* outputResource, NVSDK_NGX_Handle** pHandle)
{
	
}

// 매 프레임마다 DLSS를 적용하는 함수
void Renderer::D3D12DLSSApp::ApplyDLSS(NVSDK_NGX_Handle* handle, ID3D12GraphicsCommandList* commandList, ID3D12Resource* input, ID3D12Resource* output)
{
	
}

void Renderer::D3D12DLSSApp::CleanupDLSS()
{
   
}

void Renderer::D3D12DLSSApp::Update(float& deltaTime)
{
	//D3D12App::Update(deltaTime);
}

void Renderer::D3D12DLSSApp::UpdateGUI(float& deltaTime)
{
	D3D12App::UpdateGUI(deltaTime);
}

void Renderer::D3D12DLSSApp::Render(float& deltaTime)
{
	auto& pso = computePsoList["Compute"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

	{
		

		ThrowIfFailed(m_commandList->Close());

		ID3D12CommandList* lists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

		/*ThrowIfFailed(m_swapChain->Present(0, 0));
		m_frameIndex = (m_frameIndex + 1) % m_swapChainCount;*/

		FlushCommandQueue();

		CopyResourceToSwapChain(deltaTime);
	}
}

void Renderer::D3D12DLSSApp::RenderGUI(float& deltaTime)
{
	D3D12App::RenderGUI(deltaTime);
}
