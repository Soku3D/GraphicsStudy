#include "D3D12ComputeShaderApp.h"
#include "Renderer.h"

Renderer::D3D12ComputeShaderApp::D3D12ComputeShaderApp(const int& width, const int& height)
	:D3D12App(width, height)
{
	bUseGUI = false;
}

bool Renderer::D3D12ComputeShaderApp::Initialize()
{
	if (!D3D12App::Initialize())
		return false;
	return true;
}

bool Renderer::D3D12ComputeShaderApp::InitGUI()
{
	if (!D3D12App::InitGUI())
		return false;
	return true;
}

bool Renderer::D3D12ComputeShaderApp::InitDirectX()
{
	if(!D3D12App::InitDirectX())
		return false;

	return true;
}

void Renderer::D3D12ComputeShaderApp::OnResize()
{
	D3D12App::OnResize();
}

void Renderer::D3D12ComputeShaderApp::Update(float& deltaTime)
{
	D3D12App::Update(deltaTime);
}

void Renderer::D3D12ComputeShaderApp::UpdateGUI(float& deltaTime)
{
	//D3D12App::UpdateGUI(deltaTime);
}

void Renderer::D3D12ComputeShaderApp::Render(float& deltaTime)
{
	D3D12App::Render(deltaTime);

	auto& pso = computePsoList["Compute"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

	{
		m_commandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				HDRRenderTargetBuffer(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS
			));
		
		m_commandList->SetComputeRootSignature(pso.GetRootSignature());
		
		// UAV Heap 
		ID3D12DescriptorHeap* ppHeaps[] = { m_hdrUavHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		
		// UAV Texture
		m_commandList->SetComputeRootDescriptorTable(0, m_hdrUavHeap->GetGPUDescriptorHandleForHeapStart());

		m_commandList->Dispatch((UINT)(m_screenWidth / 256.f), m_screenHeight, 1);

		m_commandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				HDRRenderTargetBuffer(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			));

	}
	//ResolveSubresource(m_commandList);
	CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

	/*ThrowIfFailed(m_swapChain->Present(0, 0));
	m_frameIndex = (m_frameIndex + 1) % m_swapChainCount;*/

	FlushCommandQueue();
}

void Renderer::D3D12ComputeShaderApp::RenderGUI(float& deltaTime)
{
	D3D12App::RenderGUI(deltaTime);
}
