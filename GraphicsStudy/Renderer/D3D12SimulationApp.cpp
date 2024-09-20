#include "D3D12SimulationApp.h"
#include <random>
#include <numeric>

#define SIMULATION_PARTICLE_SIZE 768
#define SPH_SIMULATION_PARTICLE_SIZE SIMULATION_PARTICLE_SIZE * 5

Renderer::D3D12SimulationApp::D3D12SimulationApp(const int& width, const int& height)
	:D3D12App(width, height)
{
	bUseTextureApp = false;
	bUseCubeMapApp = false;
	bUseGUI = false;

	m_appName = "SimulationApp";

	//m_backbufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
}

bool Renderer::D3D12SimulationApp::Initialize()
{
	if (!D3D12App::Initialize())
		return false;

	colorLists =
	{
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 0.65f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.3f, 0.0f, 0.5f },
		{ 0.5f, 0.0f, 1.0f }
	};

	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	Utility::CreateConstantBuffer(m_device, m_commandList, mSimulationConstantBuffer);
	Utility::CreateConstantBuffer(m_device, m_commandList, mCFDConstantBuffer);

	particle.Initialize(100);
	particle.BuildResources(m_device, m_commandList);

	sphParticle.InitializeSPH(SPH_SIMULATION_PARTICLE_SIZE);
	sphParticle.BuildResources(m_device, m_commandList);

	stableFluids.Initialize();

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

	if (m_device == nullptr)
		return;

	FlushCommandQueue();
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	stableFluids.BuildResources(m_device, m_commandList, m_screenWidth, m_screenHeight, m_hdrFormat);

	FlushCommandList(m_commandList);
}

void Renderer::D3D12SimulationApp::Update(float& deltaTime)
{
	using DirectX::SimpleMath::Vector3;

	D3D12App::Update(deltaTime);

	mSimulationConstantBuffer.mStructure.deltaTime = (deltaTime < 1 / 300.f ? deltaTime : 1 / 300.f);
	mSimulationConstantBuffer.mStructure.time = (float)m_timer.GetElapsedTime();
	mSimulationConstantBuffer.UpdateBuffer();

	static int colorIndex = 0;
	static float mPrevMouseNdcX = -1.0f;
	static float mPrevMouseNdcY = -1.0f;

	float ndcX = 0.f;
	float ndcY = 0.f;

	if (lMouseButtonClicked) {
		GetCursorPos(&mCursorPosition);
		ScreenToClient(m_mainWnd, &mCursorPosition);

		ndcX = (2.f * ((mCursorPosition.x) / (m_screenWidth - 1.f))) - 1.f;
		ndcY = (2.f * ((mCursorPosition.y) / (m_screenHeight - 1.f))) - 1.f;

		if (fire) { // 처음 누른 경우
			fire = false;
			colorIndex = (colorIndex + 1) % colorLists.size();
			mCFDConstantBuffer.mStructure.velocity = DirectX::SimpleMath::Vector3::Zero;
		}
		else {

			float ndcDeltaX = mouseDeltaX / (float)(m_screenWidth);
			float ndcDeltaY = mouseDeltaY / (float)(m_screenHeight);
			//std::cout << ndcDeltaX << ' ' << ndcX - mPrevMouseNdcX << '\n';
			//mCFDConstantBuffer.mStructure.velocity = DirectX::SimpleMath::Vector3(ndcDeltaX, ndcDeltaY, 0.f);
			mCFDConstantBuffer.mStructure.velocity = DirectX::SimpleMath::Vector3(ndcX, ndcY, 0.f) -
				DirectX::SimpleMath::Vector3(mPrevMouseNdcX, mPrevMouseNdcY, 0.f);
			mCFDConstantBuffer.mStructure.velocity *= 10.0f;
		}
		mCFDConstantBuffer.mStructure.i = mCursorPosition.x;
		mCFDConstantBuffer.mStructure.j = mCursorPosition.y;

		mPrevMouseNdcX = ndcX;
		mPrevMouseNdcY = ndcY;
	}
	else {
		mCFDConstantBuffer.mStructure.i = -1;
		mCFDConstantBuffer.mStructure.j = -1;
	}
	mCFDConstantBuffer.mStructure.color = colorLists[colorIndex];
	mCFDConstantBuffer.mStructure.deltaTime = (deltaTime < 1 / 60.f ? deltaTime : 1 / 60.f);
	//mCFDConstantBuffer.mStructure.deltaTime = 1 / 60.f;
	mCFDConstantBuffer.mStructure.radius = 50.f;
	mCFDConstantBuffer.mStructure.viscosity = mGuiViscosity;
	mCFDConstantBuffer.mStructure.vorticity = mGuiVorticity;
	mCFDConstantBuffer.UpdateBuffer();
}

void Renderer::D3D12SimulationApp::UpdateGUI(float& deltaTime)
{
	ImGui::SliderFloat("Vorticity value", &mGuiVorticity, 0.f, 1.f);
	ImGui::SliderFloat("Viscosity value", &mGuiViscosity, 0.f, 10.f);
}

void Renderer::D3D12SimulationApp::Render(float& deltaTime)
{
	GeneratePerlinNoise(deltaTime);
	//ParticleSimulation(deltaTime);
	//SPH(deltaTime);
	//CFD(deltaTime);
}

void Renderer::D3D12SimulationApp::ParticleSimulation(float& deltaTime)
{
	SimulationPass(deltaTime);
	SimulationRenderPass(deltaTime);
	PostProcessing(deltaTime, "SimulationPostProcessing");
	if (m_backbufferFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) {
		D3D12App::PostProcessing(deltaTime);
		CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());
	}
	else
		CopyResourceToSwapChain(deltaTime);
}

void Renderer::D3D12SimulationApp::GeneratePerlinNoise(float& deltaTime)
{
	PostProcessing(deltaTime, "PerlinNoise");
	if (m_backbufferFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) {
		D3D12App::PostProcessing(deltaTime);
		CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());
	}
	else
		CopyResourceToSwapChain(deltaTime);
}

void Renderer::D3D12SimulationApp::SPH(float& deltaTime)
{
	SPHSimulationPass(deltaTime, "SphComputeRho");
	SPHSimulationPass(deltaTime, "SphComputeForces");
	SPHSimulationPass(deltaTime, "SphSimulationCompute");
	SPHSimulationRenderPass(deltaTime);

	//PostProcessing(deltaTime);
	
	if (m_backbufferFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) {
		D3D12App::PostProcessing(deltaTime);
		CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());
	}
	else
		CopyResourceToSwapChain(deltaTime);
	
	//CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());
}

void Renderer::D3D12SimulationApp::CFD(float& deltaTime)
{
	bCaptureBackbuffer = true;

	CFDPass(deltaTime, "CFDSourcing");
	CFDVorticityPass(deltaTime, "CFDComputeVorticity", 14, 6);
	CFDVorticityPass(deltaTime, "CFDVorticityConfinement", 1, 16);
	CFDDiffusePass(deltaTime);

	CFDComputeDivergencePass(deltaTime);
	CFDComputePressurePass(deltaTime);
	CFDApplyPressurePass(deltaTime);
	
	CFDAdvectionPass(deltaTime);

	//RenderGUI(deltaTime);
	
	if (m_backbufferFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) {
		CopyResource(m_commandList, CurrentBackBuffer(), stableFluids.GetDensityResource(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}

	else
		CopyDensityToSwapChain(deltaTime);

	
	//RenderFont(deltaTime);
	
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

void Renderer::D3D12SimulationApp::SPHSimulationPass(float& deltaTime, const std::string& psoName)
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

void Renderer::D3D12SimulationApp::PostProcessing(float& deltaTime, const std::string& psoName) {

	//D3D12App::Render(deltaTime);

	auto& pso = computePsoList[psoName];

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
		1.f, 0, 0, nullptr);
	//m_commandList->ClearRenderTargetView(HDRRendertargetView(), clear, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), false, nullptr);
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
	m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), false, nullptr);
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());

	ID3D12DescriptorHeap* pHeaps[] = {
		sphParticle.GetSrvHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, sphParticle.GetSrvHandle());
	const float blendColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_commandList->OMSetBlendFactor(blendColor);
	m_commandList->DrawInstanced(sphParticle.GetParticleCount(), 1, 0, 0);

	int fps = (int)(1.f / deltaTime);
	std::wstringstream wss;
	wss << L"FPS : " << fps;
	RenderFonts(wss.str(), m_hdrResourceDescriptors, m_spriteBatchHDR, m_fontHDR, m_commandList);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::RenderFont(float& deltaTime)
{
	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(0, 0, 255), simulationRenderPassEvent);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), false, nullptr);
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	int fps = (int)(1.f / deltaTime);
	std::wstringstream wss;
	wss << L"FPS : " << fps;
	D3D12App::RenderFonts(wss.str(), m_hdrResourceDescriptors, m_spriteBatchHDR, m_fontHDR, m_commandList);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);

	//ThrowIfFailed(m_swapChain->Present(1, 0));
	//m_frameIndex = (m_frameIndex + 1) % m_swapChainCount;

	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::CFDPass(float& deltaTime, const std::string& psoName)
{
	auto& pso = computePsoList[psoName];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdSourcingEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		stableFluids.GetHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, stableFluids.GetHandle(0));
	m_commandList->SetComputeRootConstantBufferView(1, mCFDConstantBuffer.GetGpuAddress());
	m_commandList->Dispatch((UINT)std::ceil(m_screenWidth / 32.f), (UINT)std::ceil(m_screenHeight / 32.f), 1);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::CFDAdvectionPass(float& deltaTime)
{
	CopyResource(m_commandList, stableFluids.GetDensityTempResource(), stableFluids.GetDensityResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	CopyResource(m_commandList, stableFluids.GetVelocityTempResource(), stableFluids.GetVelocityResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	auto& pso = computePsoList["CFDAdvection"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdAdvectionEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		stableFluids.GetHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, stableFluids.GetHandle(0));
	m_commandList->SetComputeRootDescriptorTable(1, stableFluids.GetHandle(10));
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGpuAddress());
	m_commandList->Dispatch((UINT)std::ceil(m_screenWidth / 32.f), (UINT)std::ceil(m_screenHeight / 32.f), 1);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::CFDComputeDivergencePass(float& deltaTime)
{
	auto& pso = computePsoList["CFDComputeDivergence"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdAdvectionEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		stableFluids.GetHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, stableFluids.GetHandle(2));
	m_commandList->SetComputeRootDescriptorTable(1, stableFluids.GetHandle(6));
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGpuAddress());
	m_commandList->Dispatch((UINT)std::ceil(m_screenWidth / 32.f), (UINT)std::ceil(m_screenHeight / 32.f), 1);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::CFDComputePressurePass(float& deltaTime)
{
	auto& pso = computePsoList["CFDComputePressure"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdComputePressureEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());

	for (int i = 0; i <= 100; i++)
	{
		if (i % 2 == 0) {
			ID3D12DescriptorHeap* pHeaps[] = {
				stableFluids.GetPHeap()
			};
			m_commandList->SetDescriptorHeaps(1, pHeaps);
			m_commandList->SetComputeRootDescriptorTable(0, stableFluids.GetPHandle(0));
			m_commandList->SetComputeRootDescriptorTable(1, stableFluids.GetPHandle(1));
		}
		else {
			ID3D12DescriptorHeap* pHeaps[] = {
				stableFluids.GetPTHeap()
			};
			m_commandList->SetDescriptorHeaps(1, pHeaps);
			m_commandList->SetComputeRootDescriptorTable(0, stableFluids.GetPTHandle(0));
			m_commandList->SetComputeRootDescriptorTable(1, stableFluids.GetPTHandle(1));
		}
		m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGpuAddress());
		m_commandList->Dispatch((UINT)std::ceil(m_screenWidth / 32.f), (UINT)std::ceil(m_screenHeight / 32.f), 1);
	}

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::CFDDiffusePass(float& deltaTime)
{
	//CopyResource(m_commandList, stableFluids.GetDensityTempResource(), stableFluids.GetDensityResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//CopyResource(m_commandList, stableFluids.GetVelocityTempResource(), stableFluids.GetVelocityResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	auto& pso = computePsoList["CFDComputeDiffuse"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdDiffuseEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
				stableFluids.GetHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGpuAddress());
	for (int i = 0; i < 10; i++)
	{
		if (i % 2 != 0)
		{
			m_commandList->SetComputeRootDescriptorTable(0, stableFluids.GetHandle(0));
			m_commandList->SetComputeRootDescriptorTable(1, stableFluids.GetHandle(10));
		}
		else
		{
			m_commandList->SetComputeRootDescriptorTable(0, stableFluids.GetHandle(12));
			m_commandList->SetComputeRootDescriptorTable(1, stableFluids.GetHandle(5));
		}

		m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGpuAddress());
		m_commandList->Dispatch((UINT)std::ceil(m_screenWidth / 32.f), (UINT)std::ceil(m_screenHeight / 32.f), 1);
	}

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::CFDVorticityPass(float& deltaTime, const std::string& psoName, int uavIndex, int srvIndex)
{
	auto& pso = computePsoList[psoName];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdVorticityEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		stableFluids.GetHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, stableFluids.GetHandle(uavIndex));
	m_commandList->SetComputeRootDescriptorTable(1, stableFluids.GetHandle(srvIndex));
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGpuAddress());
	m_commandList->Dispatch((UINT)std::ceil(m_screenWidth / 32.f), (UINT)std::ceil(m_screenHeight / 32.f), 1);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());

}

void Renderer::D3D12SimulationApp::CFDApplyPressurePass(float& deltaTime)
{
	auto& pso = computePsoList["CFDApplyPressure"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdApplyPressureEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		stableFluids.GetHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, stableFluids.GetHandle(1)); // Velocity UAV
	m_commandList->SetComputeRootDescriptorTable(1, stableFluids.GetHandle(9)); // Pressure SRV
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGpuAddress());
	m_commandList->Dispatch((UINT)std::ceil(m_screenWidth / 32.f), (UINT)std::ceil(m_screenHeight / 32.f), 1);

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
	/*ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("GUI");
	UpdateGUI(deltaTime);

	ImGui::End();
	ImGui::Render();

	{
		ThrowIfFailed(m_guiCommandAllocator->Reset());
		ThrowIfFailed(m_guiCommandList->Reset(m_guiCommandAllocator.Get(), nullptr));

		PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(0, 255, 255), guiPassEvent);


		m_guiCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			));

		m_guiCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), false, nullptr);
		ID3D12DescriptorHeap* pHeaps[] = { m_guiFontHeap.Get() };
		m_guiCommandList->SetDescriptorHeaps(static_cast<UINT>(std::size(pHeaps)), pHeaps);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_guiCommandList.Get());

		m_guiCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT
			));
	}
	m_guiCommandList->Close();
	ID3D12CommandList* lists[] = { m_guiCommandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

	m_swapChain->Present(0, 0);
	m_frameIndex = (m_frameIndex + 1) % m_swapChainCount;

	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());*/
	D3D12App::RenderGUI(deltaTime);
}

void Renderer::D3D12SimulationApp::FireParticles(const int& fireCount)
{

	std::random_device rd;
	std::mt19937 gen(rd());
	if (fire) {
		GetCursorPos(&mCursorPosition);
		ScreenToClient(m_mainWnd, &mCursorPosition);
		std::uniform_real_distribution<> distribVelocityDir(-3.14f, 3.14f);
		fire = false;
		DirectX::SimpleMath::Vector3 ndcPosition = XMFLOAT3((float)mCursorPosition.x / m_screenWidth, (float)mCursorPosition.y / m_screenHeight, 0.f);
		ndcPosition = ndcPosition * 2.f;
		ndcPosition -= XMFLOAT3(1.f, 1.f, 0.f);
		ndcPosition.y *= -1.f;
		//std::cout << ndcPosition.x << ' ' << ndcPosition.y << ' ';
		int sleepCount = 0;
		for (UINT i = 0; i < sphParticle.GetParticleCount(); i++)
		{
			if (sphParticle[i].life <= 0.f) {
				sphParticle[i].position = ndcPosition;
				float theta = (float)distribVelocityDir(gen);;
				sphParticle[i].velocity = XMFLOAT3((float)std::cos(theta) * 2.f, (float)std::sin(theta) * 2.f, 0.f);
				sphParticle[i].life = 3.f;
				++sleepCount;
				if (sleepCount == fireCount)
					break;
			}
		}
		std::cout << "sleep particle Count : " << sleepCount << '\n';
	}
}

void Renderer::D3D12SimulationApp::CopyDensityToSwapChain(float& deltaTime) {

	auto& pso = utilityPsoLists["CopyDensity"];
	{
		ThrowIfFailed(m_commandAllocator->Reset());
		ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

		PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(0, 255, 0), copyDensityToSwapChainEvent);

		m_commandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			));

		FLOAT clearColor[4] = { 0.f,0.f,0.f,0.f };

		m_commandList->ClearRenderTargetView(CurrentBackBufferView(), clearColor, 0, nullptr);
		m_commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), false, nullptr);
		m_commandList->RSSetScissorRects(1, &m_scissorRect);
		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());
		m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D12DescriptorHeap* pHeaps[] = { stableFluids.GetHeap() };
		m_commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(pHeaps)), pHeaps);
		m_commandList->SetGraphicsRootDescriptorTable(0, stableFluids.GetHandle(5));

		m_screenMesh->Render(deltaTime, m_commandList, false);

		m_commandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT
			));
	}

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());
}