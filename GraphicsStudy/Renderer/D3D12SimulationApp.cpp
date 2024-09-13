#include "D3D12SimulationApp.h"
#include <random>

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

	Utility::CreateConstantBuffer(m_device, mSimulationConstantBuffer);

	particle.Initialize(100);
	particle.BuildResources(m_device, m_commandList);

	sphParticle.InitializeSPH(SIMULATION_PARTICLE_SIZE);
	sphParticle.BuildResources(m_device, m_commandList);

	m_commandList->Close();
	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	sphParticle.BuildDescriptors(m_device, m_commandList);
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

	CopyResource(m_commandList, sphParticle.GetReadBack(), sphParticle.GetGpu(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	sphParticle.CopyToCpu();

	std::random_device rd;
	std::mt19937 gen(rd());
	if (fire) {
		std::uniform_real_distribution<> distribVelocityDir(-3.14f, 3.14f);
		fire = false;
		DirectX::SimpleMath::Vector3 ndcPosition = XMFLOAT3((float)mCursorPosition.x / m_screenWidth, (float)mCursorPosition.y / m_screenHeight, 0.f);
		ndcPosition = ndcPosition * 2.f;
		ndcPosition -= XMFLOAT3(1.f, 1.f, 0.f);
		ndcPosition.y *= -1.f;
		//std::cout << ndcPosition.x << ' ' << ndcPosition.y << ' ';
		int sleepCount = 0;
		int fireCount = 200;
		for (UINT i = 0; i < sphParticle.GetParticleCount(); i++)
		{
			if (sphParticle[i].mLife <= 0.f) {
				sphParticle[i].mPosition = ndcPosition;
				float theta = distribVelocityDir(gen);;
				sphParticle[i].mVelocity = XMFLOAT2((float)std::cos(theta) * 2.f, (float)std::sin(theta) * 2.f);
				sphParticle[i].mLife = 3.f;
				++sleepCount;
				if (sleepCount == fireCount)
					break;
			}
		}
		std::cout << "sleep particle Count : " << sleepCount << '\n';
	}
	for (UINT i = 0; i < sphParticle.GetParticleCount(); i++)
	{
		int sleepCount = 0;
		int spawnCount = 100;
		if (sphParticle[i].mLife <= 0.f) 
		{
			sphParticle[i].mPosition = sphParticle[i].mOriginPosition;
			sphParticle[i].mVelocity = sphParticle[i].mOriginVelocity;
			sphParticle[i].mLife = 3.f;
			++sleepCount;
			if (sleepCount == spawnCount)
				break;
		}
	}
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	sphParticle.CopyToGpu(m_device, m_commandList);

	FlushCommandList(m_commandList);

	mSimulationConstantBuffer.mStructure.time = ((deltaTime) < (1 / 60.f) ? deltaTime : (1 / 60.f));
	mSimulationConstantBuffer.UpdateBuffer();
}

void Renderer::D3D12SimulationApp::UpdateGUI(float& deltaTime)
{
	D3D12App::UpdateGUI(deltaTime);
}

void Renderer::D3D12SimulationApp::Render(float& deltaTime)
{
	//std::cout << 1 / deltaTime << ' ';
	//ParticleSimulation(deltaTime);
	SPH(deltaTime);
}
void Renderer::D3D12SimulationApp::ParticleSimulation(float& deltaTime)
{
	SimulationPass(deltaTime);
	SimulationRenderPass(deltaTime);
	PostProcessing(deltaTime);
	CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());
}

void Renderer::D3D12SimulationApp::SPH(float& deltaTime)
{
	//SPHSimulationPass(deltaTime, "SphComputeRho");
	SPHSimulationPass(deltaTime, "SphSimulationCompute");
	SPHSimulationRenderPass(deltaTime);
	//PostProcessing(deltaTime);
	//CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());
	D3D12App::PostProcessing(deltaTime);
	CopyResourceToSwapChain(deltaTime);
	
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
	//m_commandList->ClearUnorderedAccessViewUint(,)
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, particle.GetUavHandle());
	m_commandList->SetComputeRootConstantBufferView(1, mSimulationConstantBuffer.GetGpuAddress());
	UINT dispatchX = (particle.GetParticleCount() - 1 + SIMULATION_PARTICLE_SIZE) / SIMULATION_PARTICLE_SIZE;
	m_commandList->Dispatch(dispatchX, 1, 1);
	//m_commandList->ExecuteIndirect(,)
	ThrowIfFailed(m_commandList->Close());
	
	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::SPHSimulationPass(float& deltaTime,const std::string& psoName)
{
	auto& pso = computePsoList[psoName];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), sphSimulationPassEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		sphParticle.GetUavHeap()
	};
	//m_commandList->ClearUnorderedAccessViewUint(,)
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, sphParticle.GetUavHandle());
	m_commandList->SetComputeRootConstantBufferView(1, mSimulationConstantBuffer.GetGpuAddress());
	UINT dispatchX = (sphParticle.GetParticleCount() - 1 + SIMULATION_PARTICLE_SIZE) / SIMULATION_PARTICLE_SIZE;
	m_commandList->Dispatch(dispatchX, 1, 1);
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

	auto& pso = computePsoList["SimulationPostProcessing"];

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
		m_commandList->SetComputeRootConstantBufferView(1, mSimulationConstantBuffer.GetGpuAddress());
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
	//m_commandList->ClearRenderTargetView(CurrentBackBufferView(), clear, 0, nullptr);
	m_commandList->ClearDepthStencilView(HDRDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.f,0,0,nullptr);
	//m_commandList->ClearRenderTargetView(HDRRendertargetView(), clear, 0, nullptr);

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

void Renderer::D3D12SimulationApp::SPHSimulationRenderPass(float& deltaTime)
{
	auto& pso = passPsoLists["SimulationRenderPass"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(0, 0, 255), simulationRenderPassEvent);

	FLOAT clear[4] = { 0,0,0,0 };
	//m_commandList->ClearRenderTargetView(CurrentBackBufferView(), clear, 0, nullptr);
	m_commandList->ClearDepthStencilView(HDRDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.f, 0, 0, nullptr);
	m_commandList->ClearRenderTargetView(HDRRendertargetView(), clear, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());

	ID3D12DescriptorHeap* pHeaps[] = {
		sphParticle.GetSrvHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, sphParticle.GetSrvHandle());
	m_commandList->DrawInstanced(sphParticle.GetParticleCount(), 1, 0, 0);
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
