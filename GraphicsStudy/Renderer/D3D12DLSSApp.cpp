#include "D3D12DLSSApp.h"
static constexpr int APP_ID = 231313132;

static sl::float4x4 Get4X4(DirectX::SimpleMath::Matrix& mat) {
	sl::float4x4 ret;
	for (size_t i = 0; i < 4; i++)
	{
		ret[i].x = mat.m[i][0];
		ret[i].y = mat.m[i][1];
		ret[i].z = mat.m[i][2];
		ret[i].w = mat.m[i][3];
	}
	return ret;
}

Renderer::D3D12DLSSApp::D3D12DLSSApp(const int& width, const int& height)
	:D3D12PassApp(width, height)
{
	bUseGUI = false;
	bRenderCubeMap = true;
	bRenderMeshes = true;
	bRenderFbx = false;
	bRenderNormal = false;

	m_camera->SetPositionAndDirection(DirectX::SimpleMath::Vector3(0.f, 0.f, 0.f),
		DirectX::SimpleMath::Vector3(0.f, 0.f, 1.f));

	m_appName = "DLSSApp";
}

bool Renderer::D3D12DLSSApp::Initialize()
{
	if (!D3D12PassApp::Initialize())
		return false;

	if (!InitializeDLSS()) {
		return false;
	}

	mDlssConstantBuffer.Initialize(m_device, m_commandList);

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
	/*std::shared_ptr<Core::StaticMesh> sphere = std::make_shared<Core::StaticMesh>();
	sphere->Initialize(GeometryGenerator::PbrSphere(0.8f, 100, 100, L"Bricks075A_4K-PNG0_Color.png"),
		m_device, m_commandList, DirectX::SimpleMath::Vector3(0.f, 0.f, 0.f), Material(), true);

	m_staticMeshes.push_back(sphere);*/

	std::shared_ptr<Core::StaticMesh> sphere = std::make_shared<Core::StaticMesh>();
	sphere->Initialize(GeometryGenerator::PbrTriangle(1.f, L"Bricks075A_4K-PNG0_Color.png"),
		m_device, m_commandList, DirectX::SimpleMath::Vector3(0.f, 0.f, 1.f), Material(), true);

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

bool Renderer::D3D12DLSSApp::InitializeDLSS()
{
	sl::Preferences prefs = {};
	prefs.applicationId = APP_ID;
	prefs.renderAPI = sl::RenderAPI::eD3D12;
	prefs.logLevel = sl::LogLevel::eDefault;
	prefs.numFeaturesToLoad = 1;
	prefs.showConsole = true;
	sl::Feature featuresToLoad[] = { sl::kFeatureDLSS };
	prefs.featuresToLoad = featuresToLoad;
	const wchar_t* pluginPaths[] = {
		L"C:/Users/son/Streamline_Sample/_bin/"  // 실제 플러그인 경로로 변경
	};
	prefs.pathsToPlugins = pluginPaths;
	prefs.numPathsToPlugins = sizeof(pluginPaths) / sizeof(pluginPaths[0]);
	sl::Result result = slInit(prefs);
	if (result != sl::Result::eOk) {
		std::cerr << "Streamline 초기화 실패! 결과 코드: " << static_cast<int>(result) << std::endl;
		return false;
	}
	if (SL_FAILED(result, slSetD3DDevice(m_device.Get())))
	{
		std::cerr << "Streamline 장치 초기화 실패! 결과 코드: " << static_cast<int>(result) << std::endl;
		return false;
	}
	//slSetFeatureLoaded(sl::kFeatureDLSS_G, false);
	return true;
}

void Renderer::D3D12DLSSApp::ApplyAntiAliasing()
{
	sl::DLSSOptimalSettings dlssSettings;
	sl::DLSSOptions dlssOptions;
	// These are populated based on user selection in the UI
	dlssOptions.dlaaPreset = sl::DLSSPreset::ePresetA;
	dlssOptions.qualityPreset = sl::DLSSPreset::ePresetD;
	dlssOptions.balancedPreset = sl::DLSSPreset::ePresetD;
	dlssOptions.performancePreset = sl::DLSSPreset::ePresetD;
	dlssOptions.ultraPerformancePreset = sl::DLSSPreset::ePresetA;
	// These are populated based on user selection in the UI
	dlssOptions.mode = sl::DLSSMode::eDLAA; // e.g. sl::eDLSSModeBalanced;
	dlssOptions.outputWidth = m_screenWidth;    // e.g 1920;
	dlssOptions.outputHeight = m_screenHeight; // e.g. 1080;
	dlssOptions.colorBuffersHDR = sl::Boolean::eTrue; // assuming HDR pipeline
	dlssOptions.useAutoExposure = sl::Boolean::eFalse; // autoexposure is not to be used if a proper exposure texture is available
	dlssOptions.alphaUpscalingEnabled = sl::Boolean::eFalse; // experimental alpha upscaling, enable to upscale alpha channel of color texture
	// Now let's check what should our rendering resolution be
	if (SL_FAILED(result, slDLSSGetOptimalSettings(dlssOptions, dlssSettings)))
	{
		std::cerr << "slDLSSGetOptimalSettings 실패! 결과 코드: " << static_cast<int>(result) << std::endl;
		return;
	}
	dlssOptions.sharpness = dlssSettings.optimalSharpness; // optimal sharpness

	// Setup rendering based on the provided values in the sl::DLSSSettings structure
	m_viewport.Width = dlssSettings.renderWidthMax;
	m_viewport.Height = dlssSettings.renderHeightMax;

	sl::Resource colorIn = { sl::ResourceType::eTex2d, HDRRenderTargetBuffer(), nullptr, nullptr, static_cast<uint32_t>(D3D12_RESOURCE_STATE_RENDER_TARGET) };
	sl::Resource colorOut = { sl::ResourceType::eTex2d,  HDRRenderTargetBuffer2(), nullptr, nullptr, static_cast<uint32_t>(D3D12_RESOURCE_STATE_RENDER_TARGET) };
	sl::Resource depth = { sl::ResourceType::eTex2d, m_hdrDepthStencilBuffer.Get(), nullptr, nullptr, static_cast<uint32_t>(D3D12_RESOURCE_STATE_DEPTH_WRITE) };
	sl::Resource mvec = { sl::ResourceType::eTex2d, m_hdrMotionVector.Get(), nullptr, nullptr, static_cast<uint32_t>(D3D12_RESOURCE_STATE_RENDER_TARGET) };
	sl::Resource exposure = { sl::ResourceType::eTex2d, m_hdrExposure.Get(), nullptr, nullptr, static_cast<uint32_t>(D3D12_RESOURCE_STATE_RENDER_TARGET) };


	sl::Extent renderExtent{ 0, 0, HDRRenderTargetBuffer()->GetDesc().Width, HDRRenderTargetBuffer()->GetDesc().Height };
	sl::Extent fullExtent{ 0, 0, HDRRenderTargetBuffer2()->GetDesc().Width, HDRRenderTargetBuffer2()->GetDesc().Height };

	sl::ResourceTag colorInTag = sl::ResourceTag{ &colorIn, sl::kBufferTypeScalingInputColor, sl::ResourceLifecycle::eOnlyValidNow };
	sl::ResourceTag colorOutTag = sl::ResourceTag{ &colorOut, sl::kBufferTypeScalingOutputColor, sl::ResourceLifecycle::eOnlyValidNow };
	sl::ResourceTag depthTag = sl::ResourceTag{ &depth, sl::kBufferTypeDepth, sl::ResourceLifecycle::eValidUntilPresent };
	sl::ResourceTag mvecTag = sl::ResourceTag{ &mvec, sl::kBufferTypeMotionVectors, sl::ResourceLifecycle::eOnlyValidNow };
	sl::ResourceTag exposureTag = sl::ResourceTag{ &exposure, sl::kBufferTypeExposure, sl::ResourceLifecycle::eOnlyValidNow };
	sl::ResourceTag inputs[] = { colorInTag, colorOutTag, depthTag, mvecTag };

	if (SL_FAILED(result, slSetTag(mViewport, inputs, _countof(inputs), m_commandList.Get())))
	{
		std::cerr << "slSetTag 실패! 결과 코드: " << static_cast<int>(result) << std::endl;
		return;
	}
	// Set preferred Render Presets per Perf Quality Mode. These are typically set one time
	// and established while evaluating DLSS SR Image Quality for your Application.
	// It will be set to DSSPreset::eDefault if unspecified.
	// Please Refer to section 3.12 of the DLSS Programming Guide for details.

	if (SL_FAILED(result, slDLSSSetOptions(mViewport, dlssOptions)))
	{
		std::cerr << "slDLSSSetOptions 실패! 결과 코드: " << static_cast<int>(result) << std::endl;
		return;
	}

	if (SL_FAILED(result, slGetNewFrameToken(mCurrentFrame)))
	{
		std::cerr << "slGetNewFrameToken 실패! 결과 코드: " << static_cast<int>(result) << std::endl;
		return;
	}

	sl::Constants consts = {};
	// Set motion vector scaling based on your setup
	float aspectRatio = 1280.0f / 720.0f;
	float fov = XMConvertToRadians(70.0f);
	float nearPlane = 0.1f;
	float farPlane = 100.0f;

	// 카메라 변환 매트릭스 계산
	float f = 1.0f / tanf(fov / 2.0f);
	DirectX::SimpleMath::Matrix matrix = XMMATRIX(
		f / aspectRatio, 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, (farPlane + nearPlane) / (nearPlane - farPlane), (2 * farPlane * nearPlane) / (nearPlane - farPlane),
		0.0f, 0.0f, -1.0f, 0.0f
	);
	DirectX::SimpleMath::Matrix mat = m_camera->GetProjMatrix();
	consts.cameraViewToClip = Get4X4(mat);
	mat = mat.Invert();
	consts.clipToCameraView = Get4X4(mat);

	// 기타 변환 매트릭스들
	consts.clipToLensClip[0] = sl::float4(1, 0, 0, 0);
	consts.clipToLensClip[1] = sl::float4(0, 1, 0, 0);
	consts.clipToLensClip[2] = sl::float4(0, 0, 1, 0);
	consts.clipToLensClip[3] = sl::float4(0, 0, 0, 1);
	consts.clipToPrevClip = consts.clipToLensClip;
	consts.prevClipToClip = consts.clipToLensClip;

	// 카메라 관련 설정값
	consts.jitterOffset = sl::float2(0.25f, 0.25f);
	consts.mvecScale = sl::float2(1.0f, 1.0f);
	consts.cameraPinholeOffset = sl::float2(0.0f, 0.0f);
	consts.cameraPos = sl::float3(0.0f, 0.0f, 0.0f);
	consts.cameraUp = sl::float3(0.0f, 1.0f, 0.0f);
	consts.cameraRight = sl::float3(1.0f, 0.0f, 0.0f);
	consts.cameraFwd = sl::float3(0.0f, 0.0f, 1.0f);

	consts.cameraNear = m_camera->GetNeaPlane();
	consts.cameraFar = m_camera->GetFarPlane();
	consts.cameraFOV = m_camera->GetFov();
	consts.cameraAspectRatio = m_camera->GetAspectRatio();
	consts.motionVectorsInvalidValue = -1.0f;

	// 부울 값들
	consts.depthInverted = sl::Boolean::eFalse;
	consts.cameraMotionIncluded = sl::Boolean::eTrue;
	consts.motionVectors3D = sl::Boolean::eFalse;
	consts.reset = sl::Boolean::eTrue;
	consts.orthographicProjection = sl::Boolean::eTrue;
	consts.motionVectorsDilated = sl::Boolean::eTrue;
	consts.motionVectorsJittered = sl::Boolean::eFalse;
	//Set all other constants here
	if (SL_FAILED(result, slSetConstants(consts, *mCurrentFrame, mViewport))) // constants are changing per frame so frame index is required
	{
		std::cerr << "slSetConstants 실패! 결과 코드: " << static_cast<int>(result) << std::endl;
		return;
	}

	sl::ViewportHandle view(mViewport);
	const sl::BaseStructure* viewportInputs[] = { &view };

	if (SL_FAILED(result, slEvaluateFeature(sl::kFeatureDLSS, *mCurrentFrame, viewportInputs, _countof(viewportInputs), m_commandList.Get())))
	{
		std::cerr << "slEvaluateFeature 실패! 결과 코드: " << static_cast<int>(result) << std::endl;
		return;
	}


}

void Renderer::D3D12DLSSApp::Update(float& deltaTime)
{
	while (true) {
		if (addPlayerCount > 0) {
			addPlayerCount--;
			AddPlayer();
		}
		else
			break;
	}

	if (createSession) {
		createSession = false;
		onlineSystem->CreateLobby(4);
	}
	if (findSession) {
		findSession = false;
		onlineSystem->FindLobby();
	}
	PlayerData data;
	data.position = mCharacter->GetPosition();
	data.yTheta = mCharacter->GetYTheta();

	onlineSystem->UpdateData(data);
	onlineSystem->Update();
	for (int i = 0; i < (int)mPlayers.size(); ++i)
	{
		UpdatePlayer((int)i, onlineSystem->GetClientData(i));
	}

	//m_inputHandler->ExicuteCommand(mCharacter.get(), deltaTime, bIsFPSMode);
	//mCharacter->Update(deltaTime);
	m_inputHandler->ExicuteCommand(m_camera.get(), deltaTime, bIsFPSMode);
	m_camera->Update(deltaTime);

	{
		// 카메라 고정
		m_passConstantData->ViewMat = m_camera->GetViewMatrix();
		m_passConstantData->ProjMat = m_camera->GetProjMatrix();
		m_passConstantData->eyePosition = m_camera->GetPosition();

		// 카메라 캐릭터 고정
		/*m_passConstantData->ViewMat = mCharacter->GetViewMatrix();
		m_passConstantData->ProjMat = mCharacter->GetProjMatrix();
		m_passConstantData->eyePosition = mCharacter->GetCameraPosition();*/

		m_passConstantData->ViewMat = m_passConstantData->ViewMat.Transpose();
		m_passConstantData->ProjMat = m_passConstantData->ProjMat.Transpose();
		memcpy(m_pCbvDataBegin, m_passConstantData, sizeof(GlobalVertexConstantData));
	}

	// LightPass ConstantBuffer
	{
		m_ligthPassConstantData->eyePos = m_passConstantData->eyePosition;
		m_ligthPassConstantData->lod = gui_lod;
		m_ligthPassConstantData->light[0].position = gui_lightPos;

		m_ligthPassConstantData->ao = gui_ao;
		m_ligthPassConstantData->metallic = gui_metallic;
		m_ligthPassConstantData->roughness = gui_roughness;
		m_ligthPassConstantData->expose = gui_cubeMapExpose;

		memcpy(m_pLPCDataBegin, m_ligthPassConstantData, sizeof(LightPassConstantData));
	}

	DirectX::SimpleMath::Matrix mat = DirectX::XMMatrixTranslation(gui_lightPos.x, gui_lightPos.y, gui_lightPos.z);
	if (m_lightMeshes.size() > 0)
		m_lightMeshes[0]->UpdateWorldRow(mat);

	for (auto& mesh : m_staticMeshes) {
		mesh->Update(deltaTime);
	}

	for (auto& mesh : m_lightMeshes) {
		mesh->Update(deltaTime);
	}

	for (auto& fbx : m_fbxList) {
		fbx->Update(deltaTime);
	}

	// ComputeShader(PostProcessing)
	mCsBuffer.mStructure.time = ((deltaTime) < (1 / 60.f) ? deltaTime : (1 / 60.f));
	mCsBuffer.UpdateBuffer();

	// CubeMap ConstantBuffer
	m_pCubeMapConstantData->expose = gui_cubeMapExpose;
	m_pCubeMapConstantData->lodLevel = gui_cubeMapLod;
	memcpy(m_pCubeMapCbufferBegin, m_pCubeMapConstantData, sizeof(CubeMapConstantData));

	mPostprocessingConstantBuffer.mStructure.bUseGamma = false;
	mPostprocessingConstantBuffer.UpdateBuffer();

	mDlssConstantBuffer.mStructure.ViewMat = m_passConstantData->ViewMat;
	mDlssConstantBuffer.mStructure.ProjMat = m_passConstantData->ProjMat;
	mDlssConstantBuffer.mStructure.prevViewMat = prevView;
	mDlssConstantBuffer.mStructure.prevProjMat = prevProj;
	mDlssConstantBuffer.mStructure.eyePosition = m_camera->GetPosition();
	mDlssConstantBuffer.UpdateBuffer();

	prevView = mDlssConstantBuffer.mStructure.ViewMat;
	prevProj = mDlssConstantBuffer.mStructure.ProjMat;
}

void Renderer::D3D12DLSSApp::UpdateGUI(float& deltaTime)
{
	D3D12PassApp::UpdateGUI(deltaTime);
}

void Renderer::D3D12DLSSApp::Render(float& deltaTime)
{
	GeometryPass(deltaTime);
	FbxGeometryPass(deltaTime);
	//RenderCubeMap(deltaTime);
	LightPass(deltaTime);
	RenderNormalPass(deltaTime);
	RenderBoundingBoxPass(deltaTime);
	//CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());

	PostProcessing(deltaTime);

	RenderMotionVectorPass(deltaTime);


	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
	ApplyAntiAliasing();
	
	FlushCommandList(m_commandList);
	
	CopyResource(m_commandList, HDRRenderTargetBuffer(), HDRRenderTargetBuffer2(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	CD3DX12_GPU_DESCRIPTOR_HANDLE hadnle(m_hdrSrvHeap->GetGPUDescriptorHandleForHeapStart(), 0, m_csuHeapSize);
	CopyResourceToSwapChain(deltaTime, m_hdrSrvHeap.Get(), hadnle);

}

void Renderer::D3D12DLSSApp::RenderMotionVectorPass(float& deltaTime)
{
	auto& pso = passPsoLists["RenderMotionVectorPass"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), renderMotionVectorPassEvent);
	FLOAT clearColor[2] = { 0.f, 0.f };
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mMotionVectorHeap->GetCPUDescriptorHandleForHeapStart());
	m_commandList->ClearRenderTargetView(handle, clearColor, 0, nullptr);

	m_commandList->OMSetRenderTargets(1, &handle, false, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());

	// View Proj Matrix Constant Buffer 
	m_commandList->SetGraphicsRootConstantBufferView(0, mDlssConstantBuffer.GetGpuAddress());


	if (bRenderMeshes)
	{
		for (int i = 0; i < m_staticMeshes.size(); ++i) {
			for (int j = 0; j < (int)m_staticMeshes[i]->meshCount; j++)
			{
				m_staticMeshes[i]->Render(deltaTime, m_commandList, true, (int)j);
			}
		}
	}

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* plists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12DLSSApp::RenderGUI(float& deltaTime)
{
	D3D12PassApp::RenderGUI(deltaTime);
}
