#include "D3D12ComputeShaderApp.h"
#include "Renderer.h"
#include <DirectXTexEXR.h>


Renderer::D3D12ComputeShaderApp::D3D12ComputeShaderApp(const int& width, const int& height)
	:D3D12App(width, height)
{
	bUseGUI = true;
}

bool Renderer::D3D12ComputeShaderApp::Initialize()
{
	if (!D3D12App::Initialize())
		return false;

	psConstantData = new CSConstantData();
	psConstantData->time = 0;

	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(CSConstantData)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_csBuffer));

	D3D12_RANGE range(0, 0);
	ThrowIfFailed(m_csBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCbufferBegin)));
	memcpy(m_pCbufferBegin, psConstantData, sizeof(CSConstantData));

	

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
	if (!D3D12App::InitDirectX())
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

	psConstantData->time = (float)m_timer.GetElapsedTime();

	memcpy(m_pCbufferBegin, psConstantData, sizeof(CSConstantData));
}

void Renderer::D3D12ComputeShaderApp::UpdateGUI(float& deltaTime)
{
	D3D12App::UpdateGUI(deltaTime);
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

		m_commandList->SetComputeRootDescriptorTable(0, m_hdrUavHeap->GetGPUDescriptorHandleForHeapStart());

		m_commandList->SetComputeRootConstantBufferView(1, m_csBuffer->GetGPUVirtualAddress());
		m_commandList->Dispatch((UINT)ceil(m_screenWidth / 32.f), (UINT)ceil(m_screenHeight / 32.f), 1);

		m_commandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				HDRRenderTargetBuffer(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			));


		//ResolveSubresource(m_commandList);
		//CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());
		
		ThrowIfFailed(m_commandList->Close());

		ID3D12CommandList* lists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

		/*ThrowIfFailed(m_swapChain->Present(0, 0));
		m_frameIndex = (m_frameIndex + 1) % m_swapChainCount;*/

		FlushCommandQueue();

		CopyResourceToSwapChain(deltaTime);
	}
}

void Renderer::D3D12ComputeShaderApp::RenderGUI(float& deltaTime)
{
	D3D12App::RenderGUI(deltaTime);
}
