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
	//bUseTextureApp = false;
	//bUseCubeMapApp = false;
	bUseDefaultSceneApp = false;
	bUseGUI = false;
	m_appName = "RaytracingApp";
	/*m_camera->SetPositionAndDirection(DirectX::SimpleMath::Vector3(0, 0, 0),
		DirectX::SimpleMath::Vector3(0, 0, 1));*/
		//m_camera->SetPositionAndDirection(DirectX::SimpleMath::Vector3(0, 0, 0),
		//	DirectX::SimpleMath::Vector3(0, 0, 1));
}

Renderer::D3D12RayTracingApp::~D3D12RayTracingApp()
{
	delete mCharacterMesh;
	delete hitShaderTable;
	std::cout << "~D3D12RayTracingApp" << std::endl;
}

bool Renderer::D3D12RayTracingApp::Initialize()
{
	if (!D3D12App::Initialize())
		return false;

	// Direct RayTracing Command List 생성
	ThrowIfFailed(m_commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)));
	m_commandAllocator->Reset();
	m_dxrCommandList->Reset(m_commandAllocator.Get(), nullptr);

	CD3DX12_DESCRIPTOR_RANGE1 rangeTable[1];
	rangeTable[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 1);
	//rangeTable[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);

	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(ARRAYSIZE(rangeTable), rangeTable);
	rootParameters[1].InitAsConstants(SizeOfInUint32(PrimitiveConstantBuffer), 1, 0);
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&localRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), error.GetAddressOf()));
	m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_raytracingLocalSignature));

	m_screenMesh = std::make_shared<Core::StaticMesh>();
	m_screenMesh->Initialize(GeometryGenerator::Rectangle(2.f, L""), m_device, m_commandList);

	CreateConstantBuffer();
	InitRayTracingScene();
	InitializeViews();
	BuildAccelerationStructures();

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
	for (size_t i = 0; i < hitGroupNames.size(); i++)
	{
		auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hitGroup->SetClosestHitShaderImport(closestHitShaderName);
		hitGroup->SetHitGroupExport(hitGroupNames[i]);
		hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
	}
	for (size_t i = 0; i < characterHitGroupNames.size(); i++)
	{
		auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hitGroup->SetClosestHitShaderImport(closestHitShaderName);
		hitGroup->SetHitGroupExport(characterHitGroupNames[i]);
		hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
	}

	// Shader config
	auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	UINT payloadSize = sizeof(RayPayload);   // float4 color
	UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
	shaderConfig->Config(payloadSize, attributeSize);

	const WCHAR* shaderPayloadExports[] = {
			rayGenerationShaderName,
			missShaderName
	};
	auto association = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
	association->AddExports(shaderPayloadExports);
	for (size_t i = 0; i < hitGroupNames.size(); i++)
	{
		association->AddExport(hitGroupNames[i]);
	}
	for (size_t i = 0; i < characterHitGroupNames.size(); i++)
	{
		association->AddExport(characterHitGroupNames[i]);
	}
	association->SetSubobjectToAssociate(*shaderConfig);

	//// Local root signature and shader association
	auto localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
	localRootSignature->SetRootSignature(m_raytracingLocalSignature.Get());

	//// Shader association
	auto rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
	rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
	for (size_t i = 0; i < hitGroupNames.size(); i++)
	{
		rootSignatureAssociation->AddExport(hitGroupNames[i]);
	}
	for (size_t i = 0; i < characterHitGroupNames.size(); i++)
	{
		rootSignatureAssociation->AddExport(characterHitGroupNames[i]);
	}

	// Global root signature
	auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRootSignature->SetRootSignature(raytracingGlobalSignature.Get());

	auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
	UINT maxRecursionDepth = MAX_RAY_RECURSION_DEPTH;
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

	void* rayGenShaderID;
	void* missShaderIDs;
	std::vector<void*> hitGroupShaderIDs(hitGroupNames.size());
	std::vector<void*> characterHitGroupShaderIDs(characterHitGroupNames.size());

	rayGenShaderID = stateObjectProperties->GetShaderIdentifier(rayGenerationShaderName);
	missShaderIDs = stateObjectProperties->GetShaderIdentifier(missShaderName);

	for (size_t i = 0; i < hitGroupNames.size(); i++)
	{
		hitGroupShaderIDs[i] = stateObjectProperties->GetShaderIdentifier(hitGroupNames[i]);
	}
	for (size_t i = 0; i < characterHitGroupNames.size(); i++)
	{
		characterHitGroupShaderIDs[i] = stateObjectProperties->GetShaderIdentifier(characterHitGroupNames[i]);
	}

	ShaderTable raygenShaderTable(m_device, 1, shaderIdentifierSize);
	raygenShaderTable.push_back(ShaderRecord(rayGenShaderID, shaderIdentifierSize));
	m_rgsTable = raygenShaderTable.GetResource();

	ShaderTable missShaderTable(m_device, 1, shaderIdentifierSize);
	missShaderTable.push_back(ShaderRecord(missShaderIDs, shaderIdentifierSize));
	m_missTable = missShaderTable.GetResource();

	UINT numShaderRecords = (UINT)m_staticMeshes.size();
	UINT shaderRecordSize = 128;

	hitShaderTable = new ShaderTable(m_device, numShaderRecords, shaderRecordSize);

	for (size_t i = 0; i < hitGroupNames.size(); i++)
	{
		//hitShaderTable.push_back(ShaderRecord(hitGroupShaderIDs[i], shaderIdentifierSize, &(m_textureHeap->GetGPUDescriptorHandleForHeapStart().ptr), sizeof(m_textureHeap->GetGPUDescriptorHandleForHeapStart().ptr)));
		//hitShaderTable.push_back(ShaderRecord(hitGroupShaderIDs[i], 0, &(m_staticMeshes[i]->m_primitiveConstantData), 32));
		memcpy(hitShaderTable->m_mappedShaderRecoreds, hitGroupShaderIDs[i], shaderIdentifierSize);
		hitShaderTable->m_mappedShaderRecoreds += 32;

		// raytracing heap에 모든 텍스쳐가 view가 들어있다
		CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_raytracingHeaps[i]->GetGPUDescriptorHandleForHeapStart());
		//CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_staticMeshes[i]->GetIndexGpuHandle());
		memcpy(hitShaderTable->m_mappedShaderRecoreds, &handle.ptr, sizeof(handle.ptr));
		hitShaderTable->m_mappedShaderRecoreds += 8;
		memcpy(hitShaderTable->m_mappedShaderRecoreds, &(m_staticMeshes[i]->m_primitiveConstantData), sizeof(PrimitiveConstantBuffer));
		hitShaderTable->m_mappedShaderRecoreds += 88;
	}
	for (size_t i = 0; i < characterHitGroupNames.size(); i++)
	{
		//hitShaderTable.push_back(ShaderRecord(hitGroupShaderIDs[i], shaderIdentifierSize, &(m_textureHeap->GetGPUDescriptorHandleForHeapStart().ptr), sizeof(m_textureHeap->GetGPUDescriptorHandleForHeapStart().ptr)));
		//hitShaderTable.push_back(ShaderRecord(hitGroupShaderIDs[i], 0, &(m_staticMeshes[i]->m_primitiveConstantData), 32));
		memcpy(hitShaderTable->m_mappedShaderRecoreds, characterHitGroupShaderIDs[i], shaderIdentifierSize);
		hitShaderTable->m_mappedShaderRecoreds += 32;

		// raytracing heap에 모든 텍스쳐가 view가 들어있다
		CD3DX12_GPU_DESCRIPTOR_HANDLE handle(mChracterRaytracingHeaps[i]->GetGPUDescriptorHandleForHeapStart());
		//CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_staticMeshes[i]->GetIndexGpuHandle());
		memcpy(hitShaderTable->m_mappedShaderRecoreds, &handle.ptr, sizeof(handle.ptr));
		hitShaderTable->m_mappedShaderRecoreds += 8;
		PrimitiveConstantBuffer data = mCharacter->GetprimitiveConstantData();
		memcpy(hitShaderTable->m_mappedShaderRecoreds, &data, sizeof(PrimitiveConstantBuffer));
		hitShaderTable->m_mappedShaderRecoreds += 88;
	}
}

void Renderer::D3D12RayTracingApp::InitializeViews() {
	m_raytracingHeaps.resize(m_staticMeshes.size());
	mChracterRaytracingHeaps.resize(mCharacter->GetMeshCount());

	UINT indicesBufferCount = 2;
	UINT textureBufferCount = 6;
	//UINT cubeMapTextureBufferCount = 4;
	UINT srvCount = indicesBufferCount + textureBufferCount;
	for (size_t i = 0; i < m_staticMeshes.size(); i++)
	{
		Utility::CreateDescriptorHeap(m_device, m_raytracingHeaps[i], DescriptorType::SRV, srvCount, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvTextureHandle(m_textureHeapNSV->GetCPUDescriptorHandleForHeapStart());
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvIndexHandle(m_staticMeshes[i]->GetIndexCpuHandle());
		CD3DX12_CPU_DESCRIPTOR_HANDLE destHandle(m_raytracingHeaps[i]->GetCPUDescriptorHandleForHeapStart());
		if (m_textureMap.count(m_staticMeshes[i]->GetTexturePath()) > 0) {
			srvTextureHandle.Offset(m_textureMap[m_staticMeshes[i]->GetTexturePath()], m_csuHeapSize);
		}
		else {
			srvTextureHandle.Offset(m_textureMap[L"zzzdefaultAlbedo.dds"], m_csuHeapSize);
		}
		m_device->CopyDescriptorsSimple(indicesBufferCount, destHandle, srvIndexHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		destHandle.Offset(indicesBufferCount, m_csuHeapSize);
		m_device->CopyDescriptorsSimple(textureBufferCount, destHandle, srvTextureHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	for (int i = 0; i < (int)mCharacter->GetMeshCount(); i++)
	{
		Utility::CreateDescriptorHeap(m_device, mChracterRaytracingHeaps[i], DescriptorType::SRV, srvCount, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvTextureHandle(m_textureHeapNSV->GetCPUDescriptorHandleForHeapStart());
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvIndexHandle(mCharacter->GetIndexCpuHandle(i));
		CD3DX12_CPU_DESCRIPTOR_HANDLE destHandle(mChracterRaytracingHeaps[i]->GetCPUDescriptorHandleForHeapStart());
		if (m_textureMap.count(mCharacter->GetTexturePath(i)) > 0) {
			srvTextureHandle.Offset(m_textureMap[mCharacter->GetTexturePath(i)], m_csuHeapSize);
		}
		else {
			srvTextureHandle.Offset(m_textureMap[L"zzzdefaultAlbedo.dds"], m_csuHeapSize);
		}
		m_device->CopyDescriptorsSimple(indicesBufferCount, destHandle, srvIndexHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		destHandle.Offset(indicesBufferCount, m_csuHeapSize);
		m_device->CopyDescriptorsSimple(textureBufferCount, destHandle, srvTextureHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	Utility::CreateDescriptorHeap(m_device, m_raytracingGlobalHeap, DescriptorType::SRV, 5, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvCubeTextureHandle(m_cubeMapTextureHeapNSV->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHdrUavHandle(m_hdrUavHeapNSV->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE destHandle(m_raytracingGlobalHeap->GetCPUDescriptorHandleForHeapStart());
	m_device->CopyDescriptorsSimple(1, destHandle, srvHdrUavHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	destHandle.Offset(1, m_csuHeapSize);
	if (m_cubeTextureMap.count(cubeMapTextureName) > 0) {
		UINT offset = m_cubeTextureMap[cubeMapTextureName] - 2;
		srvCubeTextureHandle.Offset(offset, m_csuHeapSize);
	}
	else {
		UINT offset = m_cubeTextureMap[L"DefaultEnvHDR.dds"] - 2;
		srvCubeTextureHandle.Offset(offset, m_csuHeapSize);
	}
	m_device->CopyDescriptorsSimple(4, destHandle, srvCubeTextureHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
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

	Utility::CreateConstantBuffer(m_device, m_commandList, mCsBuffer);
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
	if (!m_staticMeshes.empty()) {
		InitializeViews();
	}
}

void Renderer::D3D12RayTracingApp::InitRayTracingScene()
{
	using DirectX::SimpleMath::Vector3;

	characterMesh = new Core::StaticMesh();
	DirectX::SimpleMath::Matrix tr = DirectX::XMMatrixRotationY(XM_PI);
	std::tuple<std::vector<RaytracingMeshData>, Animation::AnimationData> soldierData;
	soldierData = GeometryGenerator::ReadFromFile<RaytracingVertex, uint32_t>("swat.fbx", false, true, tr);
	//auto [soldier, _] = GeometryGenerator::ReadFromFile<PbrVertex, uint32_t>("swat.fbx", false, true, tr);
	characterMesh->Initialize(std::get<0>(soldierData), m_device, m_commandList,
		Vector3(0.f, 0.f, 0.f),
		Material(1.f, 1.f, 1.f, 0.5f),
		false /*AO*/, false /*Height*/, true /*Metallic*/, true /*Normal*/, false /*Roughness*/, false /*Tesslation*/);
	//characterMesh->Initialize(GeometryGenerator::RTSphere(0.4f, 100, 100, L"worn-painted-metal_Albedo.dds"), m_device, m_commandList, Vector3(0.5, 0.4f, 0.f));
	characterMesh->BuildAccelerationStructures<RaytracingVertex, uint32_t>(m_device, m_dxrCommandList);

	for (size_t i = 0; i < characterMesh->meshCount; i++)
	{
		characterHitGroupNames.push_back(L"HitGroupCharacter" + i);
	}
	characterMesh->SetTexturePath(L"Soldier_Body_Albedo.dds", 0);
	characterMesh->SetTexturePath(L"Soldier_head_Albedo.dds", 1);
	characterMesh->SetTexturePath(L"Soldier_Body_Albedo.dds", 2);
	
	mCharacter->SetStaticMeshComponent(characterMesh);
	mCharacter->SetPosition(XMFLOAT3(0.f, 0.45f, 0.f));

	std::shared_ptr<Core::StaticMesh> box = std::make_shared<Core::StaticMesh>();
	//sphere1->Initialize(GeometryGenerator::RTSphere(0.45f, 100, 100), m_device, m_commandList, Vector3(-0.5, 0, 0));
	box->Initialize(GeometryGenerator::RTBox(0.4f, L"DiamondPlate008C_4K-PNG_Albedo.dds"), m_device, m_commandList, Vector3(-0.5, 0.4f, 0.5));
	box->BuildAccelerationStructures<RaytracingVertex, uint32_t>(m_device, m_dxrCommandList);
	hitGroupNames.push_back(L"HitGroupSphere1");

	std::shared_ptr<Core::StaticMesh> sphere = std::make_shared<Core::StaticMesh>();
	sphere->Initialize(GeometryGenerator::RTSphere(0.4f, 100, 100, L"worn-painted-metal_Albedo.dds"), m_device, m_commandList, Vector3(0.5, 0.4f, 0.f));
	//sphere2->Initialize(GeometryGenerator::RTBox(0.4f, L"worn-painted-metal_albedo.png"), m_device, m_commandList, Vector3(0.5, 0, 0.5));
	sphere->BuildAccelerationStructures<RaytracingVertex, uint32_t>(m_device, m_dxrCommandList);
	hitGroupNames.push_back(L"HitGroupSphere2");

	std::shared_ptr<Core::StaticMesh> plane = std::make_shared<Core::StaticMesh>();
	plane->Initialize(GeometryGenerator::RTBox(100.f, 1.f, 100.f), m_device, m_commandList, Vector3(0.0, -1.0f, 0.f));
	plane->BuildAccelerationStructures<RaytracingVertex, uint32_t>(m_device, m_dxrCommandList);
	hitGroupNames.push_back(L"HitGroupPlane");

	std::shared_ptr<Core::StaticMesh> cubeMap = std::make_shared<Core::StaticMesh>();
	cubeMap->Initialize(GeometryGenerator::RTCubeMapBox(100.f), m_device, m_commandList);
	cubeMap->SetTexturePath(cubeMapTextureName);
	cubeMap->SetIsCubeMap(true);
	cubeMap->BuildAccelerationStructures<RaytracingVertex, uint32_t>(m_device, m_dxrCommandList);
	hitGroupNames.push_back(L"HitGroupCube");

	m_staticMeshes.push_back(cubeMap);
	m_staticMeshes.push_back(box);
	m_staticMeshes.push_back(sphere);
	m_staticMeshes.push_back(plane);

	//for (size_t i = 0; i < soldier.size(); i++)
	//{
	//	std::shared_ptr<Core::StaticMesh> character = std::make_shared<Core::StaticMesh>();
	//	character->Initialize(soldier[i], m_device, m_commandList,
	//		Vector3(0.f, 0.45f, 0.f),
	//		Material(1.f, 1.f, 1.f, 1.f),
	//		false /*AO*/, false /*Height*/, true /*Metallic*/, true /*Normal*/, true /*Roughness*/, false /*Tesslation*/);
	//	character->BuildAccelerationStructures<RaytracingVertex, uint32_t>(m_device, m_dxrCommandList);
	//	character->SetTexturePath(pathList[i]);
	//	hitGroupNames.push_back(L"HitGroupCharacter"+ i);
	//	m_staticMeshes.push_back(character);
	//}
}

// TLAS 생성
void Renderer::D3D12RayTracingApp::BuildAccelerationStructures()
{
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE
		| D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = buildFlags;
	topLevelInputs.NumDescs = (UINT)m_staticMeshes.size() + (UINT)mCharacter->GetMeshCount();
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	UINT id = 0;

	for (size_t i = 0; i < m_staticMeshes.size(); i++)
	{
		//instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
		DirectX::SimpleMath::Matrix Model = m_staticMeshes[i]->GetTransformMatrix();
		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
		for (int x = 0; x < 3; x++)
		{
			for (int y = 0; y <= 3; y++)
			{
				instanceDesc.Transform[x][y] = Model.m[x][y];
			}
		}
		if (id == 0) {
			instanceDesc.InstanceMask = 0x02;
		}
		else {
			instanceDesc.InstanceMask = 0x01;
		}
		instanceDesc.InstanceID = id;
		instanceDesc.InstanceContributionToHitGroupIndex = id;
		instanceDesc.AccelerationStructure = m_staticMeshes[i]->GetBlas();
		m_instances.push_back(instanceDesc);
		id++;
	}
	characterInstanceId = id;
	for (int i = 0; i < (int)mCharacter->GetMeshCount(); i++)
	{
		DirectX::SimpleMath::Matrix Model = mCharacter->GetTransformMatrix();
		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
		for (int x = 0; x < 3; x++)
		{
			for (int y = 0; y <= 3; y++)
			{
				instanceDesc.Transform[x][y] = Model.m[x][y];
			}
		}
		instanceDesc.InstanceMask = 0x01;
		instanceDesc.InstanceID = id;
		instanceDesc.InstanceContributionToHitGroupIndex = id;
		instanceDesc.AccelerationStructure = mCharacter->GetBlas(i);
		m_instances.push_back(instanceDesc);
		id++;
	}

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	m_device->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);

	Utility::CreateBuffer(m_device, D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE,
		topLevelPrebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON,
		mScratchResource);

	Utility::CreateBuffer(m_device, D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE,
		topLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		m_tlas);


	UINT64 datasize = (UINT64)(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * m_instances.size());

	Utility::CreateBuffer(m_device, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, datasize, D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ, m_instanceDescs);


	m_instanceDescs->Map(0, nullptr, reinterpret_cast<void**>(&pInstancesMappedData));
	memcpy(pInstancesMappedData, m_instances.data(), datasize);
	//m_instanceDescs->Unmap(0, nullptr);

	topLevelBuildDesc = {};

	topLevelInputs.InstanceDescs = m_instanceDescs->GetGPUVirtualAddress();
	topLevelBuildDesc.Inputs = topLevelInputs;
	topLevelBuildDesc.ScratchAccelerationStructureData = mScratchResource->GetGPUVirtualAddress();
	topLevelBuildDesc.DestAccelerationStructureData = m_tlas->GetGPUVirtualAddress();

	m_dxrCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

	m_dxrCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_tlas.Get()));
	m_dxrCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(mScratchResource.Get()));

	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_tlas.Get()));

	std::wstringstream tlassName;
	tlassName << L"TLAS";
	m_tlas->SetName(tlassName.str().c_str());
}

void Renderer::D3D12RayTracingApp::Update(float& deltaTime)
{
	using DirectX::SimpleMath::Vector3;
	using DirectX::SimpleMath::Vector4;
	using DirectX::SimpleMath::Matrix;

	//m_inputHandler->ExicuteCommand(m_camera.get(), deltaTime, bIsFPSMode);
	m_inputHandler->ExicuteCommand(mCharacter.get(), deltaTime, bIsFPSMode);
	mCharacter->Update(deltaTime);
	//m_sceneCBData.cameraPosition = m_camera->GetPosition();
	m_sceneCBData.cameraPosition = mCharacter->GetCameraPosition();

	Matrix view = mCharacter->GetViewMatrix();
	Matrix projection = mCharacter->GetProjMatrix() * DirectX::XMMatrixTranslation(0, 0, m_camera->d);

	projection = projection.Invert();
	projection = projection.Transpose();

	view = view.Invert();
	view = view.Transpose();

	m_sceneCBData.view = view;
	m_sceneCBData.projection = projection;
	m_sceneCBData.d = m_camera->d;

	view.m[0][3] = 0;
	view.m[1][3] = 0;
	view.m[2][3] = 0;

	m_sceneCBData.cubeMapView = view;

	memcpy(pSceneBegin, &m_sceneCBData, sizeof(SceneConstantBuffer));
	

	float delAngle = (3.14f / 3.f) * deltaTime;
	static float angle = 0.f;
	angle += delAngle;
	Matrix rotate = DirectX::XMMatrixRotationY(angle);
	m_staticMeshes[2]->UpdateWorldRow(rotate);

	for (auto& mesh : m_staticMeshes) {
		mesh->Update(deltaTime);
	}

	// instantce Model 행렬은 Transpose 후 적용
	rotate = rotate.Transpose();
	DirectX::SimpleMath::Matrix Model = mCharacter->GetTransformMatrix();
	//Model = Model.Transpose();
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m_instances[2].Transform[i][j] = rotate.m[i][j];
			for (size_t k = 0; k < mCharacter->GetMeshCount(); k++)
			{
				m_instances[k+characterInstanceId].Transform[i][j] = Model.m[i][j];
			}
		}
	}
	UINT64 datasize = (UINT64)(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * m_instances.size());
	m_instanceDescs->Map(0, nullptr, reinterpret_cast<void**>(&pInstancesMappedData));
	memcpy(pInstancesMappedData, m_instances.data(), datasize);
	

	UINT numShaderRecords = (UINT)m_staticMeshes.size();
	UINT shaderRecordSize = 128;

	// hit constant buffer 업데이트
	hitShaderTable->ResetMap();
	for (size_t i = 0; i < hitGroupNames.size(); i++)
	{
		hitShaderTable->m_mappedShaderRecoreds += 40;
		memcpy(hitShaderTable->m_mappedShaderRecoreds, &(m_staticMeshes[i]->m_primitiveConstantData), sizeof(PrimitiveConstantBuffer));
		hitShaderTable->m_mappedShaderRecoreds += 88;
	}
	for (size_t i = 0; i < characterHitGroupNames.size(); i++)
	{
		hitShaderTable->m_mappedShaderRecoreds += 40;
		PrimitiveConstantBuffer data = mCharacter->GetprimitiveConstantData();
		memcpy(hitShaderTable->m_mappedShaderRecoreds, &data, sizeof(PrimitiveConstantBuffer));
		hitShaderTable->m_mappedShaderRecoreds += 88;
	}
}

void Renderer::D3D12RayTracingApp::UpdateGUI(float& deltaTime)
{
	//D3D12App::UpdateGUI(deltaTime);
}

void Renderer::D3D12RayTracingApp::Render(float& deltaTime)
{
	RaytracingPass(deltaTime);
	//CopyResource(m_commandList, CurrentBackBuffer(), HDRRenderTargetBuffer());
	PostProcessing(deltaTime);
	CopyResourceToSwapChain(deltaTime);
}
void Renderer::D3D12RayTracingApp::RaytracingPass(float& deltaTime)
{
	auto desc = HDRRenderTargetBuffer()->GetDesc();
	UINT width = (UINT)desc.Width;
	UINT height = desc.Height;

	m_commandAllocator->Reset();
	m_dxrCommandList->Reset(m_commandAllocator.Get(), nullptr);

	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(0, 0, 255), raytracingPass);

	m_dxrCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

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

	m_dxrCommandList->SetDescriptorHeaps(1, m_raytracingGlobalHeap.GetAddressOf());
	CD3DX12_GPU_DESCRIPTOR_HANDLE globalHandle(m_raytracingGlobalHeap->GetGPUDescriptorHandleForHeapStart());
	m_dxrCommandList->SetComputeRootDescriptorTable(0, globalHandle);
	globalHandle.Offset(1, m_csuHeapSize);
	m_dxrCommandList->SetComputeRootDescriptorTable(1, globalHandle);
	m_dxrCommandList->SetComputeRootShaderResourceView(2, GetTlas());
	m_dxrCommandList->SetComputeRootConstantBufferView(3, m_sceneCB->GetGPUVirtualAddress());

	D3D12_DISPATCH_RAYS_DESC dispactchRays = {};

	dispactchRays.RayGenerationShaderRecord.StartAddress = m_rgsTable->GetGPUVirtualAddress();

	dispactchRays.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	//TODO : size
	dispactchRays.HitGroupTable.StartAddress = hitShaderTable->GetGPUVirtualAddress();
	dispactchRays.HitGroupTable.SizeInBytes = 128 * (hitGroupNames.size() + characterHitGroupNames.size());
	dispactchRays.HitGroupTable.StrideInBytes = 128;

	dispactchRays.MissShaderTable.StartAddress = m_missTable->GetGPUVirtualAddress();
	dispactchRays.MissShaderTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	dispactchRays.MissShaderTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

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

	//CaptureBufferToPNG();
}
void Renderer::D3D12RayTracingApp::RenderCubeMap(float& deltaTime)
{
}

void Renderer::D3D12RayTracingApp::RenderGUI(float& deltaTime)
{
	D3D12App::RenderGUI(deltaTime);
}
