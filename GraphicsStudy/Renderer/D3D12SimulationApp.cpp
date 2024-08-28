#include "D3D12SimulationApp.h"

#define SIMULATION_PARTICLE_SIZE 768

Renderer::D3D12SimulationApp::D3D12SimulationApp(const int& width, const int& height)
	:D3D12App(width, height)
{
	bUseTextureApp = false;
	bUseCubeMapApp = false;
	bUseGUI = false;

	m_appName =	"SimulationApp";
}

bool Renderer::D3D12SimulationApp::Initialize()
{
	if (!D3D12App::Initialize())
		return false;

	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	particle.Initialize(SIMULATION_PARTICLE_SIZE );
	particle.BuildResources(m_device, m_commandList);

	m_commandList->Close();
	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	particle.BuildDescriptors(m_device, m_commandList);

	return true;
}

bool Renderer::D3D12SimulationApp::InitGUI()
{
	if (!D3D12App::InitGUI())
		return false;
	return true;
}

bool Renderer::D3D12SimulationApp::InitDirectX()
{
	if (!D3D12App::InitDirectX())
		return false;

	return true;
}

void Renderer::D3D12SimulationApp::OnResize()
{
	D3D12App::OnResize();
}

void Renderer::D3D12SimulationApp::Update(float& deltaTime)
{
	D3D12App::Update(deltaTime);


}

void Renderer::D3D12SimulationApp::UpdateGUI(float& deltaTime)
{
	D3D12App::UpdateGUI(deltaTime);
}

void Renderer::D3D12SimulationApp::Render(float& deltaTime)
{
	SimulationPass(deltaTime);
	SimulationRenderPass(deltaTime);
	PostProcessing(deltaTime);
	CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());
}

void Renderer::D3D12SimulationApp::SimulationPass(float& deltaTime)
{
	auto& pso = computePsoList["SimulationCompute"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), simulationPassEvent);
	
	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		particle.GetUavHeap()
	};
	//m_commandList->ClearUnorderedAccessViewUint()
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, particle.GetUavHandle());
	m_commandList->Dispatch(particle.GetParticleCount() / SIMULATION_PARTICLE_SIZE, 1, 1);
	//m_commandList->ExecuteIndirect(,)
	ThrowIfFailed(m_commandList->Close());
	
	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::PostProcessing(float& deltaTime) {

	//D3D12App::Render(deltaTime);

	auto& pso = computePsoList["Compute"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

	{
		PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(0, 0, 255), postprocessingEvent);

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

		//m_commandList->SetComputeRootConstantBufferView(1, m_csBuffer->GetGPUVirtualAddress());
		m_commandList->Dispatch((UINT)ceil(m_screenWidth / 32.f), (UINT)ceil(m_screenHeight / 32.f), 1);

		m_commandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				HDRRenderTargetBuffer(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			));

		ThrowIfFailed(m_commandList->Close());

		ID3D12CommandList* lists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

		FlushCommandQueue();
		PIXEndEvent(m_commandQueue.Get());
	}
}

void Renderer::D3D12SimulationApp::SimulationRenderPass(float& deltaTime)
{
	auto& pso = passPsoLists["SimulationRenderPass"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(0, 0, 255), simulationRenderPassEvent);

	FLOAT clear[4] = { 0,0,0,0 };
	m_commandList->ClearRenderTargetView(CurrentBackBufferView(), clear, 0, nullptr);
	m_commandList->ClearDepthStencilView(HDRDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.f,0,0,nullptr);

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());

	ID3D12DescriptorHeap* pHeaps[] = {
		particle.GetSrvHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, particle.GetSrvHandle());
	m_commandList->DrawInstanced(particle.GetParticleCount(), 1, 0, 0);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}


void Renderer::D3D12SimulationApp::RenderGUI(float& deltaTime)
{
	D3D12App::RenderGUI(deltaTime);
}
