#include "D3D12RaytracingApp.h"
#include "Raytracing.hlsl.h"

using namespace std;

const wchar_t* c_hitGroupName = L"MyHitGroup";
const wchar_t* c_raygenShaderName = L"MyRaygenShader";
const wchar_t* c_closestHitShaderName = L"MyClosestHitShader";
const wchar_t* c_missShaderName = L"MyMissShader";

Renderer::D3D12RayTracingApp::D3D12RayTracingApp(const int& width, const int& height)
	: D3D12App(width, height)
{
	bUseTextureApp = false;
	bUseCubeMapApp = false;
	bUseDefaultSceneApp = false;
	bUseGUI = false;
	/*m_camera->SetPositionAndDirection(DirectX::SimpleMath::Vector3(0, 0, 0),
		DirectX::SimpleMath::Vector3(0, 0, 1));*/
}

bool Renderer::D3D12RayTracingApp::Initialize()
{
	if (!D3D12App::Initialize())
		return false;

	
	// Direct RayTracing Command List 생성
	ThrowIfFailed(m_commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)));
	m_commandAllocator->Reset();
	m_dxrCommandList->Reset(m_commandAllocator.Get(), nullptr);

	/*CD3DX12_ROOT_PARAMETER1 rootParameters[1];
	rootParameters[0].InitAsConstants(SizeOfInUint32(m_rayGenCB), 0, 0);
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters,0,nullptr, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&localRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), error.GetAddressOf()));
	m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_raytracingLocalSignature));*/

	CreateConstantBuffer();
	InitRayTracingScene();
	CreateStateObjects();
	CreateShaderTable();


	ThrowIfFailed(m_dxrCommandList->Close());
	ID3D12CommandList* pCmdLists[] = { m_dxrCommandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(pCmdLists), pCmdLists);

	FlushCommandQueue();


	return true;
}

bool Renderer::D3D12RayTracingApp::InitGUI()
{
	if (!D3D12App::InitGUI())
		return false;
	return true;
}
void Renderer::D3D12RayTracingApp::CreateStateObjects()
{
	CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };
	auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
	lib->SetDXILLibrary(&libdxil);
	{
		lib->DefineExport(rayGenerationShaderName);
		lib->DefineExport(closestHitShaderName);
		lib->DefineExport(missShaderName);
	}

	// Triangle hit group
	auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
	hitGroup->SetClosestHitShaderImport(closestHitShaderName);
	hitGroup->SetHitGroupExport(hitGroupName);
	hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

	// Shader config
	auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	UINT payloadSize = 4 * sizeof(float);   // float4 color
	UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
	shaderConfig->Config(payloadSize, attributeSize);

	const WCHAR* shaderPayloadExports[] = {
			rayGenerationShaderName,
			hitGroupName,
			missShaderName
	};
	auto association = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
	association->AddExports(shaderPayloadExports);
	association->SetSubobjectToAssociate(*shaderConfig);

	//// Local root signature and shader association
	//auto localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
	//localRootSignature->SetRootSignature(m_raytracingLocalSignature.Get());

	//// Shader association
	//auto rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
	//rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
	//rootSignatureAssociation->AddExport(rayGenerationShaderName);

	// Global root signature
	auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRootSignature->SetRootSignature(raytracingGlobalSignature.Get());

	auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
	UINT maxRecursionDepth = 1; // ~ primary rays only. 
	pipelineConfig->Config(maxRecursionDepth);

	// Create the state object.
	ThrowIfFailed(m_device->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_rtpso)));
	/*
	D3D12_EXPORT_DESC raygenExport{
		rayGenerationShaderName,
			nullptr,
			D3D12_EXPORT_FLAG_NONE
	};

	D3D12_DXIL_LIBRARY_DESC raygeneration = {};
	raygeneration.DXILLibrary.BytecodeLength = sizeof(g_pRaytracing);
	raygeneration.DXILLibrary.pShaderBytecode = g_pRaytracing;
	raygeneration.NumExports = 1;
	raygeneration.pExports = &raygenExport;

	D3D12_STATE_SUBOBJECT rayGenshaders
	{
		D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY,
		&raygeneration
	};

	D3D12_EXPORT_DESC hitAndMissExport[2]
	{
		{
			closestHitShaderName,
			nullptr,
			D3D12_EXPORT_FLAG_NONE
		},
		{
			missShaderName,
			nullptr,
			D3D12_EXPORT_FLAG_NONE
		}
	};

	D3D12_DXIL_LIBRARY_DESC hitAndMiss = {};
	hitAndMiss.DXILLibrary.BytecodeLength = sizeof(g_pRaytracing);
	hitAndMiss.DXILLibrary.pShaderBytecode = g_pRaytracing;
	hitAndMiss.NumExports = 2;
	hitAndMiss.pExports = hitAndMissExport;

	D3D12_STATE_SUBOBJECT hitAndMissShaders
	{
		D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY,
		&hitAndMiss
	};

	D3D12_HIT_GROUP_DESC hitGroup
	{
		hitGroupName,
		D3D12_HIT_GROUP_TYPE_TRIANGLES,
		nullptr,
		closestHitShaderName,
		nullptr
	};
	D3D12_STATE_SUBOBJECT hitGroupObject
	{
		D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP,
		&hitGroup
	};

	D3D12_RAYTRACING_SHADER_CONFIG shrdConfig
	{
		sizeof(float[4]),
		sizeof(float[2])
	};
	config =
	{
		D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG,
		&shrdConfig
	};
	D3D12_STATE_SUBOBJECT globalRootSignature
	{
		D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE,
		raytracingGlobalSignature.GetAddressOf()
	};
	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig
	{
		1
	};
	D3D12_STATE_SUBOBJECT pipeline
	{
		D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG,
		&pipelineConfig
	};

	subObjects.push_back(rayGenshaders);
	subObjects.push_back(hitAndMissShaders);
	subObjects.push_back(hitGroupObject);
	subObjects.push_back(config);
	subObjects.push_back(globalRootSignature);
	subObjects.push_back(pipeline);

	const WCHAR* shaderPayloadExports[] = {
		rayGenerationShaderName,
		hitGroupName,
		missShaderName
	};

	D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION assocDesc = {};
	assocDesc.NumExports = 3;
	assocDesc.pExports = shaderPayloadExports;
	assocDesc.pSubobjectToAssociate = &(subObjects[3]);

	D3D12_STATE_SUBOBJECT association
	{
		D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION,
		& assocDesc
	};

	subObjects.push_back(association);

	rtStateObject =
	{
		D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
		(UINT)subObjects.size(),
		subObjects.data()
	};

	m_device->CreateStateObject(&rtStateObject, IID_PPV_ARGS(&m_rtpso));*/

}
void Renderer::D3D12RayTracingApp::CreateShaderTable()
{

	ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
	ThrowIfFailed(m_rtpso.As(&stateObjectProperties));
	UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	ShaderTable parameter;
	
	UINT bufferSize = (UINT)(sizeof(parameter));

	Renderer::Utility::CreateBuffer(m_device, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, bufferSize,
		D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, m_rgsTable);

	
	Renderer::Utility::CreateBuffer(m_device, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, bufferSize,
		D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, m_missTable);

	Renderer::Utility::CreateBuffer(m_device, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, bufferSize,
		D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, m_hitGroupsTable);

	CD3DX12_RANGE readRange(0, 0);
	memcpy(parameter.m_mappedShaderRecords, stateObjectProperties->GetShaderIdentifier(rayGenerationShaderName), shaderIdentifierSize);
	ThrowIfFailed(m_rgsTable->Map(0, &readRange, reinterpret_cast<void**>(&pRgsData)));
	memcpy(pRgsData, parameter.m_mappedShaderRecords, shaderIdentifierSize);
	/*pRgsData += 32;
	memcpy(pRgsData, &m_rayGenCB, sizeof(RayGenConstantBufferData));*/

	memcpy(parameter.m_mappedShaderRecords, stateObjectProperties->GetShaderIdentifier(hitGroupName), shaderIdentifierSize);
	ThrowIfFailed(m_hitGroupsTable->Map(0, &readRange, reinterpret_cast<void**>(&pHitgroupsData)));
	memcpy(pHitgroupsData, parameter.m_mappedShaderRecords, shaderIdentifierSize);


	memcpy(parameter.m_mappedShaderRecords, stateObjectProperties->GetShaderIdentifier(missShaderName), shaderIdentifierSize);
	ThrowIfFailed(m_missTable->Map(0, &readRange, reinterpret_cast<void**>(&pMissData)));
	memcpy(pMissData, parameter.m_mappedShaderRecords, shaderIdentifierSize);
}

void Renderer::D3D12RayTracingApp::CreateConstantBuffer()
{
	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(RaytraingSceneConstantData)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_sceneCB));

	D3D12_RANGE range(0, 0);
	ThrowIfFailed(m_sceneCB->Map(0, &range, reinterpret_cast<void**>(&pSceneBegin)));
	memcpy(pSceneBegin, &m_sceneCBData, sizeof(RaytraingSceneConstantData));
}

bool Renderer::D3D12RayTracingApp::InitDirectX()
{
	if (!D3D12App::InitDirectX())
		return false;
	return true;
}

void Renderer::D3D12RayTracingApp::OnResize()
{
	D3D12App::OnResize();
}

void Renderer::D3D12RayTracingApp::InitRayTracingScene()
{
	using DirectX::SimpleMath::Vector3;
	std::shared_ptr<Core::StaticMesh> box = std::make_shared<Core::StaticMesh>();
	box->Initialize(GeometryGenerator::SimpleBox(1.f), m_device, m_commandList, Vector3(0,0,2));
	box->BuildAccelerationStructures<SimpleVertex>(m_device, m_dxrCommandList);
	m_staticMeshes.push_back(box);
}

void Renderer::D3D12RayTracingApp::BuildAccelerationStructures()
{

}

void Renderer::D3D12RayTracingApp::Update(float& deltaTime)
{
	m_inputHandler->ExicuteCommand(m_camera.get(), deltaTime, bIsFPSMode);

	m_sceneCBData.cameraPosition = m_camera->GetPosition();
	m_sceneCBData.view = m_camera->GetViewMatrix() * m_camera->GetProjMatrix() * DirectX::XMMatrixTranslation(0,0,m_camera->d);
	m_sceneCBData.view = m_sceneCBData.view.Invert();
	m_sceneCBData.view = m_sceneCBData.view.Transpose();
	memcpy(pSceneBegin, &m_sceneCBData, sizeof(RaytraingSceneConstantData));

	for (auto& mesh : m_staticMeshes) {
		mesh->Update(deltaTime);
	}
}

void Renderer::D3D12RayTracingApp::UpdateGUI(float& deltaTime)
{
	//D3D12App::UpdateGUI(deltaTime);

}

void Renderer::D3D12RayTracingApp::Render(float& deltaTime)
{

	RaytracingPass(deltaTime);
	CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());
}
void Renderer::D3D12RayTracingApp::RaytracingPass(float& deltaTime)
{
	auto desc = HDRRenderTargetBuffer()->GetDesc();
	UINT width = (UINT)desc.Width;
	UINT height = desc.Height;

	m_commandAllocator->Reset();
	m_dxrCommandList->Reset(m_commandAllocator.Get(), nullptr);

	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(0, 0, 255), raytracingPass);

	FLOAT clearColor[4] = { 0.f,0.f,0.f,0.f };
	m_dxrCommandList->ClearRenderTargetView(HDRRendertargetView(), clearColor, 0, nullptr);
	m_dxrCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			HDRRenderTargetBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		));

	m_dxrCommandList->SetComputeRootSignature(raytracingGlobalSignature.Get());
	m_dxrCommandList->SetPipelineState1(m_rtpso.Get());
	m_dxrCommandList->SetDescriptorHeaps(1, m_hdrUavHeap.GetAddressOf());
	m_dxrCommandList->SetComputeRootDescriptorTable(0, m_hdrUavHeap->GetGPUDescriptorHandleForHeapStart());
	m_dxrCommandList->SetComputeRootShaderResourceView(1, m_staticMeshes[0]->GetTlas());
	m_dxrCommandList->SetComputeRootConstantBufferView(2, m_sceneCB->GetGPUVirtualAddress());

	D3D12_DISPATCH_RAYS_DESC dispactchRays = {};

	dispactchRays.RayGenerationShaderRecord.StartAddress = m_rgsTable->GetGPUVirtualAddress();
	dispactchRays.RayGenerationShaderRecord.SizeInBytes = sizeof(ShaderTable);

	dispactchRays.HitGroupTable.StartAddress = m_hitGroupsTable->GetGPUVirtualAddress();
	dispactchRays.HitGroupTable.SizeInBytes = sizeof(ShaderTable[1]);
	dispactchRays.HitGroupTable.StrideInBytes = sizeof(ShaderTable);

	dispactchRays.MissShaderTable.StartAddress = m_missTable->GetGPUVirtualAddress();
	dispactchRays.MissShaderTable.SizeInBytes = sizeof(ShaderTable[1]);
	dispactchRays.MissShaderTable.StrideInBytes = sizeof(ShaderTable);

	dispactchRays.Width = width;
	dispactchRays.Height = height;
	dispactchRays.Depth = 1;
	m_dxrCommandList->DispatchRays(&dispactchRays);

	m_dxrCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			HDRRenderTargetBuffer(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_RENDER_TARGET

		));

	ThrowIfFailed(m_dxrCommandList->Close());
	ID3D12CommandList* pCmdLists[] = { m_dxrCommandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(pCmdLists), pCmdLists);

	FlushCommandQueue();
	PIXEndEvent(m_commandQueue.Get());
}
void Renderer::D3D12RayTracingApp::RenderCubeMap(float& deltaTime)
{
}

void Renderer::D3D12RayTracingApp::RenderGUI(float& deltaTime)
{
	D3D12App::RenderGUI(deltaTime);
}
