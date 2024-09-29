#include "D3D12DLSSApp.h"

Renderer::D3D12DLSSApp::D3D12DLSSApp(const int& width, const int& height)
	:D3D12PassApp(width, height)
{
	bUseGUI = false;
	bRenderCubeMap = true;
	bRenderMeshes = true;
	bRenderFbx = false;
	bRenderNormal = false;

	m_camera->SetPositionAndDirection(DirectX::SimpleMath::Vector3(7.27f, 7.35f, -3.66f),
		DirectX::SimpleMath::Vector3(-0.55f, -0.62f, 0.55f));

	m_appName = "DLSSApp";
}

bool Renderer::D3D12DLSSApp::Initialize()
{
	if (!D3D12PassApp::Initialize())
		return false;

	InitializeDLSS();

	return true;
}

bool Renderer::D3D12DLSSApp::InitGUI()
{
	if (!D3D12PassApp::InitGUI())
		return false;

	return true;
}

bool Renderer::D3D12DLSSApp::InitDirectX()
{
	if (!D3D12PassApp::InitDirectX())
		return false;

	return true;
}

void Renderer::D3D12DLSSApp::InitScene()
{
	std::shared_ptr<Core::StaticMesh> sphere = std::make_shared<Core::StaticMesh>();
	sphere->Initialize(GeometryGenerator::PbrSphere(0.8f, 100, 100, L"Bricks075A_4K-PNG0_Color.png"),
		m_device, m_commandList, DirectX::SimpleMath::Vector3(0.f, 0.f, 0.f), Material(), true); 

	m_staticMeshes.push_back(sphere);

	m_cubeMap = std::make_shared<Core::StaticMesh>();
	m_cubeMap->Initialize(GeometryGenerator::SimpleCubeMapBox(100.f), m_device, m_commandList);
	m_cubeMap->SetTexturePath(std::wstring(L"Outdoor") + L"EnvHDR.dds");

	m_screenMesh = std::make_shared<Core::StaticMesh>();
	m_screenMesh->Initialize(GeometryGenerator::Rectangle(2.f, L""), m_device, m_commandList);
}

void Renderer::D3D12DLSSApp::OnResize()
{
	D3D12PassApp::OnResize();
}

void Renderer::D3D12DLSSApp::InitializeDLSS()
{
	sl::Preferences pref{};
	pref.showConsole = true; // for debugging, set to false in production
	//const wchar_t* pluginPaths[] = { L"C:/Users/son/Streamline/source/plugins/" };
	//pref.pathsToPlugins = pluginPaths; // change this if Streamline plugins are not located next to the executable
	pref.numPathsToPlugins = 0; // change this if Streamline plugins are not located next to the executable
	
	//sl::Preferences preferences = {};
	//preferences.renderAPI = sl::RenderAPI::eD3D12;  // DirectX 12 사용

	//// 로그 옵션 및 경로 설정 (옵션)
	//preferences.showConsole = true;
	//preferences.logLevel = sl::LogLevel::eDefault;
	//preferences.pathToLogsAndData = L""; // 로그 경로 설정

	//// 로드할 기능들 설정 - DLSS 기능을 로드합니다
	sl::Feature featuresToLoad[] = { sl::kFeatureDLSS };
	pref.featuresToLoad = featuresToLoad;
	pref.numFeaturesToLoad = sizeof(featuresToLoad) / sizeof(featuresToLoad[0]);

	////// 플러그인 경로 설정 (예시)
	////const wchar_t* pluginPaths[] = { L"plugins" };
	////preferences.pathsToPlugins = pluginPaths;
	////preferences.numPathsToPlugins = sizeof(pluginPaths) / sizeof(pluginPaths[0]);

	//// Streamline 초기화
	sl::Result result = slInit(pref);
	if (result != sl::Result::eOk) {
		printf("Streamline 초기화 실패: %d\n", result);
		return;
	}
}

void Renderer::D3D12DLSSApp::ApplyAntiAliasing()
{
	// 필요한 입력 구조체를 설정합니다
	sl::Resource colorResource = {};
	colorResource.native = CurrentBackBuffer();
	colorResource.type = sl::ResourceType::eTex2d;
	colorResource.width = CurrentBackBuffer()->GetDesc().Width;
	colorResource.height = CurrentBackBuffer()->GetDesc().Height;

	// 입력 구조체 배열을 생성합니다
	const sl::BaseStructure* inputs[] = {
		&colorResource
	};
	sl::FrameToken* currentToken = nullptr; // 프레임 토큰 포인터
	sl::Result result = slGetNewFrameToken(currentToken); // 고유한 프레임 토큰 얻기

	if (result == sl::Result::eOk) {
		//std::cout << "slGetNewFrameToken() 성공\n";
	}
	else {
		std::cout << "slGetNewFrameToken() 실패\n";

	}
	// 안티에일리어싱 기능을 적용합니다
	result = slEvaluateFeature(sl::kFeatureDLSS, *currentToken, inputs, _countof(inputs), m_commandList.Get());
	if (result != sl::Result::eOk) {
		printf("DLSS 적용 실패\n");
	}
}

void Renderer::D3D12DLSSApp::Update(float& deltaTime)
{
	D3D12PassApp::Update(deltaTime);
}

void Renderer::D3D12DLSSApp::UpdateGUI(float& deltaTime)
{
	D3D12PassApp::UpdateGUI(deltaTime);
}

void Renderer::D3D12DLSSApp::Render(float& deltaTime)
{
	D3D12PassApp::Render(deltaTime);
	ApplyAntiAliasing();
}

void Renderer::D3D12DLSSApp::RenderGUI(float& deltaTime)
{
	D3D12PassApp::RenderGUI(deltaTime);
}
