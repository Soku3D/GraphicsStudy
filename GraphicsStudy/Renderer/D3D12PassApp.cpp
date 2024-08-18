#include "D3D12PassApp.h"
#include "Renderer.h"
#include <DirectXTexEXR.h>


Renderer::D3D12PassApp::D3D12PassApp(const int& width, const int& height)
	:D3D12App(width, height)
{
	bUseGUI = true;
	bRenderCubeMap = true;
	bRenderMeshes = true;
	bRenderFbx = false;
	bRenderNormal = false;
}

bool Renderer::D3D12PassApp::Initialize()
{
	if (!D3D12App::Initialize())
		return false;
	gui_lightPos = DirectX::SimpleMath::Vector3(-1.f, 1.f, 0.f);
	InitConstantBuffers();

	return true;
}

void Renderer::D3D12PassApp::InitConstantBuffers() {
	psConstantData = new CSConstantData();
	psConstantData->time = 0;

	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(CSConstantData)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_csBuffer));

	D3D12_RANGE range(0, 0);
	ThrowIfFailed(m_csBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCbufferBegin)));
	memcpy(m_pCbufferBegin, psConstantData, sizeof(CSConstantData));


	m_pCubeMapConstantData = new CubeMapConstantData();
	m_pCubeMapConstantData->expose = gui_cubeMapExpose;
	m_pCubeMapConstantData->lodLevel = gui_cubeMapLod;

	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(CubeMapConstantData)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_cubeMapConstantBuffer));

	ThrowIfFailed(m_cubeMapConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&m_pCubeMapCbufferBegin)));
	memcpy(m_pCubeMapCbufferBegin, m_pCubeMapConstantData, sizeof(CubeMapConstantData));

}

void Renderer::D3D12PassApp::CreateVertexAndIndexBuffer()
{
	using DirectX::SimpleMath::Vector3;
	using namespace Core;

	/*std::shared_ptr<Core::StaticMesh> sphere = std::make_shared<Core::StaticMesh>();
	sphere->Initialize(GeometryGenerator::PbrSphere(1.f, 100, 100, L"worn-painted-metal_albedo.png",
		2.f, 2.f),
		m_device, m_commandList, Vector3(0.f, 0.f, 0.f), 
		Material(1.f, 1.f, 1.f, 1.f),
		true, true,	true, true,	true);
	m_staticMeshes.push_back(sphere);*/

	std::shared_ptr<StaticMesh> plane = std::make_shared<StaticMesh>();
	plane->Initialize(GeometryGenerator::PbrBox(10, 1, 10, L"worn-painted-metal_albedo.png"), m_device, m_commandList, Vector3(0.f, -1.f, 0.f),
		Material(1.f, 1.f, 1.f, 1.f),
		true, true, true, true, true);
	m_staticMeshes.push_back(plane);

	auto [box_destruction, box_destruction_animation] = GeometryGenerator::ReadFromFile("box_destruction.fbx", true);
	std::shared_ptr<Animation::FBX> wallDistructionFbx = std::make_shared<Animation::FBX>();
	wallDistructionFbx->Initialize(box_destruction, box_destruction_animation, m_device, m_commandList, true, 1.f);
	m_fbxList.push_back(wallDistructionFbx);

	m_cubeMap = std::make_shared<Core::StaticMesh>();
	m_cubeMap->Initialize(GeometryGenerator::SimpleCubeMapBox(100.f), m_device, m_commandList);
	m_cubeMap->SetTexturePath(std::wstring(L"Outdoor")+ L"EnvHDR.dds");

	m_screenMesh = std::make_shared<Core::StaticMesh>();
	m_screenMesh->Initialize(GeometryGenerator::Rectangle(2.f, L""), m_device, m_commandList);
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
	m_inputHandler->ExicuteCommand(m_camera.get(), deltaTime, bIsFPSMode);

	{
		m_passConstantData->ViewMat = m_camera->GetViewMatrix();
		m_passConstantData->ProjMat = m_camera->GetProjMatrix();

		m_passConstantData->ViewMat = m_passConstantData->ViewMat.Transpose();
		m_passConstantData->ProjMat = m_passConstantData->ProjMat.Transpose();
		memcpy(m_pCbvDataBegin, m_passConstantData, sizeof(GlobalVertexConstantData));
	}

	// LightPass ConstantBuffer
	{
		m_ligthPassConstantData->eyePos = m_camera->GetPosition();
		m_ligthPassConstantData->lod = gui_lod;
		m_ligthPassConstantData->light[0].position = gui_lightPos;

		m_ligthPassConstantData->ao = gui_ao;
		m_ligthPassConstantData->metallic = gui_metallic;
		m_ligthPassConstantData->roughness = gui_roughness;
		m_ligthPassConstantData->expose = gui_cubeMapExpose;

		memcpy(m_pLPCDataBegin, m_ligthPassConstantData, sizeof(LightPassConstantData));
	}


	for (auto& mesh : m_staticMeshes) {
		mesh->Update(deltaTime);
	}
	for (auto& fbx : m_fbxList) {
		fbx->Update(deltaTime);
	}

	// ComputeShader(PostProcessing)
	psConstantData->time = (float)m_timer.GetElapsedTime();
	memcpy(m_pCbufferBegin, psConstantData, sizeof(CSConstantData));

	// CubeMap ConstantBuffer
	m_pCubeMapConstantData->expose = gui_cubeMapExpose;
	m_pCubeMapConstantData->lodLevel = gui_cubeMapLod;
	memcpy(m_pCubeMapCbufferBegin, m_pCubeMapConstantData, sizeof(CubeMapConstantData));
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
	ImGui::SliderFloat("AO", &gui_ao, 0.f, 1.f);
	ImGui::SliderFloat("Metalic", &gui_metallic, 0.f, 1.f);
	ImGui::SliderFloat("Roughness", &gui_roughness, 0.f, 1.f);


	ImGui::SliderFloat("LOD", &gui_lod, 0.f, 10.f);
	ImGui::SliderFloat("CubeMap LodLevel", &gui_cubeMapLod, 0.f, 10.f);
	ImGui::SliderFloat("CubeMap Expose", &gui_cubeMapExpose, 0.f, 10.f);
	ImGui::SliderFloat3("Light Position", (float*)(&gui_lightPos), -10.f, 10.f);


	ImGui::Checkbox("Render Normal", &bRenderNormal);
	ImGui::Checkbox("Render CubeMap", &bRenderCubeMap);
	ImGui::Checkbox("Render FBX", &bRenderFbx);
}

void Renderer::D3D12PassApp::Render(float& deltaTime)
{
	GeometryPass(deltaTime);
	LightPass(deltaTime);
	DrawNormalPass(deltaTime);
	RenderCubeMap(deltaTime);
	CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());

	//PostProcessing(deltaTime);
	//CopyResourceToSwapChain(deltaTime);
}

void Renderer::D3D12PassApp::GeometryPass(float& deltaTime) {

	auto& pso = passPsoLists[currRenderMode + "GeometryPass"];

	if (currRenderMode == "Msaa") {
		msaaMode = true;
	}
	else {
		msaaMode = false;
	}

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	{
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
		//m_commandList->ClearRenderTargetView(HDRRendertargetView(), clearColor, 0, nullptr);

		m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->RSSetScissorRects(1, &m_scissorRect);
		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());

		// View Proj Matrix Constant Buffer 
		m_commandList->SetGraphicsRootConstantBufferView(2, m_passConstantBuffer->GetGPUVirtualAddress());
		//m_commandList->SetGraphicsRootConstantBufferView(3, m_ligthPassConstantBuffer->GetGPUVirtualAddress());

		// Texture SRV Heap 
		ID3D12DescriptorHeap* ppSrvHeaps[] = { m_textureHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppSrvHeaps), ppSrvHeaps);
		if (bRenderMeshes) {
			for (int i = 0; i < m_staticMeshes.size(); ++i) {
				CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_textureHeap->GetGPUDescriptorHandleForHeapStart());
				
				if (m_textureMap.count(m_staticMeshes[i]->GetTexturePath()) > 0) {
					handle.Offset(m_textureMap[m_staticMeshes[i]->GetTexturePath()], m_csuHeapSize);
				}
				else {
					handle.Offset(m_textureMap[L"zzzdefault.png"], m_csuHeapSize);
				}
				m_commandList->SetGraphicsRootDescriptorTable(0, handle);
				//m_commandList->SetGraphicsRootDescriptorTable(1, normalHandle);
				m_staticMeshes[i]->Render(deltaTime, m_commandList, true);
			}

		}
		if (bRenderFbx) {
			for (int i = 0; i < m_fbxList.size(); ++i) {
				m_fbxList[i]->Render(deltaTime, m_commandList, true, m_textureHeap, m_textureMap, m_csuHeapSize);
			}
		}
		
	}

	if (msaaMode) {
		for (UINT i = 0; i < geometryPassRtvNum; i++)
		{
			ResolveSubresource(m_commandList, m_geometryPassResources[i].Get(), m_geometryPassMsaaResources[i].Get());
		}
		ResolveSubresource(m_commandList, m_hdrDepthStencilBuffer.Get(), m_msaaDepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
	}

	//CopyResource(m_commandList, HDRRenderTargetBuffer(), m_geometryPassResources[GeometryPassType::albedoColor].Get(),
	//	D3D12_RESOURCE_STATE_RENDER_TARGET,
	//	D3D12_RESOURCE_STATE_RENDER_TARGET);

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* plists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

	FlushCommandQueue();

}

void Renderer::D3D12PassApp::LightPass(float& deltaTime) {
	auto& pso = passPsoLists["LightPass"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
	{
		FLOAT clearColor[4] = { 0.f,0.f,0.f,0.f };
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
		CD3DX12_GPU_DESCRIPTOR_HANDLE cubeMapHandle(m_geometryPassSrvHeap->GetGPUDescriptorHandleForHeapStart(), geometryPassRtvNum + cubeMapOffset, m_csuHeapSize);
		m_commandList->SetGraphicsRootDescriptorTable(1, cubeMapHandle);
		m_commandList->SetGraphicsRootConstantBufferView(2, m_ligthPassConstantBuffer->GetGPUVirtualAddress());
		m_screenMesh->Render(deltaTime, m_commandList, false);
	}
	

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* plists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

	FlushCommandQueue();
}


void Renderer::D3D12PassApp::DrawNormalPass(float& deltaTime) {

	if (bRenderNormal) {
		auto& pso = passPsoLists["NormalPass"];


		ThrowIfFailed(m_commandAllocator->Reset());
		ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));
		{
			m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());

			m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
			m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());
			m_commandList->RSSetScissorRects(1, &m_scissorRect);
			m_commandList->RSSetViewports(1, &m_viewport);

			m_commandList->SetGraphicsRootConstantBufferView(1, m_passConstantBuffer->GetGPUVirtualAddress());

			if (bRenderMeshes) {
				for (int i = 0; i < m_staticMeshes.size(); ++i) {
					m_staticMeshes[i]->RenderNormal(deltaTime, m_commandList, true);
				}
			}
		}
		ThrowIfFailed(m_commandList->Close());

		ID3D12CommandList* plists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(plists), plists);

		FlushCommandQueue();
	}

}

void Renderer::D3D12PassApp::RenderCubeMap(float& deltaTime)
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
			/*if (msaaMode) {

				m_commandList->OMSetRenderTargets(1, &MsaaRenderTargetView(), true, &MsaaDepthStencilView());
			}
			else {

				m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());
			}*/
			m_commandList->OMSetRenderTargets(1, &HDRRendertargetView(), true, &HDRDepthStencilView());

			m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_commandList->SetGraphicsRootSignature(pso.GetRootSignature());
			m_commandList->RSSetScissorRects(1, &m_scissorRect);
			m_commandList->RSSetViewports(1, &m_viewport);

			// View Proj Matrix Constant Buffer 
			m_commandList->SetGraphicsRootConstantBufferView(1, m_passConstantBuffer->GetGPUVirtualAddress());

			//CubeMap Expose & LodLevel
			m_commandList->SetGraphicsRootConstantBufferView(2, m_cubeMapConstantBuffer->GetGPUVirtualAddress());

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
	}
	//Render Font GUI
	//{
	//	int time = (int)m_timer.GetElapsedTime();

	//	if (msaaMode) {
	//		//RenderFonts(std::to_wstring(time), m_msaaResourceDescriptors, m_msaaSpriteBatch, m_msaaFont, m_commandList);
	//		//ResolveSubresource(m_commandList, HDRRenderTargetBuffer(), MsaaRenderTargetBuffer());
	//	}
	//	else {
	//		//RenderFonts(std::to_wstring(time), m_resourceDescriptors, m_spriteBatch, m_font, m_commandList);
	//	}
	//}
	
}


void Renderer::D3D12PassApp::PostProcessing(float& deltaTime) {

	//D3D12App::Render(deltaTime);

	auto& pso = computePsoList["Compute"];

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

	{
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

		m_commandList->SetComputeRootConstantBufferView(1, m_csBuffer->GetGPUVirtualAddress());
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
	}

}

void Renderer::D3D12PassApp::RenderGUI(float& deltaTime)
{
	D3D12App::RenderGUI(deltaTime);
}



void Renderer::D3D12PassApp::CopyResourceToSwapChain(float& deltaTime)
{
	auto& pso = utilityPsoLists["Copy"];
	{
		ThrowIfFailed(m_commandAllocator->Reset());
		ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPipelineStateObject()));

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
}