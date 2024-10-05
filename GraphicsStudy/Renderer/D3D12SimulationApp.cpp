#include "D3D12SimulationApp.h"
#include <random>
#include <numeric>

#define SIMULATION_PARTICLE_SIZE 768
#define SPH_SIMULATION_PARTICLE_SIZE SIMULATION_PARTICLE_SIZE * 5

Renderer::D3D12SimulationApp::D3D12SimulationApp(const int& width, const int& height)
	:D3D12App(width, height)
{
	bUseTextureApp = false;
	bUseCubeMapApp = true;
	bUseGUI = false;
	bRenderCubeMap = false;

	m_appName = "SimulationApp";

	m_backbufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
}

Renderer::D3D12SimulationApp::~D3D12SimulationApp()
{
	delete mSmoke;
}

bool Renderer::D3D12SimulationApp::Initialize()
{
	if (!D3D12App::Initialize())
		return false;

	DirectX::SimpleMath::Vector3 pos(0, 0, -3);
	m_camera->SetPositionAndDirection(pos, XMFLOAT3(0, 0, 1));

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

	mSimulationConstantBuffer.Initialize(m_device, m_commandList);
	mCFDConstantBuffer.Initialize(m_device, m_commandList);
	mVolumeConstantBuffer.Initialize(m_device, m_commandList);
	mCubeMapConstantBuffer.Initialize(m_device, m_commandList);

	particle.Initialize(100);
	particle.BuildResources(m_device, m_commandList, m_hdrFormat, m_screenWidth, m_screenHeight);

	sphParticle.InitializeSPH(SPH_SIMULATION_PARTICLE_SIZE);
	sphParticle.BuildResources(m_device, m_commandList);

	stableFluids.Initialize();

	InitSimulationScene();

	mCloud.InitVolumeMesh(2.f, 1.f, 1.f, m_device, m_commandList);
	mCloud.Initiailize(128, 64, 64, DXGI_FORMAT_R16_FLOAT, m_device, m_commandList);

	upscale = 2;
	smokeWidth = 128;
	smokeHeight = 64;
	smokeDepth = 64;
	mSmoke = new Volume();
	mSmoke->Initialize(smokeWidth, smokeHeight, smokeDepth, upscale, m_device, m_commandList);

	m_commandList->Close();
	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	sphParticle.BuildDescriptors(m_device, m_commandList);
	particle.BuildDescriptors(m_device, m_commandList);

	GeneratePerlinNoise();

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

void Renderer::D3D12SimulationApp::InitSimulationScene() {

	m_cubeMap = std::make_shared<Core::StaticMesh>();
	m_cubeMap->Initialize(GeometryGenerator::SimpleCubeMapBox(500.f), m_device, m_commandList);
	m_cubeMap->SetTexturePath(std::wstring(L"Outdoor") + L"EnvHDR.dds");
}

void Renderer::D3D12SimulationApp::OnResize()
{
	//m_timer.Stop();

	D3D12App::OnResize();

	if (m_device == nullptr)
		return;

	FlushCommandQueue();
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	stableFluids.BuildResources(m_device, m_commandList, m_screenWidth, m_screenHeight, m_hdrFormat);

	FlushCommandList(m_commandList);

	//m_timer.Start();
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

	mCFDConstantBuffer.mStructure.time = (float)m_timer.GetElapsedTime();
	mCFDConstantBuffer.mStructure.sourceStrength = mGuiSourceStrength;
	mCFDConstantBuffer.mStructure.upScale = upscale;

	mCFDConstantBuffer.mStructure.dxBase = XMFLOAT3(1.f / smokeWidth, 1.f / smokeHeight, 1.f / smokeDepth);
	mCFDConstantBuffer.mStructure.dxUp = 
		XMFLOAT3(1.f / (smokeWidth * upscale),
				 1.f / (smokeHeight * upscale),
				 1.f / (smokeDepth * upscale));

	mCFDConstantBuffer.UpdateBuffer();

	mCubeMapConstantBuffer.mStructure.expose = 2.f;
	mCubeMapConstantBuffer.mStructure.lodLevel = 0.f;
	mCubeMapConstantBuffer.UpdateBuffer();

	mPostprocessingConstantBuffer.mStructure.bUseGamma = false;
	mPostprocessingConstantBuffer.UpdateBuffer();

	mCloud.Update(deltaTime);
	mSmoke->Update(deltaTime);
}

void Renderer::D3D12SimulationApp::UpdateGUI(float& deltaTime)
{
	ImGui::SliderFloat("Vorticity value", &mGuiVorticity, 0.f, 3.f);
	//ImGui::SliderFloat("Viscosity value", &mGuiViscosity, 0.f, 10.f);
	ImGui::SliderFloat("Source Strength", &mGuiSourceStrength, 0.1f, 3.f);
}

void Renderer::D3D12SimulationApp::Render(float& deltaTime)
{
	//RenderNoise(deltaTime);
	//ParticleSimulation(deltaTime);
	//SPH(deltaTime); 
	//CFD(deltaTime);
	//VolumeRendering(deltaTime);
	SmokeSimulationPass(deltaTime);
}

void Renderer::D3D12SimulationApp::ParticleSimulation(float& deltaTime)
{
	SimulationPass(deltaTime);
	SimulationRenderPass(deltaTime);
	PostProcessing(deltaTime, "SimulationPostProcessing", HDRRenderTargetBuffer(), m_hdrUavHeap.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	if (m_backbufferFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) {
		D3D12App::PostProcessing(deltaTime);
		CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());
	}
	else
		D3D12App::CopyResourceToSwapChain(deltaTime);
}

void Renderer::D3D12SimulationApp::RenderNoise(float& deltaTime)
{
	//bCaptureBackbuffer = true;
	CopyResource(m_commandList, HDRRenderTargetBuffer(), particle.GetRandomResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	D3D12App::CopyResourceToSwapChain(deltaTime);
	//D3D12SimulationApp::CopyResourceToSwapChain(deltaTime, particle.GetHeap(), particle.GetHandle(3));
}

void Renderer::D3D12SimulationApp::GeneratePerlinNoise()
{
	float deltaTime = 1 / 60.f;
	PostProcessing(deltaTime, "PerlinNoise", particle.GetRandomResource(), particle.GetHeap(), D3D12_RESOURCE_STATE_COMMON, 2);
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
		D3D12App::CopyResourceToSwapChain(deltaTime);

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
		CopyResourceToSwapChain(deltaTime, stableFluids.GetHeap(), stableFluids.GetHandle(5));


	//RenderFont(deltaTime);

}

void Renderer::D3D12SimulationApp::SmokeSimulationPass(float& deltaTime) {

	bRenderSmoke = true;
	//bCaptureBackbuffer = true;

	RenderCubeMap(deltaTime);

	CopyResource(m_commandList, 
		mSmoke->GetVelocityUpTempResource(), //dest
		mSmoke->GetVelocityUpResource(), //src
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, // dest state
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS  // src state
	);

	//SmokeVorticityPass(deltaTime, "SmokeComputeVorticity", 0);
	SmokeVorticityPass(deltaTime, "SmokeVorticityConfinement", 1);

	SmokeDownSamplePass(deltaTime);

	// Sourcing Smoke Density
	SmokeSourcingDensityPass(deltaTime);
		
	// Compute Divergence of Velocity
	SmokeComputeDivergencePass(deltaTime);

	SmokeComputePressurePass(deltaTime);
	SmokeApplyPressurePass(deltaTime);

	SmokeDiffUpSamplePass(deltaTime);
	
	SmokeAdvectionPass(deltaTime);

	RenderBoundingBox(deltaTime);
	RenderVolumMesh(deltaTime);

	if (m_backbufferFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) {
		//D3D12App::PostProcessing(deltaTime);
		CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());
	}
	else
		D3D12App::CopyResourceToSwapChain(deltaTime);
	//RenderFont(deltaTime);
}


void Renderer::D3D12SimulationApp::VolumeRendering(float& deltaTime) {

	bRenderCloud = true;

	//RenderCubeMap(deltaTime);
	ComputeVolumeDensityPass(deltaTime);
	RenderBoundingBox(deltaTime);
	RenderVolumMesh(deltaTime);

	if (m_backbufferFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) {
		//D3D12App::PostProcessing(deltaTime);
		CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());
	}
	else
		D3D12App::CopyResourceToSwapChain(deltaTime);
}


void Renderer::D3D12SimulationApp::SimulationPass(float& deltaTime)
{
	auto& pso = computePsoList["SimulationCompute"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), simulationPassEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		particle.GetHeap()
	};
	//m_commandList->ClearUnorderedAccessViewUint(,)
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, particle.GetUavHandle());
	m_commandList->SetComputeRootDescriptorTable(1, particle.GetHandle(3)); // random srv

	m_commandList->SetComputeRootConstantBufferView(2, mSimulationConstantBuffer.GetGPUVirtualAddress());
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
		sphParticle.GetHeap()
	};
	//m_commandList->ClearUnorderedAccessViewUint(,)
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, sphParticle.GetUavHandle());
	m_commandList->SetComputeRootConstantBufferView(1, mSimulationConstantBuffer.GetGPUVirtualAddress());
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

void Renderer::D3D12SimulationApp::PostProcessing(float& deltaTime, const std::string& psoName,
	ID3D12Resource* hdrResource, ID3D12DescriptorHeap* resourceUavHeap, D3D12_RESOURCE_STATES resourceState, int heapIndex) {

	//D3D12App::Render(deltaTime);

	auto& pso = computePsoList[psoName];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

	{
		PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(0, 0, 255), postprocessingEvent);

		if (resourceState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
			m_commandList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					hdrResource,
					resourceState,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS
				));
		}

		m_commandList->SetComputeRootSignature(pso.GetRootSignature());

		// UAV Heap 
		ID3D12DescriptorHeap* ppHeaps[] = { resourceUavHeap };
		m_commandList->SetDescriptorHeaps(1, ppHeaps);

		UINT offset = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CD3DX12_GPU_DESCRIPTOR_HANDLE handle(resourceUavHeap->GetGPUDescriptorHandleForHeapStart(), heapIndex, offset);
		m_commandList->SetComputeRootDescriptorTable(0, handle);
		m_commandList->SetComputeRootConstantBufferView(1, mSimulationConstantBuffer.GetGPUVirtualAddress());
		//m_commandList->SetComputeRootConstantBufferView(1, m_csBuffer->GetGPUVirtualAddress());
		m_commandList->Dispatch((UINT)ceil(m_screenWidth / 32.f), (UINT)ceil(m_screenHeight / 32.f), 1);

		if (resourceState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
			m_commandList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					hdrResource,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
					resourceState));
		}

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
		particle.GetHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, particle.GetHandle(0)); // particle uav
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
		sphParticle.GetHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, sphParticle.GetHandle(0));
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

	int fps = (int)m_timer.GetFrameRate();
	std::wstringstream wss;
	wss << L"FPS : " << fps << '\n'
		<< L"time : " << (int)m_timer.GetElapsedTime();

	D3D12App::RenderFonts(wss.str(), m_bakcbufferResourceDescriptors, m_spriteBatchBakcbuffer, m_fontBakcbuffer, m_commandList);

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
	m_commandList->SetComputeRootConstantBufferView(1, mCFDConstantBuffer.GetGPUVirtualAddress());
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
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGPUVirtualAddress());
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
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdComputeDivergenceEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		stableFluids.GetHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, stableFluids.GetHandle(2));
	m_commandList->SetComputeRootDescriptorTable(1, stableFluids.GetHandle(6));
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGPUVirtualAddress());
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
		m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGPUVirtualAddress());
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
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGPUVirtualAddress());
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

		m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGPUVirtualAddress());
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
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGPUVirtualAddress());
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
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGPUVirtualAddress());
	m_commandList->Dispatch((UINT)std::ceil(m_screenWidth / 32.f), (UINT)std::ceil(m_screenHeight / 32.f), 1);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::ComputeVolumeDensityPass(float& deltaTime)
{
	auto& pso = computePsoList["ComputeVolumeDensity"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdApplyPressureEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		mCloud.GetTextureHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, mCloud.GetUavGPUHandle()); // density UAV
	m_commandList->SetComputeRootConstantBufferView(1, mVolumeConstantBuffer.GetGPUVirtualAddress());
	m_commandList->Dispatch((UINT)std::ceil(mCloud.GetWidth() / 16.f), (UINT)std::ceil(mCloud.GetHeight() / 16.f), (UINT)std::ceil(mCloud.GetDepth() / 4.f));

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::SmokeVorticityPass(float& deltaTime, const std::string& psoName, int index)
{
	auto& pso = computePsoList[psoName];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdVorticityEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		mSmoke->GetVorticityHeap(index)
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, mSmoke->GetVorticityHandle(index, 0));
	m_commandList->SetComputeRootDescriptorTable(1, mSmoke->GetVorticityHandle(index, 1));
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGPUVirtualAddress());
	mSmoke->DispatchUpScale(m_commandList);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}


void Renderer::D3D12SimulationApp::SmokeDownSamplePass(float& deltaTime)
{
	auto& pso = computePsoList["SmokeDownSample"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdSourcingEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		mSmoke->GetDownSampleHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, mSmoke->GetDownSampleHandle(0));
	m_commandList->SetComputeRootDescriptorTable(1, mSmoke->GetDownSampleHandle(2));
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGPUVirtualAddress());
	mSmoke->Dispatch(m_commandList);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}


void Renderer::D3D12SimulationApp::SmokeSourcingDensityPass(float& deltaTime)
{
	CopyResource(m_commandList, mSmoke->GetDensityTempResource(), mSmoke->GetDensityResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	CopyResource(m_commandList, mSmoke->GetVelocityTempResource(), mSmoke->GetVelocityResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	auto& pso = computePsoList["SmokeSourcingDensity"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdSourcingEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		mSmoke->GetHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, mSmoke->GetHandle(0));
	m_commandList->SetComputeRootConstantBufferView(1, mCFDConstantBuffer.GetGPUVirtualAddress());
	mSmoke->Dispatch(m_commandList);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::SmokeComputeDivergencePass(float& deltaTime)
{
	auto& pso = computePsoList["SmokeComputeDivergence"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdComputeDivergenceEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		mSmoke->GetDivergenceHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, mSmoke->GetDivergenceHandle(0)); // divergence pressure pressureTemp uav
	m_commandList->SetComputeRootDescriptorTable(1, mSmoke->GetDivergenceHandle(3)); // velocity srv
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGPUVirtualAddress());
	mSmoke->Dispatch(m_commandList);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::SmokeComputePressurePass(float& deltaTime)
{
	auto& pso = computePsoList["SmokeComputePressure"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdComputePressureEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());

	for (int i = 0; i < 20; i++)
	{
		if (i % 2 == 0) {
			ID3D12DescriptorHeap* pHeaps[] = {
				mSmoke->GetCPHeap(0)
			};
			m_commandList->SetDescriptorHeaps(1, pHeaps);
			m_commandList->SetComputeRootDescriptorTable(0, mSmoke->GetCPHandle(0, 0));
			m_commandList->SetComputeRootDescriptorTable(1, mSmoke->GetCPHandle(0, 1));
		}
		else {
			ID3D12DescriptorHeap* pHeaps[] = {
				mSmoke->GetCPHeap(1)
			};
			m_commandList->SetDescriptorHeaps(1, pHeaps);
			m_commandList->SetComputeRootDescriptorTable(0, mSmoke->GetCPHandle(1, 0));
			m_commandList->SetComputeRootDescriptorTable(1, mSmoke->GetCPHandle(1, 1));
		}
		m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGPUVirtualAddress());
		mSmoke->Dispatch(m_commandList);
	}

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}


void Renderer::D3D12SimulationApp::SmokeApplyPressurePass(float& deltaTime)
{
	auto& pso = computePsoList["SmokeApplyPressure"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdApplyPressureEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		mSmoke->GetHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, mSmoke->GetHandle(1)); // Velocity UAV
	m_commandList->SetComputeRootDescriptorTable(1, mSmoke->GetHandle(10)); // Pressure SRV
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGPUVirtualAddress());
	mSmoke->Dispatch(m_commandList);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::SmokeDiffUpSamplePass(float& deltaTime)
{
	auto& pso = computePsoList["SmokeDiffUpSample"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdSourcingEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		mSmoke->GetUpSampleHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, mSmoke->GetUpSampleHandle(0));
	m_commandList->SetComputeRootDescriptorTable(1, mSmoke->GetUpSampleHandle(2));
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGPUVirtualAddress());
	mSmoke->DispatchUpScale(m_commandList);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::SmokeAdvectionPass(float& deltaTime)
{
	CopyResource(m_commandList,
		mSmoke->GetVelocityUpTempResource(), //dest
		mSmoke->GetVelocityUpResource(), //src
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, // dest state
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS  // src state
	);
	CopyResource(m_commandList,
		mSmoke->GetDensityUpTempResource(), //dest
		mSmoke->GetDensityUpResource(), //src
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, // dest state
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS  // src state
	);
	auto& pso = computePsoList["SmokeAdvection"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cfdAdvectionEvent);

	m_commandList->SetComputeRootSignature(pso.GetRootSignature());
	ID3D12DescriptorHeap* pHeaps[] = {
		mSmoke->GetAdvectionHeap()
	};
	m_commandList->SetDescriptorHeaps(1, pHeaps);
	m_commandList->SetComputeRootDescriptorTable(0, mSmoke->GetAdvectionHandle(0)); // densityUp velocityUp uav
	m_commandList->SetComputeRootDescriptorTable(1, mSmoke->GetAdvectionHandle(2)); // densityUp velocityUp Temp srv
	m_commandList->SetComputeRootConstantBufferView(2, mCFDConstantBuffer.GetGPUVirtualAddress());
	mSmoke->DispatchUpScale(m_commandList);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* pCmdLists[] = {
		m_commandList.Get()
	};
	m_commandQueue->ExecuteCommandLists(1, pCmdLists);
	FlushCommandQueue();

	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::RenderVolumMesh(float& deltaTime)
{
	auto& pso = passPsoLists["RenderVolumePass"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	{
		PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), volumeRenderEvent);

		m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());

		m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->RSSetScissorRects(1, &m_scissorRect);
		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());

		// View Proj Matrix Constant Buffer 
		m_commandList->SetGraphicsRootConstantBufferView(2, m_passConstantBuffer.GetGPUVirtualAddress());

		if (bRenderCloud) {
			// Texture SRV Heap 
			ID3D12DescriptorHeap* ppSrvHeaps[] = { mCloud.GetTextureHeap() };
			m_commandList->SetDescriptorHeaps(_countof(ppSrvHeaps), ppSrvHeaps);
			m_commandList->SetGraphicsRootDescriptorTable(0, mCloud.GetSrvGPUHandle());
			mCloud.Render(deltaTime, m_commandList, true);
		}
		if (bRenderSmoke) {
			// Texture SRV Heap 
			ID3D12DescriptorHeap* ppSrvHeaps[] = { mSmoke->GetHeap() };
			m_commandList->SetDescriptorHeaps(_countof(ppSrvHeaps), ppSrvHeaps);
			m_commandList->SetGraphicsRootDescriptorTable(0, mSmoke->GetHandle(22)); // density Srv
			mSmoke->Render(deltaTime, m_commandList, true);
		}
	}
	FlushCommandList(m_commandList);
	PIXEndEvent(m_commandQueue.Get());
}


void Renderer::D3D12SimulationApp::RenderBoundingBox(float& deltaTime)
{
	auto& pso = passPsoLists["BoundingBoxPass"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), renderBoundingBoxPassEvent);

	FLOAT clearColor[4] = { 0.f,0.f,0.f,0.f };
	if (!bRenderCubeMap)
	{
		m_commandList->ClearRenderTargetView(HDRRendertargetView(), clearColor, 0, nullptr);
		m_commandList->ClearDepthStencilView(HDRDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			1.f, 0, 0, nullptr);
	}
	m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());
	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	m_commandList->RSSetViewports(1, &m_viewport);

	m_commandList->SetGraphicsRootConstantBufferView(1, m_passConstantBuffer.GetGPUVirtualAddress());

	if (bRenderCloud)
		mCloud.RenderBoundingBox(deltaTime, m_commandList);
	if (bRenderSmoke)
		mSmoke->RenderBoundingBox(deltaTime, m_commandList);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* plists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12SimulationApp::RenderCubeMap(float& deltaTime)
{
	auto& pso = cubePsoLists["SimulationCubeMap"];
	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cubeMapPassEvent);

	if (bRenderCubeMap)
	{
		FLOAT clearColor[4] = { 0.f,0.f,0.f,0.f };

		m_commandList->ClearRenderTargetView(HDRRendertargetView(), clearColor, 0, nullptr);
		m_commandList->ClearDepthStencilView(HDRDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			1.f, 0, 0, nullptr);
		m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());
		m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());
		m_commandList->RSSetScissorRects(1, &m_scissorRect);
		m_commandList->RSSetViewports(1, &m_viewport);

		// View Proj Matrix Constant Buffer 
		m_commandList->SetGraphicsRootConstantBufferView(1, m_passConstantBuffer.GetGPUVirtualAddress());

		//CubeMap Expose & LodLevel
		m_commandList->SetGraphicsRootConstantBufferView(2, mCubeMapConstantBuffer.GetGPUVirtualAddress());

		// CubeMap Heap 
		ID3D12DescriptorHeap* ppCubeHeaps[] = { m_cubeMapTextureHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppCubeHeaps), ppCubeHeaps);
		CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_cubeMapTextureHeap->GetGPUDescriptorHandleForHeapStart());

		if (m_cubeTextureMap.count(m_cubeMap->GetTexturePath()) > 0) {
			handle.Offset(m_cubeTextureMap[m_cubeMap->GetTexturePath()], m_csuHeapSize);
		}
		else {
			handle.Offset(m_cubeTextureMap[L"DefaultEnvHDR.dds"], m_csuHeapSize);
		}
		m_commandList->SetGraphicsRootDescriptorTable(0, handle);

		m_cubeMap->Render(deltaTime, m_commandList, false);
	}

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* plists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());
}


void Renderer::D3D12SimulationApp::RenderGUI(float& deltaTime)
{
	ImGui_ImplDX12_NewFrame();
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
	PIXEndEvent(m_commandQueue.Get());
	//D3D12App::RenderGUI(deltaTime);
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

// src srv handle
void Renderer::D3D12SimulationApp::CopyResourceToSwapChain(float& deltaTime, ID3D12DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE handle) {

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

		ID3D12DescriptorHeap* pHeaps[] = { heap };
		m_commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(pHeaps)), pHeaps);
		//m_commandList->SetGraphicsRootDescriptorTable(0, stableFluids.GetHandle(5));
		m_commandList->SetGraphicsRootDescriptorTable(0, handle);

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