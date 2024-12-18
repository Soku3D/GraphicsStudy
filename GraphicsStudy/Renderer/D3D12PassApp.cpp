#include "D3D12PassApp.h"
#include "Renderer.h"
#include <DirectXTexEXR.h>
#include "SteamOnlineSystem.h"

using namespace Network;

std::ostream& operator<<(std::ostream& out, const DirectX::SimpleMath::Matrix& mat) {
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			out << mat.m[i][j] << ' ';
		}
		out << '\n';
	}
	return out;
}


Renderer::D3D12PassApp::D3D12PassApp(const int& width, const int& height)
	:D3D12App(width, height)
{
	bUseGUI = true;
	bRenderCubeMap = true;
	bRenderMeshes = true;
	bRenderFbx = false;
	bRenderNormal = false;
	bUseDLAA = false;
	bSkeleton = true;
	m_appName = "PassApp";
}

Renderer::D3D12PassApp::~D3D12PassApp()
{
	if (skeletonMesh != nullptr)
		delete skeletonMesh;
	std::cout << "~D3D12PassApp" << std::endl;
	//light = nullptr;
}

bool Renderer::D3D12PassApp::Initialize()
{
	if (!D3D12App::Initialize())
		return false;

	if (bUseDLAA) {
		if (!InitializeDLSS()) {
			return false;
		}
	}
	pLight = new Core::Light();

	gui_lightPos = DirectX::SimpleMath::Vector3(0.f, 1.5f, 0.f);
	pLight->SetPositionAndDirection(gui_lightPos, XMFLOAT3(0, -1, 1));
	InitConstantBuffers();

	return true;
}

void Renderer::D3D12PassApp::InitConstantBuffers()
{
	mCubeMapConstantData.Initialize(m_device, m_commandList);
	mSkinnedMeshConstantData.Initialize(m_device, m_commandList);
}

void Renderer::D3D12PassApp::InitScene()
{
	using DirectX::SimpleMath::Vector3;
	using DirectX::SimpleMath::Matrix;
	using namespace Core;

	Matrix tr = Matrix();
	std::tuple<std::vector<PbrSkinnedMeshData>, Animation::AnimationData> soldierData;
	soldierData = GeometryGenerator::ReadFromFile<PbrSkinnedVertex, uint32_t>("KidneyHit.fbx", true, true, tr);
	skinnedMeshsoldier = std::get<0>(soldierData);
	soldierAnimation = std::get<1>(soldierData);

	mCharacter->InitStaticMesh(skinnedMeshsoldier, m_device, m_commandList);
	mCharacter->SetPosition(XMFLOAT3(0, 1.475f, 0));
	mCharacter->SetTexturePath(L"Soldier_Body_Albedo.dds", 0);
	mCharacter->SetTexturePath(L"Soldier_head_Albedo.dds", 1);
	mCharacter->SetTexturePath(L"Soldier_Body_Albedo.dds", 2);

	soldierAnimation.offsetMatrices[7] *= DirectX::XMMatrixRotationY(XMConvertToRadians(20)) * DirectX::XMMatrixTranslation(-2.791f, 11.163f, -8.093f);
	soldierAnimation.offsetMatrices[8] *= DirectX::XMMatrixRotationY(XMConvertToRadians(-20)) * DirectX::XMMatrixTranslation(2.791f, 11.163f, -8.093f);
	soldierAnimation.offsetMatrices[9] *= DirectX::XMMatrixRotationX(0.23f) * DirectX::XMMatrixTranslation(0.f, 4.f, -2.5f);
	soldierAnimation.offsetMatrices[6] *= DirectX::XMMatrixTranslation(0.f, 4.f, 0.f);
	soldierAnimation.offsetMatrices[5] *= DirectX::XMMatrixTranslation(0.f, 4.f, 0.f);

	//jaw = soldierAnimation.offsetMatrices[9];

	mCharacter->SetMeshBoundingBox(1.f);
	skeletonMesh = new Core::SkeletonMesh();
	for (size_t i = 0; i < soldierAnimation.bonePositions.size(); i++)
	{
		DirectX::SimpleMath::Vector4 pos = DirectX::SimpleMath::Vector4(0, 0, 0, 1);
		soldierAnimation.bonePositions[i] = DirectX::SimpleMath::Vector3(pos.x, pos.y, pos.z);
	}
	skeletonMesh->Initialize(soldierAnimation.bonePositions, m_device, m_commandList);

	for (size_t i = 0; i < LIGHT_COUNT; i++)
	{
		std::shared_ptr<StaticMesh> sphere = std::make_shared<StaticMesh>();
		sphere->Initialize(GeometryGenerator::PbrSphere(0.1f, 10, 10, L"zzzdefaultAlbedo.dds", 2.f, 2.f), m_device, m_commandList,
			Vector3(0.f, 0.f, 0.f),
			Material(1.f, 1.f, 0.1f, 1.f),
			false /*AO*/, false /*Height*/, false /*Metallic*/, false /*Normal*/, false /*Roughness*/, false /*Tesslation*/);
		m_lightMeshes.push_back(sphere);
	}
	std::shared_ptr<StaticMesh> plane = std::make_shared<StaticMesh>();
	std::vector<PbrMeshData> planeData = { GeometryGenerator::PbrBox(10, 1, 10, L"worn-painted-metal_Albedo.dds", 10, 1, 10) };
	plane->Initialize(planeData, m_device, m_commandList,
		Vector3(0.f, 0.f, 0.f),
		Material(1.f, 1.f, 1.f, 1.f),
		true, true, true, true, true, true);
	m_staticMeshes.push_back(plane);

	//auto [box_destruction, box_destruction_animation] = GeometryGenerator::ReadFromFile_Pbr("uvtest4.fbx", true);
	//std::shared_ptr<Animation::FBX> wallDistructionFbx = std::make_shared<Animation::FBX>();
	//wallDistructionFbx->Initialize(box_destruction, box_destruction_animation, m_device, m_commandList,
	//	true /*loopAnimation*/,
	//	1.5f /*animationSpeed*/,
	//	Vector3(0.f, 1.f, 0.f),
	//	L"uvtest_DefaultMaterial_Albedo.dds");

	//m_fbxList.push_back(wallDistructionFbx);

	m_cubeMap = std::make_shared<Core::StaticMesh>();

	m_cubeMap->Initialize(GeometryGenerator::SimpleCubeMapBox(500.f), m_device, m_commandList);
	m_cubeMap->SetTexturePath(std::wstring(L"Outdoor") + L"EnvHDR.dds");

	m_screenMesh = std::make_shared<Core::StaticMesh>();
	std::vector<BasicMeshData> screeData = { GeometryGenerator::Rectangle(2.f, L"") };
	m_screenMesh->Initialize(screeData, m_device, m_commandList);
}

bool Renderer::D3D12PassApp::InitGUI()
{
	if (!D3D12App::InitGUI())
		return false;
	return true;
}

bool Renderer::D3D12PassApp::InitDirectX()
{
	if (!D3D12App::InitDirectX())
		return false;

	return true;
}

void Renderer::D3D12PassApp::OnResize()
{
	D3D12App::OnResize();
}

void Renderer::D3D12PassApp::Update(float& deltaTime)
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

	if (cameraFocusing) {
		m_inputHandler->ExicuteCommand(mCharacter.get(), deltaTime, bIsFPSMode);
		mCharacter->Update(deltaTime);
	}
	else {
		m_inputHandler->ExicuteCommand(m_camera.get(), deltaTime, bIsFPSMode);
		m_camera->Update(deltaTime);
	}


	{
		if (!cameraFocusing) {
			m_passConstantBuffer.mStructure.ViewMat = m_camera->GetViewMatrix();
			m_passConstantBuffer.mStructure.ProjMat = m_camera->GetProjMatrix();
			m_passConstantBuffer.mStructure.eyePosition = m_camera->GetPosition();
		}
		else {
			// 카메라 캐릭터 고정
			m_passConstantBuffer.mStructure.ViewMat = mCharacter->GetViewMatrix();
			m_passConstantBuffer.mStructure.ProjMat = mCharacter->GetProjMatrix();
			m_passConstantBuffer.mStructure.eyePosition = mCharacter->GetCameraPosition();
		}


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

	// Update Skeleton Animation
	static float frame = 0;

	if (bRunAnimation) {
		frame += 0.5f;
	}
	//DirectX::SimpleMath::Matrix m = DirectX::XMMatrixRotationY(gui_eyeRotation);
	soldierAnimation.Update((int)frame);
	if (frame > soldierAnimation.clips[0].keys.size())
	{
		frame = 0;
	}

	//soldierAnimation.boneTransforms[6] = head * DirectX::XMMatrixTranslation(gui_headX, gui_headY, gui_headZ);
	//soldierAnimation.offsetMatrices[9] = jaw * DirectX::XMMatrixRotationX(gui_jawRotation) *DirectX::XMMatrixTranslation(0,0,gui_jawZ);
	float ndcX = -2;
	float ndcY = -2;
	if (lMouseButtonClicked) {

		GetCursorPos(&mCursorPosition);
		ScreenToClient(m_mainWnd, &mCursorPosition);

		ndcX = (2.f * ((mCursorPosition.x + 0.5f) / (m_screenWidth))) - 1.f;
		ndcY = (2.f * ((mCursorPosition.y + 0.5f) / (m_screenHeight))) - 1.f;
	}

	for (int i = 0; i < soldierAnimation.boneTransforms.size(); i++)
	{
		int maxIdx = (int)soldierAnimation.boneTransforms.size() - 1;
		mSkinnedMeshConstantData.mStructure.boneTransforms[i] = soldierAnimation.Get(i).Transpose();
		mSkinnedMeshConstantData.mStructure.baseTransforms[i] = (soldierAnimation.tPoseTransforms[i] * soldierAnimation.defaultTransform).Transpose();

		int32_t parentsId = soldierAnimation.boneParentsId[i];
		int index = i / 4;
		int index2 = i % 4;
		if (i <= maxIdx)
		{
			if (index2 == 0)
				mSkinnedMeshConstantData.mStructure.parentsIndex[index].x = (float)parentsId;
			else if (index2 == 1)
				mSkinnedMeshConstantData.mStructure.parentsIndex[index].y = (float)parentsId;
			else if (index2 == 2)
				mSkinnedMeshConstantData.mStructure.parentsIndex[index].z = (float)parentsId;
			else
				mSkinnedMeshConstantData.mStructure.parentsIndex[index].w = (float)parentsId;

		}
		if (lMouseButtonClicked && parentsId > 0) {
			GetCursorPos(&mCursorPosition);
			ScreenToClient(m_mainWnd, &mCursorPosition);

			float ndcX = (2.f * ((mCursorPosition.x + 0.5f) / (m_screenWidth))) - 1.f;
			float ndcY = (2.f * ((mCursorPosition.y + 0.5f) / (m_screenHeight))) - 1.f;
			ndcY *= -1;

			DirectX::SimpleMath::Vector3 childCenter = DirectX::SimpleMath::Vector4::Transform(DirectX::SimpleMath::Vector4(0, 0, 0, 1), soldierAnimation.tPoseTransforms[i] * soldierAnimation.defaultTransform * soldierAnimation.Get(i));
			DirectX::SimpleMath::Vector3 parentsCenter = DirectX::SimpleMath::Vector4::Transform(DirectX::SimpleMath::Vector4(0, 0, 0, 1), soldierAnimation.tPoseTransforms[parentsId] * soldierAnimation.defaultTransform * soldierAnimation.Get(parentsId));

			//DirectX::SimpleMath::Vector3 childCenter = DirectX::SimpleMath::Vector4::Transform(DirectX::SimpleMath::Vector4(0, 0, 0, 1), soldierAnimation.tPoseTransforms[i] * soldierAnimation.defaultTransform * soldierAnimation.Get(i));
			//DirectX::SimpleMath::Vector3 parentsCenter = DirectX::SimpleMath::Vector4::Transform(DirectX::SimpleMath::Vector4(0, 0, 0, 1), soldierAnimation.tPoseTransforms[parentsId] * soldierAnimation.defaultTransform * soldierAnimation.Get(parentsId));

			DirectX::SimpleMath::Vector3 center = (childCenter + parentsCenter) * 0.5f;
			float length = (childCenter - parentsCenter).Length();
			skeletonBoundingSphere.Radius = length / 2.5f;
			skeletonBoundingSphere.Center = center;

			DirectX::SimpleMath::Vector3 ndcNear = DirectX::SimpleMath::Vector3(ndcX, ndcY, 0.f);
			DirectX::SimpleMath::Vector3 ndcFar = DirectX::SimpleMath::Vector3(ndcX, ndcY, 1.f);
			DirectX::SimpleMath::Matrix inverseViewProj = (m_camera->GetViewMatrix() * m_camera->GetProjMatrix()).Invert();

			DirectX::SimpleMath::Vector3 nearWorld = DirectX::SimpleMath::Vector3::Transform(ndcNear, inverseViewProj);
			DirectX::SimpleMath::Vector3 farWorld = DirectX::SimpleMath::Vector3::Transform(ndcFar, inverseViewProj);
			DirectX::SimpleMath::Vector3 o = nearWorld;
			DirectX::SimpleMath::Vector3 d = farWorld - nearWorld;
			d.Normalize();
			DirectX::SimpleMath::Ray curRay = DirectX::SimpleMath::Ray(nearWorld, d);


			float l;
			if (curRay.Intersects(skeletonBoundingSphere, l))
			{
				skeletonName = soldierAnimation.boneIdToName[i];
				selectedSkeletonId = i;
				if (index2 == 0)
					mSkinnedMeshConstantData.mStructure.isClicked[index].x = 1.f;
				else if (index2 == 1)
					mSkinnedMeshConstantData.mStructure.isClicked[index].y = 1.f;
				else if (index2 == 2)
					mSkinnedMeshConstantData.mStructure.isClicked[index].z = 1.f;
				else
					mSkinnedMeshConstantData.mStructure.isClicked[index].w = 1.f;
			}
			else {
				//skeletonName = "";

				if (index2 == 0)
					mSkinnedMeshConstantData.mStructure.isClicked[index].x = 0.f;
				else if (index2 == 1)
					mSkinnedMeshConstantData.mStructure.isClicked[index].y = 0.f;
				else if (index2 == 2)
					mSkinnedMeshConstantData.mStructure.isClicked[index].z = 0.f;
				else
					mSkinnedMeshConstantData.mStructure.isClicked[index].w = 0.f;

			}
		}
		else if (selectedSkeletonId == i)
		{
			skeletonName = soldierAnimation.boneIdToName[i];
			//selectedSkeletonId = i;
			if (index2 == 0)
				mSkinnedMeshConstantData.mStructure.isClicked[index].x = 1.f;
			else if (index2 == 1)
				mSkinnedMeshConstantData.mStructure.isClicked[index].y = 1.f;
			else if (index2 == 2)
				mSkinnedMeshConstantData.mStructure.isClicked[index].z = 1.f;
			else
				mSkinnedMeshConstantData.mStructure.isClicked[index].w = 1.f;
		}
		else {
			//skeletonName = "";
			//selectedSkeletonId = -1;

			if (index2 == 0)
				mSkinnedMeshConstantData.mStructure.isClicked[index].x = 0.f;
			else if (index2 == 1)
				mSkinnedMeshConstantData.mStructure.isClicked[index].y = 0.f;
			else if (index2 == 2)
				mSkinnedMeshConstantData.mStructure.isClicked[index].z = 0.f;
			else
				mSkinnedMeshConstantData.mStructure.isClicked[index].w = 0.f;
		}
	}
	mSkinnedMeshConstantData.UpdateBuffer();

	//Update Light
	pLight->SetPositionAndDirection(gui_lightPos, XMFLOAT3(0, -1, 1));
	DirectX::SimpleMath::Matrix mat = DirectX::XMMatrixTranslation(gui_lightPos.x, gui_lightPos.y, gui_lightPos.z);
	if (m_lightMeshes.size() > 0)
		m_lightMeshes[0]->UpdateWorldRow(mat);

}

void Renderer::D3D12PassApp::UpdateGUI(float& deltaTime)
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
	if (ImGui::Button("CreateSession")) {
		createSession = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("FindSession")) {
		findSession = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Animation")) {
		bRunAnimation = !bRunAnimation;
	}
	ImGui::Text(skeletonName.c_str());

	ImGui::SliderFloat("Skeleton X", &gui_skeletonX, -2.f, 2.f);
	ImGui::SliderFloat("Skeleton Y", &gui_skeletonY, -2, 2);
	ImGui::SliderFloat("Skeleton Z", &gui_skeletonZ, -2, 2);

	ImGui::SliderFloat("jawRotation", &gui_jawRotation, -3.14f, 3.14f);
	ImGui::SliderFloat("eyeRotation", &gui_eyeRotation, -3.14f, 3.14f);

	ImGui::SliderFloat("AO", &gui_material.ao, 0.f, 1.f);
	ImGui::SliderFloat("Metalic", &gui_material.metallic, 0.f, 1.f);
	ImGui::SliderFloat("Roughness", &gui_material.roughness, 0.f, 1.f);
	//ImGui::SliderFloat("LOD", &gui_lod, 0.f, 10.f);

	ImGui::SliderFloat("CubeMap LodLevel", &gui_cubeMapLod, 0.f, 10.f);
	ImGui::SliderFloat("CubeMap Expose", &gui_cubeMapExpose, 0.f, 10.f);
	ImGui::SliderFloat3("Light Position", (float*)(&gui_lightPos), -10.f, 10.f);

	ImGui::Checkbox("Render BoundingBox", &bRenderBoundingBox);
	ImGui::SameLine();
	ImGui::Checkbox("Render Normal", &bRenderNormal);
	ImGui::SameLine();
	ImGui::Checkbox("Render CubeMap", &bRenderCubeMap);

	ImGui::Checkbox("Character Focusing", &cameraFocusing);
	ImGui::SameLine();
	ImGui::Checkbox("Render FBX", &bRenderFbx);
	ImGui::SameLine();
	ImGui::Checkbox("Render Mesh", &bRenderMeshes);
	ImGui::SameLine();
	ImGui::Checkbox("Use DLAA", &guiUseDLAA);
	ImGui::SliderFloat("Jitter Offset", &guiDLAAJitterOffset, 0.f, 10.f);


}

void Renderer::D3D12PassApp::Render(float& deltaTime)
{
	GeometryPass(deltaTime);
	FbxGeometryPass(deltaTime);
	SkinnedMeshGeometryPass(deltaTime);
	//DepthOnlyPass(deltaTime);
	RenderCubeMap(deltaTime);
	LightPass(deltaTime);
	RenderNormalPass(deltaTime);
	RenderBoundingBoxPass(deltaTime);
	RenderSkeleton(deltaTime);
	//CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());

	PostProcessing(deltaTime);

	if (bUseDLAA) {
		ApplyAntiAliasing();
		if (guiUseDLAA) {

			CopyResource(m_commandList, HDRRenderTargetBuffer(), HDRRenderTargetBuffer2(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
	}


	CopyResourceToSwapChain(deltaTime);
}

void Renderer::D3D12PassApp::GeometryPass(float& deltaTime) {

	auto& pso = passPsoLists[currRenderMode + "GeometryPass"];
	//auto& pso = passPsoLists["DefaultGeometryPass"];
	if (currRenderMode == "Msaa") {
		msaaMode = true;
	}
	else {
		msaaMode = false;
	}

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	{
		PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), geomeytyPassEvent);
		FLOAT clearColor[4] = { 0.f,0.f,0.f,0.f };

		if (msaaMode) {
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GeometryPassMsaaRTV());
			for (UINT i = 0; i < geometryPassRtvNum; i++)
			{
				m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
				rtvHandle.Offset(1, m_rtvHeapSize);
			}
			m_commandList->ClearDepthStencilView(MsaaDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
				1.f, 0, 0, nullptr);

			m_commandList->OMSetRenderTargets(geometryPassRtvNum, &GeometryPassMsaaRTV(), true, &MsaaDepthStencilView());
		}
		else {
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GeometryPassRTV());
			for (UINT i = 0; i < geometryPassRtvNum; i++)
			{
				m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
				rtvHandle.Offset(1, m_rtvHeapSize);
			}

			m_commandList->ClearDepthStencilView(HDRDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
				1.f, 0, 0, nullptr);

			m_commandList->OMSetRenderTargets(geometryPassRtvNum, &GeometryPassRTV(), true, &HDRDepthStencilView());
		}
		m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->RSSetScissorRects(1, &m_scissorRect);
		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());

		// View Proj Matrix Constant Buffer 
		m_commandList->SetGraphicsRootConstantBufferView(2, m_passConstantBuffer.GetGPUVirtualAddress());

		// Texture SRV Heap 
		ID3D12DescriptorHeap* ppSrvHeaps[] = { m_textureHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppSrvHeaps), ppSrvHeaps);

		if (bRenderMeshes)
		{
			RenderStaticMeshes(deltaTime);
		}
		if (bRenderLights)
		{
			RenderLightMeshes(deltaTime);
		}
		// Render Player
		//RenderCharacter(deltaTime);
		//RenderPlayers(deltaTime);
	}

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* plists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());
}

// FBX 렌더의 경우 Domain & Hull Shader를 사용하지 않고, NormalMap도 사용하지 않는다
void Renderer::D3D12PassApp::FbxGeometryPass(float& deltaTime) {

	auto& pso = passPsoLists[currRenderMode + "FbxGeometryPass"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), fbxGeomeytyPassEvent);

	if (bRenderFbx) {

		if (msaaMode) {
			m_commandList->OMSetRenderTargets(geometryPassRtvNum, &GeometryPassMsaaRTV(), true, &MsaaDepthStencilView());
		}
		else {
			m_commandList->OMSetRenderTargets(geometryPassRtvNum, &GeometryPassRTV(), true, &HDRDepthStencilView());
		}

		m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->RSSetScissorRects(1, &m_scissorRect);
		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());

		// View Proj Matrix Constant Buffer 
		m_commandList->SetGraphicsRootConstantBufferView(2, m_passConstantBuffer.GetGPUVirtualAddress());
		//m_commandList->SetGraphicsRootConstantBufferView(3, m_ligthPassConstantBuffer->GetGPUVirtualAddress());

		// Texture SRV Heap 
		ID3D12DescriptorHeap* ppSrvHeaps[] = { m_textureHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppSrvHeaps), ppSrvHeaps);

		for (int i = 0; i < m_fbxList.size(); ++i) {
			m_fbxList[i]->Render(deltaTime, m_commandList, true, m_textureHeap, m_textureMap, m_csuHeapSize);
		}
	}

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* plists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12PassApp::SkinnedMeshGeometryPass(float& deltaTime) {

	auto& pso = passPsoLists["SkinnedMeshGeometryPass"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), fbxGeomeytyPassEvent);

	if (msaaMode) {
		m_commandList->OMSetRenderTargets(geometryPassRtvNum, &GeometryPassMsaaRTV(), true, &MsaaDepthStencilView());
	}
	else {
		m_commandList->OMSetRenderTargets(geometryPassRtvNum, &GeometryPassRTV(), true, &HDRDepthStencilView());
	}

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());

	// View Proj Matrix Constant Buffer 
	m_commandList->SetGraphicsRootConstantBufferView(2, m_passConstantBuffer.GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootConstantBufferView(3, mSkinnedMeshConstantData.GetGPUVirtualAddress());

	// Texture SRV Heap 
	ID3D12DescriptorHeap* ppSrvHeaps[] = { m_textureHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppSrvHeaps), ppSrvHeaps);

	// Render Player
	RenderCharacter(deltaTime);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* plists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(plists), plists);


	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12PassApp::DepthOnlyPass(float& deltaTime) {

	auto& pso = passPsoLists["DepthOnlyPass"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), geomeytyPassEvent);
	FLOAT clearColor[4] = { 0.f,0.f,0.f,0.f };


	m_commandList->OMSetRenderTargets(geometryPassRtvNum, &GeometryPassMsaaRTV(), true, &MsaaDepthStencilView());

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());

	// View Proj Matrix Constant Buffer 
	m_commandList->SetGraphicsRootConstantBufferView(2, m_passConstantBuffer.GetGPUVirtualAddress());

	// Texture SRV Heap 
	ID3D12DescriptorHeap* ppSrvHeaps[] = { m_textureHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppSrvHeaps), ppSrvHeaps);

	if (bRenderMeshes)
	{
		RenderStaticMeshes(deltaTime);
	}

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* plists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());
}

void Renderer::D3D12PassApp::LightPass(float& deltaTime) {
	//auto& pso = passPsoLists[currRenderMode + "LightPass"];
	auto& pso = passPsoLists["DefaultLightPass"];
	if (currRenderMode == "Msaa") {
		msaaMode = true;
	}
	else {
		msaaMode = false;
	}
	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	{
		PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), lightPassEvent);

		FLOAT clearColor[4] = { 0.f,0.f,0.f,0.f };
		//if (msaaMode) {
		//	m_commandList->ClearRenderTargetView(MsaaRenderTargetView(), clearColor, 0, nullptr);
		//	//m_commandList->OMSetRenderTargets(1, &MsaaRenderTargetView(), false, nullptr);
		//	//m_commandList->OMSetRenderTargets(1, &MsaaRenderTargetView(), true, &MsaaDepthStencilView());
		//}
		//else {
		//	m_commandList->ClearRenderTargetView(HDRRendertargetView(), clearColor, 0, nullptr);
		//	//m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), false, nullptr);
		//	//m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());

		//}

		m_commandList->ClearRenderTargetView(HDRRendertargetView(), clearColor, 0, nullptr);
		m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), false, nullptr);
		m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->RSSetScissorRects(1, &m_scissorRect);
		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());

		ID3D12DescriptorHeap* ppSrvHeaps[] = { m_geometryPassSrvHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppSrvHeaps), ppSrvHeaps);

		m_commandList->SetGraphicsRootDescriptorTable(0, m_geometryPassSrvHeap->GetGPUDescriptorHandleForHeapStart());

		int cubeMapOffset = m_cubeTextureMap[m_cubeMap->GetTexturePath()] - 2;
		CD3DX12_GPU_DESCRIPTOR_HANDLE cubeMapHandle(m_geometryPassSrvHeap->GetGPUDescriptorHandleForHeapStart(), geometryPassRtvNum + 1/*depth*/ + cubeMapOffset, m_csuHeapSize);
		m_commandList->SetGraphicsRootDescriptorTable(1, cubeMapHandle);
		m_commandList->SetGraphicsRootConstantBufferView(2, m_ligthPassConstantBuffer.GetGPUVirtualAddress());
		m_screenMesh->Render(deltaTime, m_commandList, false);
	}

	/*if (msaaMode) {
		ResolveSubresource(m_commandList,HDRRenderTargetBuffer(), MsaaRenderTargetBuffer());
	}*/
	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* plists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());

}

void Renderer::D3D12PassApp::RenderNormalPass(float& deltaTime) {

	if (bRenderNormal) {
		auto& pso = passPsoLists["NormalPass"];


		ThrowIfFailed(m_commandAllocator->Reset());
		ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
		{
			PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), drawNormalPassEvent);


			m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());

			m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
			m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());
			m_commandList->RSSetScissorRects(1, &m_scissorRect);
			m_commandList->RSSetViewports(1, &m_viewport);

			m_commandList->SetGraphicsRootConstantBufferView(0, m_passConstantBuffer.GetGPUVirtualAddress());

			if (bRenderMeshes) {
				for (int i = 0; i < (int)m_staticMeshes.size(); ++i) {
					for (int j = 0; j < (int)m_staticMeshes[i]->meshCount; j++) {
						m_staticMeshes[i]->RenderNormal(deltaTime, m_commandList, true, j);
					}
				}
			}
			for (int j = 0; j < (int)mCharacter->GetMeshCount(); j++) {
				mCharacter->RenderNormal(deltaTime, m_commandList, true, j);
			}
		}

		ThrowIfFailed(m_commandList->Close());

		ID3D12CommandList* plists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

		FlushCommandQueue();
		PIXEndEvent(m_commandQueue.Get());
	}

}

void Renderer::D3D12PassApp::RenderBoundingBoxPass(float& deltaTime) {

	if (bRenderBoundingBox) {
		auto& pso = passPsoLists["BoundingBoxPass"];


		ThrowIfFailed(m_commandAllocator->Reset());
		ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
		{
			PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), renderBoundingBoxPassEvent);
			m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());

			m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
			m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());
			m_commandList->RSSetScissorRects(1, &m_scissorRect);
			m_commandList->RSSetViewports(1, &m_viewport);

			m_commandList->SetGraphicsRootConstantBufferView(1, m_passConstantBuffer.GetGPUVirtualAddress());

			if (bRenderMeshes) {
				for (int i = 0; i < m_staticMeshes.size(); ++i) {
					m_staticMeshes[i]->RenderBoundingBox(deltaTime, m_commandList);
				}
			}
			mCharacter->RenderBoundingBox(deltaTime, m_commandList);
		}

		ThrowIfFailed(m_commandList->Close());

		ID3D12CommandList* plists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

		FlushCommandQueue();
		PIXEndEvent(m_commandQueue.Get());
	}

}

void Renderer::D3D12PassApp::RenderSkeleton(float& deltaTime) {

	if (bSkeleton) {
		auto& pso = passPsoLists["RenderSkeleton"];


		ThrowIfFailed(m_commandAllocator->Reset());
		ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
		{
			PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), renderBoundingBoxPassEvent);
			m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), false, nullptr);

			m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
			m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());
			m_commandList->RSSetScissorRects(1, &m_scissorRect);
			m_commandList->RSSetViewports(1, &m_viewport);

			m_commandList->SetGraphicsRootConstantBufferView(0, mSkinnedMeshConstantData.GetGPUVirtualAddress());
			m_commandList->SetGraphicsRootConstantBufferView(1, m_passConstantBuffer.GetGPUVirtualAddress());
			//m_commandList->DrawInstanced(56, 1, 0, 0);
			skeletonMesh->Render(deltaTime, m_commandList);
		}

		ThrowIfFailed(m_commandList->Close());

		ID3D12CommandList* plists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

		FlushCommandQueue();
		PIXEndEvent(m_commandQueue.Get());
	}

}

void Renderer::D3D12PassApp::RenderCubeMap(float& deltaTime)
{
	auto& pso = cubePsoLists[(currRenderMode + "CubeMap")];
	//auto& pso = cubePsoLists[("DefaultCubeMap")];
	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), cubeMapPassEvent);

	if (bRenderCubeMap)
	{

		/*if (msaaMode) {

			m_commandList->OMSetRenderTargets(1, &MsaaRenderTargetView(), true, &MsaaDepthStencilView());
		}
		else {

			m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());
		}*/
		if (msaaMode) {
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GeometryPassMsaaRTV());
			rtvHandle.Offset(2, m_rtvHeapSize);
			m_commandList->OMSetRenderTargets(2, &rtvHandle, true, &MsaaDepthStencilView());
		}
		else {
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GeometryPassRTV());
			rtvHandle.Offset(2, m_rtvHeapSize);
			m_commandList->OMSetRenderTargets(2, &rtvHandle, true, &HDRDepthStencilView());
		}

		//m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());

		m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());
		m_commandList->RSSetScissorRects(1, &m_scissorRect);
		m_commandList->RSSetViewports(1, &m_viewport);

		// View Proj Matrix Constant Buffer 
		m_commandList->SetGraphicsRootConstantBufferView(1, m_passConstantBuffer.GetGPUVirtualAddress());

		//CubeMap Expose & LodLevel
		m_commandList->SetGraphicsRootConstantBufferView(2, mCubeMapConstantData.GetGPUVirtualAddress());

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

	if (bRenderFPS)
	{
		DirectX::SimpleMath::Vector3 p = m_camera->GetPosition();
		DirectX::SimpleMath::Vector3 d = m_camera->GetForwardDirection();
		std::wstringstream wss;

		/*wss << L"Position - x: " << p.x
			<< L", y: " << p.y
			<< L", z: " << p.z
			<< '\n'
			<< L"Directon - x: " << d.x
			<< L", y: " << d.y
			<< L", z: " << d.z;*/
		wss << L"FPS : " << (int)m_timer.GetFrameRate();

		if (msaaMode) {
			RenderFonts(wss.str(), m_msaaResourceDescriptors, m_msaaSpriteBatch, m_msaaFont, m_commandList);

		}
		else {
			RenderFonts(wss.str(), m_resourceDescriptors, m_spriteBatch, m_font, m_commandList);
		}
		//RenderFonts(wss.str(), m_resourceDescriptors, m_spriteBatch, m_font, m_commandList);

	}


	if (msaaMode) {
		for (UINT i = 0; i < geometryPassRtvNum; i++)
		{
			ResolveSubresource(m_commandList, m_geometryPassResources[i].Get(), m_geometryPassMsaaResources[i].Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				m_geometryPassFormat);
		}
		ResolveSubresource(m_commandList, m_hdrDepthStencilBuffer.Get(), m_msaaDepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
	}
	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* plists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());
}


void Renderer::D3D12PassApp::RenderGUI(float& deltaTime)
{
	D3D12App::RenderGUI(deltaTime);
}

void Renderer::D3D12PassApp::RenderStaticMeshes(float& deltaTime) {
	for (int i = 0; i < m_staticMeshes.size(); ++i) {
		CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_textureHeap->GetGPUDescriptorHandleForHeapStart());

		for (int j = 0; j < (int)m_staticMeshes[i]->meshCount; j++)
		{
			if ((int)m_textureMap.count(m_staticMeshes[i]->GetTexturePath(j)) > 0) {
				handle.Offset(m_textureMap[m_staticMeshes[i]->GetTexturePath(j)], m_csuHeapSize);
			}
			else {
				handle.Offset(m_textureMap[L"zzzdefaultAlbedo.dds"], m_csuHeapSize);
			}
			m_commandList->SetGraphicsRootDescriptorTable(0, handle);
			m_staticMeshes[i]->Render(deltaTime, m_commandList, true, (int)j);
		}
	}
}

void Renderer::D3D12PassApp::RenderLightMeshes(float& deltaTime) {
	for (int i = 0; i < m_lightMeshes.size(); ++i) {
		CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_textureHeap->GetGPUDescriptorHandleForHeapStart());

		for (int j = 0; j < (int)m_lightMeshes[i]->meshCount; j++)
		{
			if ((int)m_textureMap.count(m_lightMeshes[i]->GetTexturePath(j)) > 0) {
				handle.Offset(m_textureMap[m_lightMeshes[i]->GetTexturePath(j)], m_csuHeapSize);
			}
			else {
				handle.Offset(m_textureMap[L"zzzdefaultAlbedo.dds"], m_csuHeapSize);
			}
			m_commandList->SetGraphicsRootDescriptorTable(0, handle);
			m_lightMeshes[i]->Render(deltaTime, m_commandList, true, (int)j);
		}
	}
}


void Renderer::D3D12PassApp::RenderCharacter(float& deltaTime) {

	for (int j = 0; j < (int)mCharacter->GetMeshCount(); j++)
	{

		CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_textureHeap->GetGPUDescriptorHandleForHeapStart());
		if ((int)m_textureMap.count(mCharacter->GetTexturePath(j)) > 0) {
			handle.Offset(m_textureMap[mCharacter->GetTexturePath(j)], m_csuHeapSize);
		}
		else {
			handle.Offset(m_textureMap[L"zzzdefaultAlbedo.dds"], m_csuHeapSize);
		}
		m_commandList->SetGraphicsRootDescriptorTable(0, handle);
		mCharacter->Render(deltaTime, m_commandList, j);


	}
}

void Renderer::D3D12PassApp::RenderPlayers(float& deltaTime) {
	for (int i = 0; i < mPlayers.size(); ++i) {
		for (int j = 0; j < (int)mPlayers[i]->meshCount; j++)
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_textureHeap->GetGPUDescriptorHandleForHeapStart());
			if ((int)m_textureMap.count(mPlayers[i]->GetTexturePath(j)) > 0) {
				handle.Offset(m_textureMap[mPlayers[i]->GetTexturePath(j)], m_csuHeapSize);
			}
			else {
				handle.Offset(m_textureMap[L"zzzdefaultAlbedo.dds"], m_csuHeapSize);
			}
			m_commandList->SetGraphicsRootDescriptorTable(0, handle);
			mPlayers[i]->Render(deltaTime, m_commandList, true, j);
		}

	}
}