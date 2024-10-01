#include "D3D12DLSSApp.h"


Renderer::D3D12DLSSApp::D3D12DLSSApp(const int& width, const int& height)
	:D3D12PassApp(width, height)
{
	bUseGUI = true;
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
	std::shared_ptr<Core::StaticMesh> sphere = std::make_shared<Core::StaticMesh>();
	sphere->Initialize(GeometryGenerator::PbrSphere(0.4f, 100, 100, L"Bricks075A_4K-PNG0_Color.png"),
		m_device, m_commandList, DirectX::SimpleMath::Vector3(0.2f, 0.f, 2.5f), Material(), true);

	m_staticMeshes.push_back(sphere);

	/*std::shared_ptr<Core::StaticMesh> sphere = std::make_shared<Core::StaticMesh>();
	sphere->Initialize(GeometryGenerator::PbrTriangle(1.f, L"Bricks075A_4K-PNG0_Color.png"),
		m_device, m_commandList, DirectX::SimpleMath::Vector3(0.f, 0.f, 1.f), Material(), true);

	m_staticMeshes.push_back(sphere);*/

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
		m_passConstantBuffer.mStructure.ViewMat = m_camera->GetViewMatrix();
		m_passConstantBuffer.mStructure.ProjMat = m_camera->GetProjMatrix();
		m_passConstantBuffer.mStructure.eyePosition = m_camera->GetPosition();

		// 카메라 캐릭터 고정
		/*m_passConstantBuffer.mStructure.ViewMat = mCharacter->GetViewMatrix();
		m_passConstantBuffer.mStructure.ProjMat = mCharacter->GetProjMatrix();
		m_passConstantBuffer.eyePosition = mCharacter->GetCameraPosition();*/

		m_passConstantBuffer.mStructure.ViewMat = m_passConstantBuffer.mStructure.ViewMat.Transpose();
		m_passConstantBuffer.mStructure.ProjMat = m_passConstantBuffer.mStructure.ProjMat.Transpose();
		m_passConstantBuffer.UpdateBuffer();
	}

	// LightPass ConstantBuffer
	{
		m_ligthPassConstantBuffer.mStructure.eyePos = m_passConstantBuffer.mStructure.eyePosition;
		m_ligthPassConstantBuffer.mStructure.lod = gui_lod;
		m_ligthPassConstantBuffer.mStructure.light[0].position = gui_lightPos;

		m_ligthPassConstantBuffer.mStructure.ao = gui_ao;
		m_ligthPassConstantBuffer.mStructure.metallic = gui_metallic;
		m_ligthPassConstantBuffer.mStructure.roughness = gui_roughness;
		m_ligthPassConstantBuffer.mStructure.expose = gui_cubeMapExpose;
		m_ligthPassConstantBuffer.UpdateBuffer();

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
	mCubeMapConstantData.mStructure.expose = gui_cubeMapExpose;
	mCubeMapConstantData.mStructure.lodLevel = gui_cubeMapLod;
	mCubeMapConstantData.UpdateBuffer();

	mPostprocessingConstantBuffer.mStructure.bUseGamma = false;
	mPostprocessingConstantBuffer.UpdateBuffer();

	mDlssConstantBuffer.mStructure.ViewMat = m_passConstantBuffer.mStructure.ViewMat;
	mDlssConstantBuffer.mStructure.ProjMat = m_passConstantBuffer.mStructure.ProjMat;
	mDlssConstantBuffer.mStructure.prevViewMat = prevView;
	mDlssConstantBuffer.mStructure.prevProjMat = prevProj;
	mDlssConstantBuffer.mStructure.eyePosition = m_camera->GetPosition();
	mDlssConstantBuffer.UpdateBuffer();

	prevView = mDlssConstantBuffer.mStructure.ViewMat;
	prevProj = mDlssConstantBuffer.mStructure.ProjMat;
}

void Renderer::D3D12DLSSApp::UpdateGUI(float& deltaTime)
{
	ImGui::Checkbox("USE DLAA", &guiUseDLAA);
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
	ApplyAntiAliasing();
	if (guiUseDLAA) {
		
		CopyResource(m_commandList, HDRRenderTargetBuffer(), HDRRenderTargetBuffer2(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE hadnle(m_hdrSrvHeap->GetGPUDescriptorHandleForHeapStart(), 0, m_csuHeapSize);
	CopyResourceToSwapChain(deltaTime, m_hdrSrvHeap.Get(), hadnle);

}

void Renderer::D3D12DLSSApp::RenderMotionVectorPass(float& deltaTime)
{
	auto& pso = passPsoLists["RenderMotionVectorPass"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), renderMotionVectorPassEvent);
	FLOAT clearColor[4] = { 0.f, 0.f, 0.f, 0.f };
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mMotionVectorHeap->GetCPUDescriptorHandleForHeapStart());
	m_commandList->ClearRenderTargetView(handle, clearColor, 0, nullptr);

	m_commandList->OMSetRenderTargets(1, &handle, false, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());

	// View Proj Matrix Constant Buffer 
	m_commandList->SetGraphicsRootConstantBufferView(0, mDlssConstantBuffer.GetGPUVirtualAddress());


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
