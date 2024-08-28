#include "D3D12App.h"
#include "GeometryGenerator.h"
#include "Renderer.h"
#include "directxtk12/DDSTextureLoader.h"
#include "DirectXTex.h"
#include "AnimationClip.h"

#include <DirectXTexEXR.h>
#include <tuple>
#include "pix3.h"
#include <fp16.h>
#include <ctime>

#pragma warning(disable : 4996)

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

using namespace Core;

Renderer::D3D12App::D3D12App(const int& width, const int& height)
	:SimpleApp(width, height),
	bUseWarpAdapter(false),
	m_minimumFeatureLevel(D3D_FEATURE_LEVEL_11_0),
	m_viewport(D3D12_VIEWPORT()),
	m_scissorRect(D3D12_RECT())
{
	m_passConstantData = new GlobalVertexConstantData();
	m_ligthPassConstantData = new LightPassConstantData();

	textureBasePath = L"Textures/";
	cubeMapTextureBasePath = L"Textures/CubeMaps/";
	exrTextureBasePath = L"Textures/HDR/";

	bUseGUI = true;
}

Renderer::D3D12App::~D3D12App()
{
	std::cout << "~D3D12App" << std::endl;

	if (m_device != nullptr)
		FlushCommandQueue();
	for (int i = 0; i < m_swapChainCount; i++)
	{
		m_renderTargets[i].Reset();
	}
	m_msaaDepthStencilBuffer.Reset();
}

bool Renderer::D3D12App::Initialize()
{
	if (!SimpleApp::Initialize()) {
		return false;
	}
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	CreateConstantBuffer();

	// Init PSO & RootSignature
	Renderer::Initialize();
	Renderer::Finalize(m_device);

	if (bUseTextureApp) {
		CreateTextures();
	}
	if (bUseCubeMapApp) {
		CreateCubeMapTextures();
	}
	if (bUseDefaultSceneApp) {
		InitScene();
	}
	CreateFontFromFile(L"Fonts/default.spritefont", m_font, m_spriteBatch, m_resourceDescriptors, false, m_hdrFormat, m_depthStencilFormat);
	CreateFontFromFile(L"Fonts/default.spritefont", m_msaaFont, m_msaaSpriteBatch, m_msaaResourceDescriptors, true, m_hdrFormat, m_depthStencilFormat);

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

	FlushCommandQueue();

	InitGUI();

	return true;
}

bool Renderer::D3D12App::InitGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	// io.Fonts->TexID = (ImTextureID)m_guiFont->GetSpriteSheet().ptr;

	ImGui::StyleColorsLight();
	const char* fontPath = "Fonts/Hack-Bold.ttf";
	float fontSize = 15.0f;
	// 폰트 로드
	io.Fonts->AddFontFromFileTTF(fontPath, fontSize);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_guiFontHeap));

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(m_mainWnd);
	/*ImGui_ImplDX12_Init(m_device.Get(), m_swapChainCount, m_backbufferFormat,
		m_guiResourceDescriptors->Heap(),
		m_guiResourceDescriptors->GetFirstCpuHandle(),
		m_guiResourceDescriptors->GetFirstGpuHandle());*/
	ImGui_ImplDX12_Init(m_device.Get(), m_swapChainCount, m_backbufferFormat,
		m_guiFontHeap.Get(),
		m_guiFontHeap->GetCPUDescriptorHandleForHeapStart(),
		m_guiFontHeap->GetGPUDescriptorHandleForHeapStart());

	return true;
}

bool Renderer::D3D12App::InitDirectX()
{
	UINT dxgiFactoryFlags = 0;

	// Enable the debug layer
#if defined(DEBUG) || defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif
	// Create Factory and Device 
	ComPtr<ID3D12Device> device;
	CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory));
	if (bUseWarpAdapter) {
		m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&m_warpAdapter));

		ThrowIfFailed(D3D12CreateDevice(
			m_warpAdapter.Get(),
			m_minimumFeatureLevel,
			IID_PPV_ARGS(&device)));
	}
	else {
		ThrowIfFailed(D3D12CreateDevice(
			nullptr,
			m_minimumFeatureLevel,
			IID_PPV_ARGS(&device)));
	}
	ThrowIfFailed(device.As(&m_device));

	// Check Msaa Quality
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaData;
	ZeroMemory(&msaaData, sizeof(msaaData));
	msaaData.Format = m_msaaFormat;
	msaaData.NumQualityLevels = m_numQualityLevels;
	msaaData.SampleCount = m_sampleCount;

	ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaData, sizeof(msaaData)));
	std::cout << "Msaa Num Quality Level : " << msaaData.NumQualityLevels << std::endl;
	m_numQualityLevels = msaaData.NumQualityLevels;
	msaaQuality = m_numQualityLevels;
	m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));

	CreateCommandObjects();
	CreateDescriptorHeaps();

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 scDesc;
	ZeroMemory(&scDesc, sizeof(scDesc));
	scDesc.Width = m_screenWidth;
	scDesc.Height = m_screenHeight;
	scDesc.Format = m_backbufferFormat;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scDesc.BufferCount = m_swapChainCount;

	ComPtr<IDXGISwapChain1> swapChain;

	ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		m_mainWnd,
		&scDesc,
		nullptr,
		nullptr,
		swapChain.ReleaseAndGetAddressOf()
	));

	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	return true;
}

void Renderer::D3D12App::OnResize()
{
	if (m_device == nullptr)
		return;

	FlushCommandQueue();
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	for (int i = 0; i < m_swapChainCount; i++)
	{
		m_renderTargets[i].Reset();
	}
	m_msaaDepthStencilBuffer.Reset();
	m_hdrRenderTarget.Reset();
	m_hdrDepthStencilBuffer.Reset();
	m_msaaRenderTarget.Reset();

	ThrowIfFailed(m_swapChain->ResizeBuffers(m_swapChainCount,
		m_screenWidth,
		m_screenHeight,
		DXGI_FORMAT_UNKNOWN,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	D3D12_UNORDERED_ACCESS_VIEW_DESC swapChainuavDesc;
	swapChainuavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	swapChainuavDesc.Format = m_backbufferFormat;
	swapChainuavDesc.Texture2D.MipSlice = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_swapChainRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < m_swapChainCount; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_renderTargets[i].ReleaseAndGetAddressOf()));
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_rtvHeapSize);
	}

	CreateDepthBuffer(m_msaaDepthStencilBuffer, MsaaDepthStencilView(), true);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_msaaDepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	CreateDepthBuffer(m_hdrDepthStencilBuffer, HDRDepthStencilView(), false);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_hdrDepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	D3D12_RESOURCE_FLAGS uavFlag = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	D3D12_RESOURCE_FLAGS rtvFlag = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	CreateResourceBuffer(m_msaaRenderTarget, m_msaaFormat, true, rtvFlag);
	CreateResourceBuffer(m_hdrRenderTarget, m_hdrFormat, false, uavFlag);
	
	UINT copyBufferSize = m_screenWidth * m_screenHeight * 4 * sizeof(uint16_t);
	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(copyBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&imageBuffer));

	/*CreateResourceBuffer(m_copyBuffer, m_hdrFormat, false, D3D12_RESOURCE_FLAG_NONE, 
		D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, false);
	m_hdrRenderTarget->SetName(L"HDR RenderTarget");*/
	//UINT dataSize = m_screenWidth * m_screenHeight * 4 * sizeof(uint16_t);
	//Utility::CreateBuffer(m_device, D3D12_HEAP_TYPE_READBACK, D3D12_HEAP_FLAG_NONE, dataSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, m_copyBuffer);

	CreateResourceView(m_msaaRenderTarget, m_msaaFormat, true, m_msaaRtvHeap->GetCPUDescriptorHandleForHeapStart(), m_device, DescriptorType::RTV);
	CreateResourceView(m_hdrRenderTarget, m_hdrFormat, false, m_hdrRtvHeap->GetCPUDescriptorHandleForHeapStart(), m_device, DescriptorType::RTV);
	CreateResourceView(m_hdrRenderTarget, m_hdrFormat, false, m_hdrUavHeap->GetCPUDescriptorHandleForHeapStart(), m_device, DescriptorType::UAV);
	CreateResourceView(m_hdrRenderTarget, m_hdrFormat, false, m_hdrSrvHeap->GetCPUDescriptorHandleForHeapStart(), m_device, DescriptorType::SRV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE gPass_rtvHeapHandle(m_geometryPassRtvHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE gPass_srvHeapHandle(m_geometryPassSrvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < geometryPassRtvNum; i++)
	{
		CreateResourceBuffer(m_geometryPassResources[i], m_geometryPassFormat, false, rtvFlag);

		CreateResourceView(m_geometryPassResources[i], m_geometryPassFormat, false, gPass_rtvHeapHandle, m_device, DescriptorType::RTV);
		CreateResourceView(m_geometryPassResources[i], m_geometryPassFormat, false, gPass_srvHeapHandle, m_device, DescriptorType::SRV);
		gPass_rtvHeapHandle.Offset(1, m_rtvHeapSize);
		gPass_srvHeapHandle.Offset(1, m_csuHeapSize);
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE gPassMsaa_rtvHeapHandle(m_geometryPassMsaaRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < geometryPassRtvNum; i++)
	{
		CreateResourceBuffer(m_geometryPassMsaaResources[i], m_geometryPassFormat, true, rtvFlag);
		CreateResourceView(m_geometryPassMsaaResources[i], m_geometryPassFormat, true, gPassMsaa_rtvHeapHandle, m_device, DescriptorType::RTV);
		gPassMsaa_rtvHeapHandle.Offset(1, m_rtvHeapSize);
	}

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();

	m_viewport = { 0.f ,0.f,(FLOAT)m_screenWidth ,(FLOAT)m_screenHeight, 0.f, 1.f };
	m_scissorRect = { 0 ,0,static_cast<LONG>(m_screenWidth), static_cast<LONG>(m_screenHeight) };
}

void Renderer::D3D12App::Update(float& deltaTime)
{
	m_inputHandler->ExicuteCommand(m_camera.get(), deltaTime, bIsFPSMode);

	m_passConstantData->ViewMat = m_camera->GetViewMatrix();
	m_passConstantData->ProjMat = m_camera->GetProjMatrix();

	m_passConstantData->ViewMat = m_passConstantData->ViewMat.Transpose();
	m_passConstantData->ProjMat = m_passConstantData->ProjMat.Transpose();

	m_ligthPassConstantData->eyePos = m_camera->GetPosition();
	m_ligthPassConstantData->lod = 0.f;

	memcpy(m_pCbvDataBegin, m_passConstantData, sizeof(GlobalVertexConstantData));
	memcpy(m_pLPCDataBegin, m_ligthPassConstantData, sizeof(LightPassConstantData));


	for (auto& mesh : m_staticMeshes) {
		mesh->Update(deltaTime);
	}
	for (auto& fbx : m_fbxList) {
		fbx->Update(deltaTime);
	}
}

void Renderer::D3D12App::UpdateGUI(float& deltaTime)
{
	if (ImGui::BeginCombo("Mode", currRenderMode.c_str())) // The second parameter is the label previewed before opening the combo.
	{
		for (int n = 0; n < modePsoListNames.size(); n++)
		{
			bool is_selected = (currRenderMode == modePsoListNames[n]); // You can store your selection however you want, outside or inside your objects
			if (ImGui::Selectable(modePsoListNames[n].c_str(), is_selected)) {
				currRenderMode = modePsoListNames[n];
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	//ImGui::SliderFloat3("Light Position", (float*)&gui_lightPos, -10.f, 10.f);
	/*ImGui::SliderFloat("Material Shineness", &gui_shineness, 1.f, 100.f);
	ImGui::SliderFloat("Material Diffuse", &gui_diffuse, 0.f, 1.f);
	ImGui::SliderFloat("Material Specualr", &gui_specular, 0.f, 1.f);*/
	//ImGui::SliderFloat("LOD", &gui_lod, 0.f, 10.f);
	//ImGui::SliderFloat("Frame", &gui_frame, 0.f, 49.f);
}

void Renderer::D3D12App::Render(float& deltaTime)
{
	RenderMeshes(deltaTime);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

	FlushCommandQueue();

	RenderCubeMap(deltaTime);

}

void Renderer::D3D12App::RenderGUI(float& deltaTime)
{
	if (bUseGUI) {
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
	}
	ThrowIfFailed(m_swapChain->Present(0, 0));
	m_frameIndex = (m_frameIndex + 1) % m_swapChainCount;

	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12App::RenderMeshes(float& deltaTime) {

	auto& pso = modePsoLists[currRenderMode];
	msaaMode = false;
	if (currRenderMode == "Msaa") {
		msaaMode = true;
	}

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

	{
		FLOAT clearColor[4] = { 0.f,0.f,0.f,0.f };
		if (msaaMode) {
			m_commandList->ClearRenderTargetView(MsaaRenderTargetView(), clearColor, 0, nullptr);
			m_commandList->ClearDepthStencilView(MsaaDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
				1.f, 0, 0, nullptr);

			m_commandList->OMSetRenderTargets(1, &MsaaRenderTargetView(), true, &MsaaDepthStencilView());
		}
		else {
			m_commandList->ClearRenderTargetView(HDRRendertargetView(), clearColor, 0, nullptr);
			m_commandList->ClearDepthStencilView(HDRDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
				1.f, 0, 0, nullptr);
			m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());
		}

		m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->RSSetScissorRects(1, &m_scissorRect);
		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());

		// View Proj Matrix Constant Buffer 
		m_commandList->SetGraphicsRootConstantBufferView(2, m_passConstantBuffer->GetGPUVirtualAddress());
		m_commandList->SetGraphicsRootConstantBufferView(3, m_ligthPassConstantBuffer->GetGPUVirtualAddress());

		// Texture SRV Heap 
		ID3D12DescriptorHeap* ppSrvHeaps[] = { m_textureHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppSrvHeaps), ppSrvHeaps);

		for (int i = 0; i < m_staticMeshes.size(); ++i) {
			CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_textureHeap->GetGPUDescriptorHandleForHeapStart());

			if (m_textureMap.count(m_staticMeshes[i]->GetTexturePath()) > 0) {
				handle.Offset(m_textureMap[m_staticMeshes[i]->GetTexturePath()], m_csuHeapSize);
			}
			else {
				handle.Offset(m_textureMap[L"default.png"], m_csuHeapSize);
			}
			m_commandList->SetGraphicsRootDescriptorTable(0, handle);

			m_staticMeshes[i]->Render(deltaTime, m_commandList, true);
		}
		// Exr Texture SRV Heap 
		/*ID3D12DescriptorHeap* ppSrvHeaps[] = { m_exrSrvHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppSrvHeaps), ppSrvHeaps);

		for (int i = 0; i < m_staticMeshes.size(); ++i) {
			CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_exrSrvHeap->GetGPUDescriptorHandleForHeapStart());
			handle.Offset(m_textureMap[L"test.exr"], m_csuHeapSize);

			m_commandList->SetGraphicsRootDescriptorTable(0, handle);

			m_staticMeshes[i]->Render(deltaTime, m_commandList, true);
		}*/

		for (int i = 0; i < m_fbxList.size(); ++i) {
			m_fbxList[i]->Render(deltaTime, m_commandList, true, m_textureHeap, m_textureMap, m_csuHeapSize);
		}

		if (msaaMode && !bRenderCubeMap) {
			ResolveSubresource(m_commandList, HDRRenderTargetBuffer(), MsaaRenderTargetBuffer());
		}
	}

}

void Renderer::D3D12App::RenderCubeMap(float& deltaTime)
{
	if (bRenderCubeMap)
	{
		auto& pso = cubePsoLists[(currRenderMode + "CubeMap")];
		if (currRenderMode == "Msaa") {
			msaaMode = true;
		}
		else {
			msaaMode = false;
		}
		ThrowIfFailed(m_commandAllocator->Reset());
		ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

		{
			if (msaaMode) {

				m_commandList->OMSetRenderTargets(1, &MsaaRenderTargetView(), true, &MsaaDepthStencilView());
			}
			else {

				m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());
			}

			m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());
			m_commandList->RSSetScissorRects(1, &m_scissorRect);
			m_commandList->RSSetViewports(1, &m_viewport);

			// View Proj Matrix Constant Buffer 
			m_commandList->SetGraphicsRootConstantBufferView(1, m_passConstantBuffer->GetGPUVirtualAddress());

			// CubeMap Heap 
			ID3D12DescriptorHeap* ppCubeHeaps[] = { m_cubeMapTextureHeap.Get() };
			m_commandList->SetDescriptorHeaps(_countof(ppCubeHeaps), ppCubeHeaps);

			m_commandList->SetGraphicsRootDescriptorTable(0, m_cubeMapTextureHeap->GetGPUDescriptorHandleForHeapStart());

			m_cubeMap->Render(deltaTime, m_commandList, false);

		}
	}
	//Render Font GUI
	{
		int time = (int)m_timer.GetElapsedTime();

		if (msaaMode) {
			RenderFonts(std::to_wstring(time), m_msaaResourceDescriptors, m_msaaSpriteBatch, m_msaaFont, m_commandList);
			ResolveSubresource(m_commandList, HDRRenderTargetBuffer(), MsaaRenderTargetBuffer());
		}
		else {
			RenderFonts(std::to_wstring(time), m_resourceDescriptors, m_spriteBatch, m_font, m_commandList);
		}
	}
	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* plists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

	FlushCommandQueue();
}

void Renderer::D3D12App::CreateCommandObjects()
{
	m_device->CreateCommandAllocator(m_commandType, IID_PPV_ARGS(&m_commandAllocator));
	m_device->CreateCommandAllocator(m_commandType, IID_PPV_ARGS(&m_guiCommandAllocator));

	D3D12_COMMAND_QUEUE_DESC cqDesc;
	ZeroMemory(&cqDesc, sizeof(cqDesc));
	cqDesc.Type = m_commandType;

	m_device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&m_commandQueue));

	ThrowIfFailed(m_device->CreateCommandList(
		0,
		m_commandType,
		m_commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&m_commandList)));

	m_commandList->Close();

	ThrowIfFailed(m_device->CreateCommandList(
		0,
		m_commandType,
		m_guiCommandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&m_guiCommandList)));

	m_guiCommandList->Close();
}

void Renderer::D3D12App::CreateDescriptorHeaps() {

	m_rtvHeapSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvHeapSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_csuHeapSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	Utility::CreateDescriptorHeap(m_device, m_swapChainRtvHeap, DescriptorType::RTV, m_swapChainCount, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, L"SwapChain RTV");
	Utility::CreateDescriptorHeap(m_device, m_dsvHeap, DescriptorType::DSV, 2);
	Utility::CreateDescriptorHeap(m_device, m_cbvHeap, DescriptorType::CBV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	Utility::CreateDescriptorHeap(m_device, m_msaaRtvHeap, DescriptorType::RTV, 1);
	Utility::CreateDescriptorHeap(m_device, m_hdrRtvHeap, DescriptorType::RTV, 1);
	Utility::CreateDescriptorHeap(m_device, m_hdrUavHeap, DescriptorType::UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, L"HDR UAV");
	Utility::CreateDescriptorHeap(m_device, m_hdrSrvHeap, DescriptorType::SRV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	Utility::CreateDescriptorHeap(m_device, m_geometryPassRtvHeap, DescriptorType::RTV, geometryPassRtvNum);
	Utility::CreateDescriptorHeap(m_device, m_geometryPassSrvHeap, DescriptorType::SRV, geometryPassRtvNum + 4, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	Utility::CreateDescriptorHeap(m_device, m_geometryPassMsaaRtvHeap, DescriptorType::RTV, geometryPassRtvNum);



}

void Renderer::D3D12App::FlushCommandQueue() {
	++m_currentFence;

	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));

	if (m_fence->GetCompletedValue() < m_currentFence) {
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_currentFence, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void Renderer::D3D12App::InitScene()
{
	using DirectX::SimpleMath::Vector3;

	/*std::shared_ptr<StaticMesh> sphere = std::make_shared<StaticMesh>();
	sphere->Initialize(GeometryGenerator::Sphere(0.8f, 100, 100, L"Bricks075A_4K-PNG0_Color.png"),
		m_device, m_commandList, Vector3(0.f, 0.f, 0.f), Material(), true);*/

		/*std::shared_ptr<StaticMesh> plane = std::make_shared<StaticMesh>();
		plane->Initialize(GeometryGenerator::Box(5, 1, 5, L"Bricks075A_4K-PNG_Color.png"), m_device, m_commandList, Vector3(0.f, -1.f, 0.f));*/

		//m_staticMeshes.push_back(sphere);
		//m_staticMeshes.push_back(plane);

		/*std::shared_ptr<StaticMesh> box = std::make_shared<StaticMesh>();
		box->Initialize(GeometryGenerator::Box(5, L"Bricks075A_4K-PNG_Color.png"), m_device, m_commandList, Vector3(0.f, -1.f, 0.f),
			Material(0.7f,1.f,1.f));

		m_staticMeshes.push_back(box);*/

	auto [box_destruction, box_destruction_animation] = GeometryGenerator::ReadFromFile_Pbr("box_destruction.fbx", true);
	std::shared_ptr<Animation::FBX> wallDistructionFbx = std::make_shared<Animation::FBX>();
	wallDistructionFbx->Initialize(box_destruction, box_destruction_animation, m_device, m_commandList, true, 1.f);
	m_fbxList.push_back(wallDistructionFbx);

	m_cubeMap = std::make_shared<StaticMesh>();
	m_cubeMap->Initialize(GeometryGenerator::SimpleCubeMapBox(100.f), m_device, m_commandList);

	m_screenMesh = std::make_shared<Core::StaticMesh>();
	m_screenMesh->Initialize(GeometryGenerator::Rectangle(2.f, L""), m_device, m_commandList);
}

void Renderer::D3D12App::RenderFonts(
	const std::wstring& output,
	std::shared_ptr<DirectX::DescriptorHeap>& resourceDescriptors,
	std::shared_ptr<DirectX::SpriteBatch>& spriteBatch,
	std::shared_ptr<DirectX::SpriteFont>& font,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList) {

	m_graphicsMemory.reset();

	m_graphicsMemory = std::make_unique<DirectX::GraphicsMemory>(m_device.Get());

	{
		ID3D12DescriptorHeap* heaps[] = { resourceDescriptors->Heap() };
		commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);
		spriteBatch->Begin(commandList.Get());

		spriteBatch->SetViewport(m_viewport);
		float margin = 5.f;
		m_fontPos = DirectX::SimpleMath::Vector2(m_screenWidth - margin, margin);
		//m_fontPos = DirectX::SimpleMath::Vector2(m_screenWidth/ 2.f , m_screenHeight / 2.f);
		DirectX::SimpleMath::Vector2 origin = font->MeasureString(output.c_str());
		origin.y = 0.f;

		DirectX::XMVECTORF32 color = DirectX::Colors::White;
		font->DrawString(spriteBatch.get(), output.c_str(),
			m_fontPos, color, 0.f, origin);

		spriteBatch->End();
	}
}

void Renderer::D3D12App::CreateConstantBuffer()
{
	std::vector<GlobalVertexConstantData> constantData = {
		*m_passConstantData
	};
	Utility::CreateUploadBuffer(constantData, m_passConstantBuffer, m_device);

	CD3DX12_RANGE range(0, 0);
	ThrowIfFailed(m_passConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCbvDataBegin)));
	memcpy(m_pCbvDataBegin, m_passConstantData, sizeof(GlobalVertexConstantData));

	std::vector<LightPassConstantData> psConstantData = {
		*m_ligthPassConstantData
	};
	Utility::CreateUploadBuffer(psConstantData, m_ligthPassConstantBuffer, m_device);

	ThrowIfFailed(m_ligthPassConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pLPCDataBegin)));
}


void Renderer::D3D12App::CreateTextures() {
	namespace fs = std::filesystem;

	fs::path path = fs::current_path();
	path.append(textureBasePath);

	int file_count = 0;
	if (fs::exists(path) && fs::is_directory(path)) {
		for (const auto& entry : fs::directory_iterator(path)) {
			if (fs::is_regular_file(entry.status())) {
				++file_count;
				//std::cout << entry.path().filename().string() << '\n';
			}
		}
	}
	else {
		std::cerr << "현재 경로가 존재하지 않거나 디렉토리가 아닙니다." << std::endl;
		return;
	}
	m_textureNum = file_count;
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NodeMask = 0;
	srvHeapDesc.NumDescriptors = file_count;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_textureHeap));

	m_textureResources.resize(file_count);
	m_uploadResources.resize(file_count);

	int mapIdx = 0;
	if (fs::exists(path) && fs::is_directory(path)) {
		for (const auto& entry : fs::directory_iterator(path)) {
			if (fs::is_regular_file(entry.status())) {
				std::wstring fileName = entry.path().filename().wstring();

				Utility::CreateTextureBuffer(textureBasePath + fileName, m_textureResources[mapIdx], m_textureHeap, m_device, m_commandQueue, m_commandList, mapIdx, m_csuHeapSize, nullptr);
				m_textureMap.emplace(fileName, mapIdx++);
			}
		}
	}
}

void Renderer::D3D12App::CreateCubeMapTextures() {
	namespace fs = std::filesystem;

	fs::path path = fs::current_path();
	path.append(cubeMapTextureBasePath);

	int file_count = 0;
	if (fs::exists(path) && fs::is_directory(path)) {
		for (const auto& entry : fs::directory_iterator(path)) {
			if (fs::is_regular_file(entry.status())) {
				++file_count;
				//std::cout << entry.path().filename().string() << '\n';
			}
		}
	}
	else {
		std::cerr << "현재 경로가 존재하지 않거나 디렉토리가 아닙니다." << std::endl;
		return;
	}
	m_textureNum = file_count;

	Utility::CreateDescriptorHeap(m_device, m_cubeMapTextureHeapNSV, DescriptorType::SRV, file_count, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	Utility::CreateDescriptorHeap(m_device, m_cubeMapTextureHeap, DescriptorType::SRV, file_count, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	bool IsCubeMap = true;
	m_cubeMaptextureResources.resize(file_count);
	m_cubeMaptextureUpload.resize(file_count);


	int mapIdx = 0;
	if (fs::exists(path) && fs::is_directory(path)) {
		for (const auto& entry : fs::directory_iterator(path)) {
			if (fs::is_regular_file(entry.status())) {
				std::wstring fileName = entry.path().filename().wstring();
				IsCubeMap = true;
				if (fileName.substr(fileName.size() - 8, 4) == L"Brdf") {
					IsCubeMap = false;
				}
				Utility::CreateTextureBuffer(cubeMapTextureBasePath + fileName, m_cubeMaptextureResources[mapIdx], m_cubeMapTextureHeapNSV, m_device, m_commandQueue, m_commandList, mapIdx, m_csuHeapSize, &IsCubeMap);
				//CreateCubeMapBuffer(cubeMapTextureBasePath + fileName, m_cubeMaptextureUpload[mapIdx], m_cubeMaptextureResources[mapIdx], mapIdx);
				m_cubeTextureMap.emplace(fileName, mapIdx++);


			}
		}
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE dest_gPassHeapHandle(m_geometryPassSrvHeap->GetCPUDescriptorHandleForHeapStart(), 4, m_csuHeapSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dest_CubeHandle(m_cubeMapTextureHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE src_CubeHandle(m_cubeMapTextureHeapNSV->GetCPUDescriptorHandleForHeapStart());

	UINT numDescriptors = m_cubeMapTextureHeapNSV->GetDesc().NumDescriptors;
	m_device->CopyDescriptorsSimple(numDescriptors, dest_gPassHeapHandle, src_CubeHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_device->CopyDescriptorsSimple(numDescriptors, dest_CubeHandle, src_CubeHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

}

void Renderer::D3D12App::CreateExrTexture() {

	namespace fs = std::filesystem;

	fs::path path = fs::current_path();
	path.append(exrTextureBasePath);

	int file_count = 0;
	if (fs::exists(path) && fs::is_directory(path)) {
		for (const auto& entry : fs::directory_iterator(path)) {
			if (fs::is_regular_file(entry.status())) {
				++file_count;
				//std::cout << entry.path().filename().string() << '\n';
			}
		}
	}
	else {
		std::cerr << "현재 경로가 존재하지 않거나 디렉토리가 아닙니다." << std::endl;
		return;
	}
	m_textureNum = file_count;
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NodeMask = 0;
	srvHeapDesc.NumDescriptors = file_count;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_exrSrvHeap));

	m_exrResources.resize(file_count);
	m_exrUploadResources.resize(file_count);

	int mapIdx = 0;
	if (fs::exists(path) && fs::is_directory(path)) {
		for (const auto& entry : fs::directory_iterator(path)) {
			if (fs::is_regular_file(entry.status())) {
				std::wstring fileName = entry.path().filename().wstring();
				CreateExrBuffer(exrTextureBasePath + fileName, m_exrUploadResources[mapIdx], m_exrResources[mapIdx], mapIdx);
				m_textureMap.emplace(fileName, mapIdx++);
			}
		}
	}
}

void Renderer::D3D12App::CreateExrBuffer(std::wstring& path, ComPtr<ID3D12Resource>& upload, ComPtr<ID3D12Resource>& texture, UINT offset)
{
	using namespace DirectX;

	DirectX::TexMetadata metaData;
	if (FAILED(DirectX::GetMetadataFromEXRFile(path.c_str(), metaData))) {
		std::wcout << "GetMetadataFromEXRFile() failed: " << path << std::endl;
	}

	DirectX::ScratchImage scratchImage;
	if (FAILED(DirectX::LoadFromEXRFile(path.c_str(), nullptr, scratchImage))) {
		std::wcout << "LoadFromEXRFile() failed: " << path << std::endl;
	}

	DirectX::ScratchImage mipChain;

	ThrowIfFailed(GenerateMipMaps(scratchImage.GetImages(),
		scratchImage.GetImageCount(),
		scratchImage.GetMetadata(),
		DirectX::TEX_FILTER_DEFAULT,
		0,
		mipChain));

	const Image* images = mipChain.GetImages();
	const TexMetadata& mipChainMetadata = mipChain.GetMetadata();

	/*std::vector<uint16_t> image;
	image.resize(images[0].slicePitch);
	memcpy(image.data(), images[0].pixels, image.size());*/

	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = (UINT16)mipChainMetadata.mipLevels;
	textureDesc.Format = mipChainMetadata.format;
	textureDesc.Width = (UINT64)mipChainMetadata.width;
	textureDesc.Height = (UINT)mipChainMetadata.height;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = (UINT16)mipChainMetadata.arraySize;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	std::wcout << path << " " << mipChainMetadata.mipLevels << std::endl;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(texture.ReleaseAndGetAddressOf())));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, (UINT)(mipChainMetadata.mipLevels * mipChainMetadata.arraySize));

	// Create the GPU upload buffer.
	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(upload.ReleaseAndGetAddressOf())));

	std::vector<D3D12_SUBRESOURCE_DATA> subData;
	for (size_t i = 0; i < mipChainMetadata.mipLevels; i++)
	{
		D3D12_SUBRESOURCE_DATA textureSubData;
		textureSubData.pData = images[i].pixels;
		textureSubData.RowPitch = images[i].rowPitch;
		textureSubData.SlicePitch = images[i].slicePitch;
		subData.push_back(textureSubData);
	}
	UpdateSubresources(m_commandList.Get(), texture.Get(), upload.Get(), 0, 0, (UINT)mipChainMetadata.mipLevels, subData.data());
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = mipChainMetadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = -1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_exrSrvHeap->GetCPUDescriptorHandleForHeapStart(), offset, m_csuHeapSize);
	m_device->CreateShaderResourceView(texture.Get(), &srvDesc, handle);
}

void Renderer::D3D12App::CreateCubeMapBuffer(std::wstring& path, ComPtr<ID3D12Resource>& upload, ComPtr<ID3D12Resource>& texture,
	UINT offset)
{
	using namespace DirectX;
	TexMetadata metadata;
	ScratchImage scratchImage;
	ThrowIfFailed(LoadFromDDSFile(path.c_str(), DDS_FLAGS_ALLOW_LARGE_FILES, &metadata, scratchImage));


	// 2. Mipmap 생성
	ScratchImage mipChain;
	ThrowIfFailed(GenerateMipMaps(scratchImage.GetImages(), scratchImage.GetImageCount(), scratchImage.GetMetadata(), TEX_FILTER_DEFAULT, 0, mipChain));


	const Image* images = mipChain.GetImages();
	const TexMetadata& mipChainMetadata = mipChain.GetMetadata();

	// 3. 리소스 디스크립터 설정
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = static_cast<UINT16>(mipChainMetadata.mipLevels);
	textureDesc.Format = mipChainMetadata.format;
	textureDesc.Width = static_cast<UINT>(mipChainMetadata.width);
	textureDesc.Height = static_cast<UINT>(mipChainMetadata.height);
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = static_cast<UINT16>(mipChainMetadata.arraySize);
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	// 4. 텍스처 리소스 생성
	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texture)
	));


	// 5. 업로드 힙 리소스 생성
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, static_cast<UINT>(mipChainMetadata.mipLevels) * (UINT)mipChainMetadata.arraySize);

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	ThrowIfFailed(m_device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&upload)
	));

	// 6. 텍스처 데이터를 업로드 힙에 복사
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = images[0].pixels;
	textureData.RowPitch = images[0].rowPitch;
	textureData.SlicePitch = images[0].slicePitch;

	UpdateSubresources(m_commandList.Get(), texture.Get(), upload.Get(), 0, 0, (UINT)mipChainMetadata.mipLevels, &textureData);

	// 7. 텍스처 상태 전환
	CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		texture.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	m_commandList->ResourceBarrier(1, &resourceBarrier);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_cubeMapTextureHeapNSV->GetCPUDescriptorHandleForHeapStart(), offset, m_csuHeapSize);
	m_device->CreateShaderResourceView(texture.Get(), &srvDesc, handle);

}

void Renderer::D3D12App::CreateFontFromFile(const std::wstring& fileName,
	std::shared_ptr<DirectX::SpriteFont>& font,
	std::shared_ptr<DirectX::SpriteBatch>& spriteBatch,
	std::shared_ptr<DirectX::DescriptorHeap>& resourceDescriptors,
	bool bUseMsaa,
	DXGI_FORMAT& rtFormat,
	DXGI_FORMAT& dsFormat)
{
	resourceDescriptors = std::make_shared<DirectX::DescriptorHeap>(m_device.Get(), Descriptors::Count);

	DirectX::RenderTargetState rtState(rtFormat, dsFormat);

	if (bUseMsaa)
	{
		rtState.sampleDesc.Count = (m_numQualityLevels > 0) ? m_sampleCount : 1;
		rtState.sampleDesc.Quality = (m_numQualityLevels > 0) ? m_numQualityLevels - 1 : 0;
	}

	DirectX::ResourceUploadBatch resourceUpload(m_device.Get());
	resourceUpload.Begin();

	// Load the font
	font = std::make_shared<DirectX::SpriteFont>(m_device.Get(), resourceUpload,
		fileName.c_str(),
		resourceDescriptors->GetCpuHandle(Descriptors::MyFont),
		resourceDescriptors->GetGpuHandle(Descriptors::MyFont));

	DirectX::SpriteBatchPipelineStateDescription pd(rtState);
	spriteBatch = std::make_shared<DirectX::SpriteBatch>(m_device.Get(), resourceUpload, pd);

	auto uploadResourcesFinished = resourceUpload.End(m_commandQueue.Get());
	uploadResourcesFinished.wait();
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::D3D12App::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_swapChainRtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_frameIndex,
		m_rtvHeapSize
	);
}

ID3D12Resource* Renderer::D3D12App::CurrentBackBuffer() const
{
	return m_renderTargets[m_frameIndex].Get();
}

ID3D12Resource* Renderer::D3D12App::MsaaRenderTargetBuffer() const
{
	return m_msaaRenderTarget.Get();
}

ID3D12Resource* Renderer::D3D12App::HDRRenderTargetBuffer() const
{
	return m_hdrRenderTarget.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::D3D12App::MsaaDepthStencilView() const
{
	return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::D3D12App::HDRDepthStencilView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
		1,
		m_dsvHeapSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::D3D12App::HDRUnorderedAccesslView() const
{
	return m_hdrRtvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::D3D12App::MsaaRenderTargetView() const
{
	return m_msaaRtvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::D3D12App::HDRRendertargetView() const
{
	return m_hdrRtvHeap->GetCPUDescriptorHandleForHeapStart();
}

void  Renderer::D3D12App::ResolveSubresource(ComPtr<ID3D12GraphicsCommandList>& commandList, ID3D12Resource* dest, ID3D12Resource* src,
	D3D12_RESOURCE_STATES destState,
	D3D12_RESOURCE_STATES srcState,
	DXGI_FORMAT format
)
{
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		dest,
		destState,
		D3D12_RESOURCE_STATE_RESOLVE_DEST));

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		src,
		srcState,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE));

	commandList->ResolveSubresource(dest, 0, src, 0, format);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		src,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		srcState));

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		dest,
		D3D12_RESOURCE_STATE_RESOLVE_DEST,
		destState));
}

void  Renderer::D3D12App::CopyResource(ComPtr<ID3D12GraphicsCommandList>& commandList, ID3D12Resource* dest, ID3D12Resource* src,
	D3D12_RESOURCE_STATES destState,
	D3D12_RESOURCE_STATES srcState
)
{
	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(0, 255, 0), copyResourceEvent);

	D3D12_RESOURCE_BARRIER startBarrierList[2] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			dest,
			destState,
			D3D12_RESOURCE_STATE_COPY_DEST),

		CD3DX12_RESOURCE_BARRIER::Transition(
			src,
			srcState,
			D3D12_RESOURCE_STATE_COPY_SOURCE)
	};
	D3D12_RESOURCE_BARRIER endBarrierList[2] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			src,
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			srcState),

		CD3DX12_RESOURCE_BARRIER::Transition(
			dest,
			D3D12_RESOURCE_STATE_COPY_DEST,
			destState)
	};
	commandList->ResourceBarrier(2, startBarrierList);

	commandList->CopyResource(dest, src);

	commandList->ResourceBarrier(2, endBarrierList);


	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());

}

void Renderer::D3D12App::CopyResourceToSwapChain(float& deltaTime)
{
	auto& pso = utilityPsoLists["Copy"];
	{
		ThrowIfFailed(m_commandAllocator->Reset());
		ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

		PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(0, 255, 0), copyResourceToSwapChainEvent);
		m_commandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			));
		FLOAT clearColor[4] = { 0.f,0.f,0.f,0.f };
		if (msaaMode)
		{
			ResolveSubresource(m_commandList, HDRRenderTargetBuffer(), MsaaRenderTargetBuffer(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				DXGI_FORMAT_R16G16B16A16_FLOAT);
		}

		m_commandList->ClearRenderTargetView(CurrentBackBufferView(), clearColor, 0, nullptr);
		m_commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), false, nullptr);
		m_commandList->RSSetScissorRects(1, &m_scissorRect);
		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());
		m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D12DescriptorHeap* pHeaps[] = { m_hdrSrvHeap.Get() };
		m_commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(pHeaps)), pHeaps);
		m_commandList->SetGraphicsRootDescriptorTable(0, m_hdrSrvHeap->GetGPUDescriptorHandleForHeapStart());

		/*ID3D12DescriptorHeap* pHeaps[] = { m_exrSrvHeap.Get() };
		m_commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(pHeaps)), pHeaps);
		m_commandList->SetGraphicsRootDescriptorTable(0, m_exrSrvHeap->GetGPUDescriptorHandleForHeapStart());*/

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
void Renderer::D3D12App::CreateDepthBuffer(ComPtr<ID3D12Resource>& buffer,
	D3D12_CPU_DESCRIPTOR_HANDLE& handle, bool bUseMsaa)
{
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_screenWidth;
	depthStencilDesc.Height = m_screenHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	if (bUseMsaa) {
		depthStencilDesc.SampleDesc.Count = (m_numQualityLevels > 0) ? m_sampleCount : 1;
		depthStencilDesc.SampleDesc.Quality = (m_numQualityLevels > 0) ? m_numQualityLevels - 1 : 0;
	}
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = m_depthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(buffer.ReleaseAndGetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	if (bUseMsaa)
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
	else
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	dsvDesc.Format = m_depthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;

	m_device->CreateDepthStencilView(buffer.Get(), &dsvDesc, handle);
}

void Renderer::D3D12App::CreateResourceBuffer(ComPtr<ID3D12Resource>& buffer, DXGI_FORMAT format,
	bool bUseMsaa, D3D12_RESOURCE_FLAGS flag, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, bool bUseClear)
{
	D3D12_RESOURCE_DESC rtDesc;
	rtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rtDesc.Alignment = 0;
	rtDesc.Width = m_screenWidth;
	rtDesc.Height = m_screenHeight;
	rtDesc.DepthOrArraySize = 1;
	rtDesc.MipLevels = 1;
	rtDesc.Format = format;
	if (bUseMsaa) {
		rtDesc.SampleDesc.Count = (m_numQualityLevels > 0) ? m_sampleCount : 1;
		rtDesc.SampleDesc.Quality = (m_numQualityLevels > 0) ? m_numQualityLevels - 1 : 0;
	}
	else {
		rtDesc.SampleDesc.Count = 1;
		rtDesc.SampleDesc.Quality = 0;
	}
	rtDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rtDesc.Flags = flag;


	if (bUseClear)
	{
		FLOAT color[4] = { 0.f, 0.f, 0.f, 0.f };
		D3D12_CLEAR_VALUE optClearRtv;
		optClearRtv.Format = format;
		optClearRtv.Color[0] = 0.f;
		optClearRtv.Color[1] = 0.f;
		optClearRtv.Color[2] = 0.f;
		optClearRtv.Color[3] = 0.f;
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(heapType),
			D3D12_HEAP_FLAG_NONE,
			&rtDesc,
			resourceState,
			&optClearRtv,
			IID_PPV_ARGS(buffer.ReleaseAndGetAddressOf())));
	}
	else 
	{
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(heapType),
			D3D12_HEAP_FLAG_NONE,
			&rtDesc,
			resourceState,
			nullptr,
			IID_PPV_ARGS(buffer.ReleaseAndGetAddressOf())));
	}
}


void Renderer::D3D12App::CreateResourceView(ComPtr<ID3D12Resource>& buffer,
	DXGI_FORMAT format,
	bool bUseMsaa,
	D3D12_CPU_DESCRIPTOR_HANDLE& handle,
	ComPtr<ID3D12Device5>& deivce,
	const Renderer::DescriptorType& type)
{
	if (type == DescriptorType::RTV) {
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));
		if (bUseMsaa) {
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
		}
		else {
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		}
		rtvDesc.Format = format;
		rtvDesc.Texture2D.MipSlice = 0;

		m_device->CreateRenderTargetView(buffer.Get(), &rtvDesc, handle);
	}
	else if (type == DescriptorType::UAV) {
		if (bUseMsaa) {
			std::cout << "UAV는 MSAA로 만들 수 없습니다" << std::endl;
			return;
		}
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(uavDesc));
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Format = format;
		uavDesc.Texture2D.MipSlice = 0;
		uavDesc.Format = format;
		uavDesc.Texture2D.MipSlice = 0;

		m_device->CreateUnorderedAccessView(buffer.Get(), nullptr, &uavDesc, handle);
	}
	else if (type == DescriptorType::SRV) {

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if (bUseMsaa) {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
		}
		else {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		}
		srvDesc.Format = format;
		srvDesc.Texture2D.MipLevels = 1;
		m_device->CreateShaderResourceView(buffer.Get(), &srvDesc, handle);
	}
}
void Renderer::D3D12App::CaptureBufferToPNG() {
	
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	D3D12_RESOURCE_DESC desc = HDRRenderTargetBuffer()->GetDesc();
	UINT64 requiredSize = 0;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	m_device->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, nullptr, nullptr, &requiredSize);
	
	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			HDRRenderTargetBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_COPY_SOURCE));

	CD3DX12_TEXTURE_COPY_LOCATION dst(imageBuffer.Get(), footprint);
	CD3DX12_TEXTURE_COPY_LOCATION src(HDRRenderTargetBuffer(), 0);
	m_commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			HDRRenderTargetBuffer(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
			));

	m_commandList->Close();
	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(lists), lists);
	FlushCommandQueue();

	D3D12_RANGE range(0, 0);
	void* pData;
	UINT width = (UINT)HDRRenderTargetBuffer()->GetDesc().Width;
	UINT height = HDRRenderTargetBuffer()->GetDesc().Height;
	UINT channels = 4;

	UINT imageSize = width * height * channels;
	imagef16.resize(imageSize);
	imageUnorm.resize(imageSize);
	UINT dataSize = width * height * channels * sizeof(uint16_t);

	imageBuffer->Map(0, &range, reinterpret_cast<void**>(&pData));
	memcpy(imagef16.data(), pData, dataSize);
	imageBuffer->Unmap(0, nullptr);

	for (size_t i = 0; i < imageSize; i++)
	{
		imageUnorm[i] = (uint8_t)(fp16_ieee_to_fp32_value(imagef16[i]) * 255.f);
	}
	time_t timer = time(NULL);
	tm* t;
	t = localtime(&timer);
	std::stringstream ss;
	ss << "Results/" << m_appName << "/" << t->tm_year - 100;
	if (t->tm_mon < 9) {
		ss << '0' << t->tm_mon + 1;
	}
	else{
		ss << t->tm_mon + 1;
	}
	if (t->tm_mday < 9)	{
		ss << '0' << t->tm_mday;
	}
	else {
		ss << t->tm_mday;
	}
	ss << '-' << t->tm_hour << t->tm_min << ".png";
	stbi_write_png(ss.str().c_str(), width, height, 4, imageUnorm.data(), width * channels);
}

void Renderer::D3D12App::CreateSamplers() {
	/*D3D12_SAMPLER_DESC clampSampler = {};
	clampSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	clampSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	clampSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	clampSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	clampSampler.MipLODBias = 0;
	clampSampler.MaxAnisotropy = 1;
	clampSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	clampSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	clampSampler.MinLOD = 0.0f;
	clampSampler.MaxLOD = D3D12_FLOAT32_MAX;
	clampSampler.ShaderRegister = 0;
	clampSampler.RegisterSpace = 0;
	clampSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	m_device->CreateSampler()*/
}
D3D12_CPU_DESCRIPTOR_HANDLE Renderer::D3D12App::GeometryPassRTV() const
{
	return m_geometryPassRtvHeap->GetCPUDescriptorHandleForHeapStart();
}
D3D12_CPU_DESCRIPTOR_HANDLE Renderer::D3D12App::GeometryPassMsaaRTV() const
{
	return m_geometryPassMsaaRtvHeap->GetCPUDescriptorHandleForHeapStart();
}