#include "D3D12App.h"

Renderer::D3D12App::D3D12App(const int& width, const int& height)
	:SimpleApp(width, height),
	bUseWarpAdapter(false),
	m_minimumFeatureLevel(D3D_FEATURE_LEVEL_11_0),
	m_viewport(D3D12_VIEWPORT())
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
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
	CreateVertexAndIndexBuffer();
	CreateConstantBuffer();
	CreateRootSignature();
	CreatePSO();

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

	FlushCommandQueue();

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
	std::cout << "Msaa Num Quality Level : " << m_numQualityLevels << std::endl;

	m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));

	CreateCommandObjects();
	CreateDescriptorHeaps();

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 scDesc;
	ZeroMemory(&scDesc, sizeof(scDesc));
	scDesc.Width = m_screenWidth;
	scDesc.Height = m_screenHeight;
	scDesc.Format = m_backbufferFormat;
	scDesc.SampleDesc.Count = (m_numQualityLevels > 0) ? m_sampleCount : 1;
	scDesc.SampleDesc.Quality = (m_numQualityLevels > 0) ? m_numQualityLevels - 1 : 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scDesc.BufferCount = m_swapChainCount;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC scfDesc;
	scfDesc.RefreshRate.Denominator = 60;
	scfDesc.RefreshRate.Numerator = 1;
	// 전체화면 창모드?
	scfDesc.Windowed = true;

	ComPtr<IDXGISwapChain1> swapChain;

	ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		m_mainWnd,
		&scDesc,
		nullptr,
		nullptr,
		&swapChain
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

	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
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
		IID_PPV_ARGS(m_depthStencilBuffer.ReleaseAndGetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = m_depthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;

	m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();

	m_viewport = { 0.f,0.f,(FLOAT)m_screenWidth,(FLOAT)m_screenHeight, 0.f, 1.f };
	m_scissorRect = { 0,0,static_cast<LONG>(m_screenWidth), static_cast<LONG>(m_screenHeight) };

}

void Renderer::D3D12App::Update(const double& deltaTime)
{
}

void Renderer::D3D12App::Render(const double& deltaTime)
{
	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pso.Get()));
	{
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

		FLOAT clearColor[4] = { 1.f,1.f,1.f,1.f };
		m_commandList->ClearRenderTargetView(CurrentBackBufferView(), clearColor, 0, nullptr);
		m_commandList->ClearDepthStencilView(
			m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			1.f,
			0,
			0,
			nullptr);

		m_commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

		m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		m_commandList->IASetIndexBuffer(&m_indexBufferView);
		m_commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_commandList->RSSetScissorRects(1, &m_scissorRect);
		m_commandList->RSSetViewports(1, &m_viewport);

		m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
		ID3D12DescriptorHeap* descriptorHeaps[] = {
			m_cbvHeap.Get()
		};
		m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());

		m_commandList->DrawIndexedInstanced(3, 1, 0, 0, 0);
		
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));
	}

	m_commandList->Close();
	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

	ThrowIfFailed(m_swapChain->Present(0, 0));
	m_frameIndex = (m_frameIndex + 1) % m_swapChainCount;

	FlushCommandQueue();
}

void Renderer::D3D12App::CreateCommandObjects()
{
	m_device->CreateCommandAllocator(m_commandType, IID_PPV_ARGS(&m_commandAllocator));

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
}

void Renderer::D3D12App::CreateDescriptorHeaps() {

	m_rtvHeapSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvHeapSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_csuHeapSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	ZeroMemory(&rtvHeapDesc, sizeof(rtvHeapDesc));
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
		&dsvHeapDesc, IID_PPV_ARGS(m_dsvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_device->CreateDescriptorHeap(
		&cbvHeapDesc, IID_PPV_ARGS(m_cbvHeap.GetAddressOf())));
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

	std::vector<SimpleVertex> triangleVertices = {
		{Vector3(-1.f,-1.f,0.f)},
		{Vector3(0.f,1.f,0.f)},
		{Vector3(1.f,-1.f,0.f)},
	};
	std::vector<uint16_t> indices = {
		0,1,2
	};
	Utility::CreateBuffer(triangleVertices, m_vertexUpload, m_vertexGpu, m_device, m_commandList);
	Utility::CreateBuffer(indices, m_indexUpload, m_indexGpu, m_device, m_commandList);

	m_vertexBufferView.BufferLocation = m_vertexGpu->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = (UINT)(triangleVertices.size() * sizeof(SimpleVertex));
	m_vertexBufferView.StrideInBytes = sizeof(SimpleVertex);

	m_indexBufferView.BufferLocation = m_indexGpu->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	m_indexBufferView.SizeInBytes = UINT(sizeof(uint16_t) * indices.size());
}

void Renderer::D3D12App::CreateConstantBuffer()
{
	// 1. Create buffer
	// 2. Create bufferView
	std::vector<ConstantBuffer> constantData = {
		m_constantData
	};
	Utility::CreateUploadBuffer(constantData, m_constantUploadBuffer, m_device);

	CD3DX12_RANGE range(0, 0);
	ThrowIfFailed(m_constantUploadBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCbvDataBegin)));
	memcpy(m_pCbvDataBegin, &m_constantData, sizeof(ConstantBuffer));
	m_constantUploadBuffer->Unmap(0, nullptr);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = m_constantUploadBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = sizeof(ConstantBuffer);

	m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void Renderer::D3D12App::CreateRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE1 tables[1];
	tables[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_ROOT_PARAMETER1 rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, tables);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), error.GetAddressOf()));

	m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
}

void Renderer::D3D12App::CreatePSO()
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> elements = {
		{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
	m_simpleVertexInputLayout.NumElements = (UINT)elements.size();
	m_simpleVertexInputLayout.pInputElementDescs = elements.data();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS =
	{
		g_pTestVS,
		sizeof(g_pTestVS)
	};
	psoDesc.PS =
	{
		g_pTestPS,
		sizeof(g_pTestPS)
	};
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.InputLayout = m_simpleVertexInputLayout;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_backbufferFormat;
	psoDesc.DSVFormat = m_depthStencilFormat;
	psoDesc.SampleDesc.Count = (m_numQualityLevels > 0) ? m_sampleCount : 1;
	psoDesc.SampleDesc.Quality = (m_numQualityLevels > 0) ? m_numQualityLevels -1 : 0;
	
	ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));
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

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::D3D12App::DepthStencilView() const
{
	return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}
