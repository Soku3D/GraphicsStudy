#include "D3D11App.h"

Renderer::D3D11App::D3D11App(const int& width, const int& height)
	:SimpleApp(width, height)
{
}

Renderer::D3D11App::~D3D11App()
{
}

bool Renderer::D3D11App::Initialize()
{
	using DirectX::SimpleMath::Vector3;
	if(!SimpleApp::Initialize())
		return false;

	std::vector<SimpleVertex> vertices = {
		{Vector3(-1.f,-1.f,0.f)},
		{Vector3(0.f,1.f,0.f)},
		{Vector3(1.f,-1.f,0.f)}
	};
	std::vector<uint32_t> indices = {
		0,1,2
	};
	Utility::CreateVertexBuffer(vertices, m_vertexBuffer, m_device);
	Utility::CreateIndexBuffer(indices, m_indexBuffer, m_device);

	std::vector<D3D11_INPUT_ELEMENT_DESC> elements = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	Utility::CreateVertexShaderAndInputLayout(L"TestVS.hlsl", m_vertexShader, elements, m_inputLayout, m_device);
	Utility::CreatePixelShader(L"TestPS.hlsl", m_pixelShader, m_device);
	return true;
}

bool Renderer::D3D11App::InitDirectX()
{
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

	UINT debugFlags = 0;
	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_11_0,
	};
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
#if defined(DEBUG) || defined(_DEBUG)
	debugFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	
	ThrowIfFailed(D3D11CreateDevice(
		nullptr,
		driverType,
		NULL,
		debugFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		device.GetAddressOf(),
		&featureLevel,
		context.GetAddressOf()));

	device->CheckMultisampleQualityLevels(m_backBufferFormat, m_sampleCount,
		&m_msaaQuality);

	ThrowIfFailed(device.As(&m_device));
	ThrowIfFailed(context.As(&m_context));

	DXGI_SWAP_CHAIN_DESC scDesc;
	ZeroMemory(&scDesc, sizeof(scDesc));
	scDesc.BufferDesc.Width = m_screenWidth;
	scDesc.BufferDesc.Height = m_screenHeight;
	scDesc.BufferDesc.Format = m_backBufferFormat;
	scDesc.BufferDesc.RefreshRate.Numerator = 60;
	scDesc.BufferDesc.RefreshRate.Denominator = 1;
	scDesc.SampleDesc.Count = 1;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 2;
	scDesc.OutputWindow = m_mainWnd;
	scDesc.Windowed = true;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
		nullptr,
		driverType,
		NULL,
		debugFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&scDesc,
		m_swapChain.GetAddressOf(),
		m_device.ReleaseAndGetAddressOf(),
		&featureLevel,
		m_context.ReleaseAndGetAddressOf()
	));
	SetViewport();

	CreateRenderTargets();

	CreateDepthBuffer();


	return true;
}

void Renderer::D3D11App::OnResize()
{
}

void Renderer::D3D11App::SetViewport()
{
	m_viewport = {
		0.0,0.0,
		(FLOAT)m_screenWidth,
		(FLOAT)m_screenHeight,
		0.0,1.0
	};
	m_context->RSSetViewports(1, &m_viewport);
}

void Renderer::D3D11App::CreateRenderTargets()
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
	ThrowIfFailed(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_rtv.GetAddressOf()));
}

void Renderer::D3D11App::CreateDepthBuffer()
{
	
}

void Renderer::D3D11App::CreateRaseterizerState()
{
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory(&rsDesc, sizeof(rsDesc));
	rsDesc.FillMode = D3D11_FILL_SOLID;

	ThrowIfFailed(m_device->CreateRasterizerState(&rsDesc, m_rasterizerState.GetAddressOf()));
	m_context->RSSetState(m_rasterizerState.Get());
}

void Renderer::D3D11App::Update(const double& deltaTime)
{
}

void Renderer::D3D11App::Render(const double& deltaTime)
{
	m_context->OMSetRenderTargets(0, m_rtv.GetAddressOf(), nullptr);
	FLOAT clearColor[4] = { 0.f,0.f,0.f,0.f };
	m_context->ClearRenderTargetView(m_rtv.Get(), clearColor);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout.Get());

	m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

	m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	m_context->RSSetState(m_rasterizerState.Get());

	m_context->DrawIndexed(3, 0, 0);
	m_swapChain->Present(0, 0);
}