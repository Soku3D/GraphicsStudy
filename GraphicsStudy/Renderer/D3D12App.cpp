#include "D3D12App.h"
#include "GeometryGenerator.h"
#include "Renderer.h"

using namespace Core;

Renderer::D3D12App::D3D12App(const int& width, const int& height)
	:SimpleApp(width, height),
	bUseWarpAdapter(false),
	m_minimumFeatureLevel(D3D_FEATURE_LEVEL_11_0),
	m_viewport(D3D12_VIEWPORT()),
	m_scissorRect(D3D12_RECT())
{
	m_passConstantData = new GlobalVertexConstantData();
	m_psConstantData = new PSConstantData();
	textureBasePath = L"Textures/";
}

Renderer::D3D12App::~D3D12App()
{
	if (m_device != nullptr)
		FlushCommandQueue();
	for (int i = 0; i < m_swapChainCount; i++)
	{
		m_renderTargets[i].Reset();
	}
	m_depthStencilBuffer.Reset();

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

	CreateTextures();
	CreateVertexAndIndexBuffer();

	CreateFontFromFile(L"Fonts/default.spritefont", m_font, m_spriteBatch, m_resourceDescriptors);
	CreateFontFromFile(L"Fonts/cyberpunk.spritefont", m_guiFont, m_guiSpriteBatch, m_guiResourceDescriptors);

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
	float fontSize = 18.0f;
	// 폰트 로드
	io.Fonts->AddFontFromFileTTF(fontPath, fontSize);
	
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_fontHeap));

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(m_mainWnd);
	/*ImGui_ImplDX12_Init(m_device.Get(), m_swapChainCount, m_backbufferFormat,
		m_guiResourceDescriptors->Heap(),
		m_guiResourceDescriptors->GetFirstCpuHandle(),
		m_guiResourceDescriptors->GetFirstGpuHandle());*/
	ImGui_ImplDX12_Init(m_device.Get(), m_swapChainCount, m_backbufferFormat,
		m_fontHeap.Get(),
		m_fontHeap->GetCPUDescriptorHandleForHeapStart(),
		m_fontHeap->GetGPUDescriptorHandleForHeapStart());

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
	msaaData.Format = m_backbufferFormat;
	msaaData.NumQualityLevels = m_numQualityLevels;
	msaaData.SampleCount = m_sampleCount;

	ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaData, sizeof(msaaData)));
	//std::cout << "Msaa Num Quality Level : " << msaaData.NumQualityLevels << std::endl;
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
	m_depthStencilBuffer.Reset();

	ThrowIfFailed(m_swapChain->ResizeBuffers(m_swapChainCount,
		m_screenWidth,
		m_screenHeight,
		DXGI_FORMAT_UNKNOWN,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < m_swapChainCount; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_renderTargets[i].ReleaseAndGetAddressOf()));
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_rtvHeapSize);
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_screenWidth;
	depthStencilDesc.Height = m_screenHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	//depthStencilDesc.SampleDesc.Count = 1;
	//depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.SampleDesc.Count = (m_numQualityLevels > 0) ? m_sampleCount : 1;
	depthStencilDesc.SampleDesc.Quality = (m_numQualityLevels > 0) ? m_numQualityLevels - 1 : 0;
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
		IID_PPV_ARGS(m_depthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
	dsvDesc.Format = m_depthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;

	m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, MsaaDepthStencilView());

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	D3D12_RESOURCE_DESC rtDesc;
	rtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rtDesc.Alignment = 0;
	rtDesc.Width = m_screenWidth;
	rtDesc.Height = m_screenHeight;
	rtDesc.DepthOrArraySize = 1;
	rtDesc.MipLevels = 1;
	rtDesc.Format = m_backbufferFormat;
	rtDesc.SampleDesc.Count = (m_numQualityLevels > 0) ? m_sampleCount : 1;
	rtDesc.SampleDesc.Quality = (m_numQualityLevels > 0) ? m_numQualityLevels - 1 : 0;
	rtDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
	rtvDesc.Format = m_backbufferFormat;
	rtvDesc.Texture2D.MipSlice = 0;

	FLOAT color[4] = { 1.f, 1.f, 1.f, 1.f };
	D3D12_CLEAR_VALUE optClearRtv;
	optClearRtv.Format = m_backbufferFormat;
	optClearRtv.Color[0] = 1.f;
	optClearRtv.Color[1] = 1.f;
	optClearRtv.Color[2] = 1.f;
	optClearRtv.Color[3] = 1.f;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&rtDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&optClearRtv,
		IID_PPV_ARGS(m_msaaRenderTarget.GetAddressOf())));

	m_device->CreateRenderTargetView(m_msaaRenderTarget.Get(), &rtvDesc, m_msaaRtvHeap->GetCPUDescriptorHandleForHeapStart());

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();

	m_viewport = { 0.f ,0.f,(FLOAT)m_screenWidth ,(FLOAT)m_screenHeight, 0.f, 1.f };
	m_scissorRect = { 0 ,0,static_cast<LONG>(m_screenWidth), static_cast<LONG>(m_screenHeight) };
}

void Renderer::D3D12App::Update( float& deltaTime )
{
	m_inputHandler->ExicuteCommand(m_camera.get(), deltaTime, bIsFPSMode);

	m_passConstantData->ViewMat = m_camera->GetViewMatrix();
	m_passConstantData->ProjMat = m_camera->GetProjMatrix();
	
	m_passConstantData->ViewMat = m_passConstantData->ViewMat.Transpose();
	m_passConstantData->ProjMat = m_passConstantData->ProjMat.Transpose();

	memcpy(m_pCbvDataBegin, m_passConstantData, sizeof(GlobalVertexConstantData));

	for (auto& mesh : m_staticMeshes) {
		//mesh->Update(deltaTime);
	}
}

void Renderer::D3D12App::UpdateGUI(float& deltaTime)
{
	if (ImGui::BeginCombo("Mode", currRenderMode.c_str())) // The second parameter is the label previewed before opening the combo.
	{
		for (int n = 0; n < psoListNames.size(); n++)
		{
			bool is_selected = (currRenderMode == psoListNames[n]); // You can store your selection however you want, outside or inside your objects
			if (ImGui::Selectable(psoListNames[n].c_str(), is_selected)) {
				currRenderMode = psoListNames[n];
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
}

void Renderer::D3D12App::Render(float& deltaTime)
{
	auto& pso = psoList[currRenderMode];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	
	{
		FLOAT clearColor[4] = { 1.f,1.f,1.f,1.f };
		m_commandList->ClearRenderTargetView(GetMSAARtV(), clearColor, 0, nullptr);
		m_commandList->ClearDepthStencilView(m_msaaDsvHeap->GetCPUDescriptorHandleForHeapStart(),
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			1.f, 0, 0, NULL);
		
		m_commandList->OMSetRenderTargets(1, &GetMSAARtV(), true, &MsaaDepthStencilView());

		m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_commandList->RSSetScissorRects(1, &m_scissorRect);
		m_commandList->RSSetViewports(1, &m_viewport);
		
		m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());

		// View Proj Matrix Constant Buffer 
		m_commandList->SetGraphicsRootConstantBufferView(2, m_passConstantBuffer->GetGPUVirtualAddress());

		// Texture SRV Heap 
		ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		for (int i = 0; i < m_staticMeshes.size(); ++i) {
			CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_srvHeap->GetGPUDescriptorHandleForHeapStart());

			if (m_textureMap.count(m_staticMeshes[i]->GetTexturePath()) > 0) {
				handle.Offset(m_textureMap[m_staticMeshes[i]->GetTexturePath()], m_csuHeapSize);
			}
			else {
				handle.Offset(m_textureMap[L"default.png"], m_csuHeapSize);
			}
			m_commandList->SetGraphicsRootDescriptorTable(0, handle);

			m_staticMeshes[i]->Render(deltaTime, m_commandList);
		}

	}
	//int time = (int)m_timer.GetElapsedTime();
	//RenderFonts(std::to_wstring(time), m_resourceDescriptors, m_commandList);

	ResolveSubresource(m_commandList);
	
	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

	//ThrowIfFailed(m_swapChain->Present(0, 0));
	//m_frameIndex = (m_frameIndex + 1) % m_swapChainCount;
	
	FlushCommandQueue();
}

void Renderer::D3D12App::RenderGUI(float& deltaTime)
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
		
		m_guiCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			));	
		
		m_guiCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), false, nullptr);
		ID3D12DescriptorHeap* pHeaps[] = { m_fontHeap.Get()};
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

	ThrowIfFailed(m_swapChain->Present(0, 0));
	m_frameIndex = (m_frameIndex + 1) % m_swapChainCount;

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

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	ZeroMemory(&rtvHeapDesc, sizeof(rtvHeapDesc));
	// Msaa용 Heap
	rtvHeapDesc.NumDescriptors = m_swapChainCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_device->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(m_msaaDsvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_device->CreateDescriptorHeap(
		&cbvHeapDesc, IID_PPV_ARGS(m_cbvHeap.GetAddressOf())));


	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc2;
	ZeroMemory(&rtvHeapDesc2, sizeof(rtvHeapDesc2));
	rtvHeapDesc2.NumDescriptors = 1;
	rtvHeapDesc2.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc2.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(
		&rtvHeapDesc2, IID_PPV_ARGS(m_msaaRtvHeap.GetAddressOf())));

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

void Renderer::D3D12App::CreateVertexAndIndexBuffer()
{
	using DirectX::SimpleMath::Vector3;
	
	std::shared_ptr<StaticMesh> sphere = std::make_shared<StaticMesh>();
	sphere->Initialize(GeometryGenerator::Sphere(0.8f, 100, 100, L""), m_device, m_commandList, Vector3(-1.f, 0.f, 0.f));

	std::shared_ptr<StaticMesh> plane = std::make_shared<StaticMesh>();
	plane->Initialize(GeometryGenerator::Box(4,1,4,L"Metal.png"), m_device, m_commandList, Vector3(0.f, -1.f, 0.f));
	
	m_staticMeshes.push_back(sphere);
	m_staticMeshes.push_back(plane);
	auto meshes = GeometryGenerator::ReadFromFile("zelda.fbx");
	for (auto& mesh : meshes) {
		std::shared_ptr<StaticMesh> newMesh = std::make_shared<StaticMesh>();
		newMesh->Initialize(mesh, m_device, m_commandList);
		m_staticMeshes.push_back(newMesh);
	}

}

void Renderer::D3D12App::RenderFonts(const std::wstring& output, std::shared_ptr<DirectX::DescriptorHeap>& resourceDescriptors,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList) {
	
	m_graphicsMemory.reset();

	m_graphicsMemory = std::make_unique<DirectX::GraphicsMemory>(m_device.Get());
	
	{
		ID3D12DescriptorHeap* heaps[] = { resourceDescriptors->Heap() };
		commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);
		m_spriteBatch->Begin(commandList.Get());

		m_spriteBatch->SetViewport(m_viewport);
		float margin = 5.f;
		m_fontPos = DirectX::SimpleMath::Vector2(m_screenWidth - margin, margin);
		//m_fontPos = DirectX::SimpleMath::Vector2(m_screenWidth/ 2.f , m_screenHeight / 2.f);
		DirectX::SimpleMath::Vector2 origin = m_font->MeasureString(output.c_str());
		origin.y = 0.f;

		DirectX::XMVECTORF32 color = DirectX::Colors::Black;
		m_font->DrawString(m_spriteBatch.get(), output.c_str(),
			m_fontPos, color, 0.f, origin);

		m_spriteBatch->End();		
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

	std::vector<PSConstantData> psConstantData = {
		*m_psConstantData
	};
	Utility::CreateUploadBuffer(psConstantData, m_psConstantBuffer, m_device);

	ThrowIfFailed(m_psConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pPSDataBegin)));
	memcpy(m_pPSDataBegin, m_psConstantData, sizeof(PSConstantData));
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
	srvHeapDesc.NumDescriptors = file_count	;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));

	m_textureResources.resize(file_count);
	int mapIdx = 0;
	if (fs::exists(path) && fs::is_directory(path)) {
		for (const auto& entry : fs::directory_iterator(path)) {
			std::wstring fileName = entry.path().filename().wstring();
			Utility::CreateTextureBuffer(textureBasePath + fileName, m_textureResources[mapIdx], m_srvHeap, m_device, m_commandQueue, mapIdx, m_csuHeapSize);
			m_textureMap.emplace(fileName, mapIdx++);
		}
	}
}

void Renderer::D3D12App::CreateFontFromFile(const std::wstring& fileName,
	std::shared_ptr<DirectX::SpriteFont> & font, std::shared_ptr<DirectX::SpriteBatch>& spriteBatch,
	std::shared_ptr<DirectX::DescriptorHeap>&  resourceDescriptors) 
{
	resourceDescriptors = std::make_shared<DirectX::DescriptorHeap>(m_device.Get(),
		Descriptors::Count);

	DirectX::RenderTargetState rtState(m_backbufferFormat,
		m_depthStencilFormat);
	rtState.sampleDesc.Count = (m_numQualityLevels > 0) ? m_sampleCount : 1;
	rtState.sampleDesc.Quality = (m_numQualityLevels > 0) ? m_numQualityLevels - 1 : 0;
	
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
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_frameIndex,
		m_rtvHeapSize
	);
}

ID3D12Resource* Renderer::D3D12App::CurrentBackBuffer() const
{
	return m_renderTargets[m_frameIndex].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::D3D12App::MsaaDepthStencilView() const
{
	return m_msaaDsvHeap->GetCPUDescriptorHandleForHeapStart();
}



D3D12_CPU_DESCRIPTOR_HANDLE Renderer::D3D12App::GetMSAARtV() const
{
	return m_msaaRtvHeap->GetCPUDescriptorHandleForHeapStart();
}
void  Renderer::D3D12App::ResolveSubresource(ComPtr<ID3D12GraphicsCommandList>& commandList) {
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RESOLVE_DEST));

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_msaaRenderTarget.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE));

	commandList->ResolveSubresource(CurrentBackBuffer(), 0, m_msaaRenderTarget.Get(), 0, m_backbufferFormat);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_msaaRenderTarget.Get(),
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET));

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RESOLVE_DEST,
		D3D12_RESOURCE_STATE_PRESENT));
}